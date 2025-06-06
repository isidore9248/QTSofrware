/*
 * @Description:
 * @Version: v1.0.0
 * @Author: isidore-chen
 * @Date: 2025-05-04 20:59:47
 * @Copyright: Copyright (c) 2025 CAUC
 */
#include "USARTAss.h"
#include "SerialInfo.h"
#include "Chart.h"
#include <QDebug>
#include <stdexcept>
#include <QMessageBox>
#include <QRegularExpression> // Added for QRegularExpression

/**
 * @brief USARTAss类的构造函数。
 * @param parent 父QWidget对象。
 */
USARTAss::USARTAss(QWidget *parent)
	: QMainWindow(parent), serialOpened(false), serialSendMessage(), totalBytes(0), EndFrame("END"), RecvCheck(false),
	  ChartFrame{"START1", "START2", "START3"}, ChartFrameIndex(-1)
{
	ui.setupUi(this);

	TotalConnect();
	ShowChart();

	ui.VDetectCoord_1->setStyleSheet("background-color: lightgray;");
	ui.VDetectCoord_2->setStyleSheet("background-color: lightgray;");
	ui.VDetectCoord_3->setStyleSheet("background-color: lightgray;");

	qDebug() << "ChartFrame" << ChartFrame;
}

/**
 * @brief USARTAss类的析构函数。
 */
USARTAss::~USARTAss()
{
}

/**
 * @brief 处理打开/关闭串口按钮点击事件的槽函数。
 *
 * 该函数首先读取用户在UI中设置的串口信息，
 * 然后尝试打开或关闭串口。
 * 根据操作结果更新UI（按钮文本）并显示提示信息。
 * 如果串口成功打开，它还会连接QSerialPort的readyRead信号到RecvMessage_clicked槽函数。
 * 如果发生错误，会显示错误消息框。
 */
void USARTAss::OpenCloseUSART_clicked()
{
	ReadUsrSerialInfo();
	try
	{
		serialOpened = SerialInfo::getInstance().SerialChangestate(serialOpened);
	}
	catch (const std::runtime_error &e)
	{
		// 使用 QMessageBox 提供更清晰的错误提示
		QMessageBox::critical(this, "USART-Err", QString("An error occurred when opening or closing the serial port: %1").arg(e.what()));
		qDebug() << "Error opening/closing serial port:" << e.what();
		return; // 退出函数，避免继续执行
	}
	catch (...)
	{
		QMessageBox::critical(this, "unknown-error", "An unknown error occurred when opening or closing the serial port。");
		qDebug() << "Unknown error occurred while opening/closing serial port.";
		return; // 退出函数，避免继续执行
	}

	if (serialOpened)
	{
		connect(SerialInfo::getInstance().GetSerialPort(), &QSerialPort::readyRead, this, &USARTAss::RecvMessage_clicked);
		QMessageBox::information(this, "USART-Info", "Serial port opened successfully.");
	}
	else
	{
		// 如果串口关闭，可能需要断开连接，以避免野指针或不必要的回调
		// QObject::disconnect(SerialInfo::getInstance().GetSerialPort(), &QSerialPort::readyRead, this, &USARTAss::RecvMessage_clicked);
		// 注意：上面的 disconnect 如果 serialPort 已经被 delete 或者置为 nullptr，直接调用 GetSerialPort() 可能会有问题。
		// 更安全的做法是在 SerialInfo 类中管理连接的断开。
		// 或者确保 GetSerialPort() 在 serialPort 无效时返回 nullptr，并在这里做检查。
		QSerialPort *port = SerialInfo::getInstance().GetSerialPort();
		if (port)
		{
			QObject::disconnect(port, &QSerialPort::readyRead, this, &USARTAss::RecvMessage_clicked);
		}
		QMessageBox::information(this, "USART-Info", "Serial port closed.");
	}

	ChangeSerialButtonText(serialOpened);
}

/**
 * @brief 处理刷新串口列表按钮点击事件的槽函数。
 *
 * 该函数清空UI中的当前串口列表，然后重新扫描系统中的可用串口，
 * 并将它们及其描述添加到下拉列表中。
 * 同时，它会将数据位、停止位和校验位的UI选项重置为默认值。
 */
void USARTAss::RefreshUSART_clicked()
{
	// 清空当前串口列表
	ui.USARTInfo->clear();

	// 获取可用串口信息
	const auto availablePorts = QSerialPortInfo::availablePorts();

	// 遍历可用串口并添加到下拉框
	for (const QSerialPortInfo &portInfo : availablePorts)
	{
		// 获取串口的详细信息
		QString portDetails = QString("%1  %2")
								  .arg(portInfo.portName())
								  .arg(portInfo.description().isEmpty() ? "未知" : portInfo.description());
		// 添加到下拉框
		ui.USARTInfo->addItem(portDetails);
	}
	// 如果没有可用串口，显示提示
	if (availablePorts.isEmpty())
	{
		ui.USARTInfo->addItem("non-available-serail");
	}

	// 设置默认数据位置
	ui.DataBitsInfo->setCurrentText("8");

	// 设置默认停止位
	ui.StopBitsInfo->setCurrentText("1");

	// 设置默认奇偶校验位
	ui.ParityInfo->setCurrentText("None");
}

/**
 * @brief 处理发送消息按钮点击事件的槽函数。
 *
 * 该函数从UI的发送文本框中获取文本，添加换行符，
 * 然后通过SerialInfo单例将消息发送出去。
 */
void USARTAss::SendMessage_clicked()
{
	serialSendMessage = ui.SendSpace->toPlainText() + "\n";
	SerialInfo::getInstance().SerialSendMessage(serialSendMessage);
}

/**
 * @brief 处理串口接收到数据时的readyRead信号的槽函数。
 *
 * 该函数从SerialInfo单例读取所有可用数据，更新接收字节数显示。
 * 然后，它根据当前的状态机（WaitingForStart, WaitingForData, WaitingForEnd）
 * 处理接收到的数据帧。
 * - WaitingForStart: 等待帧头
 * - WaitingForData: 等待浮点型数据。
 * - WaitingForEnd: 等待帧尾
 * 成功接收完整数据包后，会将数据添加到相应的图表中。
 * 如果在任何阶段接收到无效数据，状态机将重置。
 */
void USARTAss::RecvMessage_clicked()
{
	buffer = SerialInfo::getInstance().SerialRecvMessage(totalBytes);
	ShowRecvBytesCount();

	// 将接收到的数据转换为字符串
	QString receivedData = QString::fromUtf8(buffer).trimmed();
	qDebug() << "Raw received data:" << receivedData;

	if (RecvCheck)
	{
		// size_t ChartFrameIndex = -1;
		std::vector<QString>::iterator ret; // 将变量声明移到switch外部
		switch (currentState)
		{
		case WaitingForStart:
		{
			ret = std::find(ChartFrame.begin(), ChartFrame.end(), receivedData);
			if (ret != ChartFrame.end())
			{
				ChartFrameIndex = std::distance(ChartFrame.begin(), ret);
				qDebug() << "Found ChartFrameIndex:" << ChartFrameIndex;
				currentStartFrame = receivedData; // 保存帧头
				currentState = WaitingForData;	  // 切换到等待数据帧状态
				ui.RecvSpace->append("Received Start Frame: " + currentStartFrame);
				qDebug() << "Received Start Frame:" << currentStartFrame;
			}
			else
			{
				this->currentState = WaitingForStart;
				this->currentStartFrame.clear();
				this->currentDataFrame = 0.0f;

				ui.RecvSpace->append("Invalid Start Frame: " + receivedData);
				qDebug() << "Invalid Start Frame:" << receivedData;
			}

			break;
		}
		case WaitingForData:
		{
			bool isFloat;
			float value = receivedData.toFloat(&isFloat);
			if (isFloat)
			{
				currentDataFrame = value;	  // 保存数据帧
				currentState = WaitingForEnd; // 切换到等待帧尾状态
				ui.RecvSpace->append("Received Data Frame: " + QString::number(currentDataFrame));
				qDebug() << "Received Data Frame:" << currentDataFrame;
			}
			else
			{
				this->currentState = WaitingForStart;
				this->currentStartFrame.clear();
				this->currentDataFrame = 0.0f;

				ui.RecvSpace->append("Invalid Data Frame: " + receivedData);
				qDebug() << "Invalid Data Frame:" << receivedData;
			}
			break;
		}

		case WaitingForEnd:
		{
			if (receivedData == EndFrame)
			{
				currentState = WaitingForStart; // 切换回等待帧头状态
				ui.RecvSpace->append("Received End Frame: " + receivedData);
				qDebug() << "Received End Frame:" << receivedData;

				// 处理完整数据包
				QString ShowMessage = "Complete Packet - Start: " + currentStartFrame +
									  ", Data: " + QString::number(currentDataFrame) +
									  ", End: " + receivedData;
				ui.RecvSpace->append(ShowMessage);

				qDebug() << "ChartFrameIndex:" << ChartFrameIndex;
				qDebug() << "currentStartFrame:" << currentStartFrame;

				if (ChartFrameIndex != -1)
				{
					Chart::GetInstance().AddToChartData(ChartFrameIndex, currentDataFrame);
					ChartFrameIndex = -1;
				}
				else
				{
				}
			}
			else
			{
				this->currentState = WaitingForStart;
				this->currentStartFrame.clear();
				this->currentDataFrame = 0.0f;

				ui.RecvSpace->append("Invalid End Frame: " + receivedData);
				qDebug() << "Invalid End Frame:" << receivedData;
			}
			break;

		default:
			ui.RecvSpace->append("Unknown State");
			qDebug() << "Unknown State";
			break;
		}
		}
	}
	else
	{
		ui.RecvSpace->append("Received Frame: " + receivedData);
	}
}

/**
 * @brief 更新UI上显示鼠标悬停坐标的标签。
 *
 * 当Chart类发出hoveredCoordinatesChanged信号时，此槽函数被调用。
 * 它根据图表索引更新相应的坐标标签。
 *
 * @param chartIndex 发生悬停事件的图表的索引 (0, 1, 或 2)。
 * @param point 悬停点的QPointF坐标。
 */
void USARTAss::updateHoveredCoordinates(int chartIndex, QPointF point)
{
	QString coordText = QString::number(point.x(), 'f', 2) + ", " + QString::number(point.y(), 'f', 2);
	if (chartIndex == 0)
	{
		ui.VDetectCoord_1->setText(coordText);
	}
	else if (chartIndex == 1)
	{
		if (ui.VDetectCoord_2)
			ui.VDetectCoord_2->setText(coordText);
	}
	else if (chartIndex == 2)
	{
		if (ui.VDetectCoord_3)
			ui.VDetectCoord_3->setText(coordText);
	}
}

void USARTAss::OpenfraemCheck_on_click()
{
	qDebug() << "i am in on click";
	// GettheFrameStartandEnd();
	RecvCheck = true;
}

/**
 * @brief 处理清空发送区按钮点击事件的槽函数。
 *
 * 该函数清空UI中的发送文本框。
 */
void USARTAss::ClearSendSpace_clicked()
{
	ui.SendSpace->clear();
}

/**
 * @brief 连接所有UI控件的信号到相应的槽函数。
 *
 * 此函数在构造函数中调用，用于设置UI交互。
 */
void USARTAss::TotalConnect()
{
	// 打开串口按钮
	connect(ui.OpenCloseUSART, &QPushButton::clicked, this, &USARTAss::OpenCloseUSART_clicked);
	// 刷新串口按钮
	connect(ui.RefreshUSART, &QPushButton::clicked, this, &USARTAss::RefreshUSART_clicked);
	// 发送消息按钮
	connect(ui.SendSerialMessage, &QPushButton::clicked, this, &USARTAss::SendMessage_clicked);
	// 清空发送区按钮
	connect(ui.ClearSendSpace, &QPushButton::clicked, this, &USARTAss::ClearSendSpace_clicked);
	// 接收消息的槽函数放在了OpenCloseUSART_clicked()中，需要指针的传递，所以在每次打开时更新

	connect(ui.OpenfraemCheck, &QRadioButton::clicked, this, &USARTAss::OpenfraemCheck_on_click);
	connect(ui.ClosefraemCheck, &QRadioButton::clicked, this, &USARTAss::ClosefraemCheck_on_click);
}

/**
 * @brief 从UI读取用户设置的串口配置信息。
 *
 * 该函数从UI控件（下拉框、文本框）中读取波特率、数据位、
 * 停止位、校验位和串口名称，然后通过SerialInfo单例
 * 设置这些配置。
 * 如果发生无效输入或其他错误，会显示警告或错误消息框。
 */
void USARTAss::ReadUsrSerialInfo()
{
	try
	{
		// 1. 读取波特率
		qint32 baudRate = ui.BaudInfo->currentText().toInt();
		// 2. 读取数据位
		qint32 DataBits = ui.DataBitsInfo->currentText().toInt();
		// 3. 读取停止位
		qint32 StopBits = ui.StopBitsInfo->currentText().toInt();
		// 4. 读取奇偶校验位
		QString parityStr = ui.ParityInfo->currentText();
		// 5. 读取串口名称
		QString portName = ui.USARTInfo->currentText();
		// 一次性设置所有配置到 SerialInfo 单例
		SerialInfo::getInstance().SetSerialConfiguration(baudRate, DataBits, StopBits, parityStr, portName);

		qDebug() << "Serial configuration read from UI and set in SerialInfo.";
	}
	catch (const std::invalid_argument &e)
	{
		// 使用 QMessageBox 提供更清晰的错误提示
		QMessageBox::warning(this, "无效输入", QString("设置串口参数时出错: %1").arg(e.what()));
		qDebug() << "Error setting serial configuration:" << e.what();
	}
	catch (...) // 捕获其他可能的未知异常
	{
		QMessageBox::critical(this, "未知错误", "设置串口参数时发生未知错误。");
		qDebug() << "Unknown error occurred while setting serial configuration.";
	}
}

/**
 * @brief 根据串口打开状态更改打开/关闭按钮的文本。
 * @param serialOpened 布尔值，指示串口是否已打开。
 */
void USARTAss::ChangeSerialButtonText(bool serialOpened)
{
	if (serialOpened)
	{
		ui.OpenCloseUSART->setText("Close");
	}
	else
	{
		ui.OpenCloseUSART->setText("Open");
	}
}

/**
 * @brief 在UI上显示已接收的总字节数。
 */
void USARTAss::ShowRecvBytesCount()
{
	ui.RXBytescount->setText("RX Bytes:" + QString::number(totalBytes));
}

/**
 * @brief 初始化并显示应用程序中的图表。
 *
 * 此函数获取Chart单例中的三个图表实例，并将它们设置到UI中
 * 相应的QChartView控件上。它还设置了抗锯齿渲染提示。
 * 最后，它连接Chart实例的hoveredCoordinatesChanged信号到
 * updateHoveredCoordinates槽函数，以更新UI上显示的鼠标悬停坐标。
 * 此连接是静态的，确保只连接一次。
 */
void USARTAss::ShowChart()
{
	// Setup for 3 chart views
	QChartView *chartViews[] = {ui.VDetectGGraph_1, ui.VDetectGGraph_2, ui.VDetectGGraph_3};

	for (int i = 0; i < 3; ++i)
	{
		QChart *chartInstance = Chart::GetInstance().GetChart(i);
		if (chartInstance)
		{
			if (chartViews[i])
			{ // Check if the QChartView pointer from UI is valid
				chartViews[i]->setChart(chartInstance);
				chartViews[i]->setRenderHints(QPainter::Antialiasing);
				// Set common properties or specific ones if needed
				// chartViews[i]->setMinimumSize(400, 300);
				// chartViews[i]->setMaximumSize(800, 600);
			}
			else
			{
				qDebug() << "UI QChartView VDetectGGraph_" << i + 1 << "is null.";
			}
		}
		else
		{
			qDebug() << "Chart instance for ChartFrameIndex" << i << "is null.";
		}
	}
	// Connect the signal once, as it now carries the chartIndex
	// Ensure this connection is made only once, e.g., in constructor or here with a check
	// For simplicity, connecting it here. If ShowChart can be called multiple times, consider a QMetaObject::Connection object to manage it.
	static QMetaObject::Connection chartSignalConnection;
	if (chartSignalConnection)
		QObject::disconnect(chartSignalConnection); // Disconnect previous if any
	chartSignalConnection = connect(&Chart::GetInstance(), &Chart::hoveredCoordinatesChanged, this, &USARTAss::updateHoveredCoordinates);
}

/**
 * @brief 处理关闭帧检查复选框点击事件的槽函数。
 *
 * 当用户点击“关闭帧检查”复选框时，此函数将 RecvCheck 标志设置为 false，
 * 禁用接收数据时的帧检查逻辑。
 */
void USARTAss::ClosefraemCheck_on_click()
{
	qDebug() << "i am in off click";
	RecvCheck = false;
}

/**
 * @brief 处理设置图表帧按钮点击事件的槽函数。
 *
 * 当用户点击“设置图表帧”按钮时，此函数调用 GetChartStartFrame
 * 从UI中读取并设置图表起始帧。
 */
void USARTAss::on_SetChartFrame_clicked()
{
	qDebug() << "i am in set chart frame";
	GetChartStartFrame();
}

/**
 * @brief 从UI中获取并设置图表起始帧。
 *
 * 此函数遍历预定义的图表帧数组，并从UI中相应的 QTextEdit 控件中
 * 读取文本内容，将其设置为每个图表的起始帧。
 */
void USARTAss::GetChartStartFrame()
{
	for (int i = 0; i < ChartFrame.size(); i++)
	{
		std::shared_ptr<QTextEdit> currentEdit; // 当前处理的编辑框

		// 根据索引分配对应的编辑框
		if (i == 0)
		{
			currentEdit.reset(ui.Chart1StartFrame, [](auto) {}); // 使用空删除器，避免重复释放
		}
		else if (i == 1)
		{
			currentEdit.reset(ui.Chart2StartFrame, [](auto) {});
		}
		else if (i == 2)
		{
			currentEdit.reset(ui.Chart3StartFrame, [](auto) {});
		}

		ChartFrame[i] = currentEdit->toPlainText();
	}
}
