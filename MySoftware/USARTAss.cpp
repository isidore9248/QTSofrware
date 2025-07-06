/*
 * @Description:
 * @Version: v1.0.0
 * @Author: isidore-chen
 * @Date: 2025-05-04 20:59:47
 * @Copyright: Copyright (c) 2025 CAUC
 */
#include "USARTAss.h"
#include "SerialInfo.h"
#include <QDebug>
#include <stdexcept>
#include <QMessageBox>
#include <QRegularExpression> // Added for QRegularExpression
#include <QThread>

 /**
  * @brief USARTAss类的构造函数。
  * @param parent 父QWidget对象。
  */
USARTAss::USARTAss(QWidget* parent)
	: QMainWindow(parent), serialOpened(false), serialSendMessage(), totalBytes(0), EndFrame("END"), RecvCheck(false),
	ChartFrame{ "START1", "START2", "START3" }, FrameIndex(-1),
	m_serialInfo(new SerialInfo(this)) // 初始化 m_serialInfo
{
	ui.setupUi(this);

	TotalConnect();

	qDebug() << "ChartFrame" << ChartFrame;
}

/**
 * @brief USARTAss类的析构函数。
 */
USARTAss::~USARTAss()
{
	// m_serialInfo 会在父对象析构时自动删除，因为它是 QObject 的子对象
	// 但需要确保其内部线程已停止
	if (m_serialInfo) {
		// SerialInfo 的析构函数会处理其内部线程的停止和清理
	}
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
		serialOpened = m_serialInfo->SerialChangestate(serialOpened); // 使用 m_serialInfo
	}
	catch (const std::runtime_error& e)
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
		// readyRead 信号的连接已在 TotalConnect 中处理，这里不需要重复连接
		QMessageBox::information(this, "USART-Info", "Serial port opened successfully.");
	}
	else
	{
		// readyRead 信号的断开连接已在 SerialInfo::SerialChangestate 中处理
		QMessageBox::information(this, "USART-Info", "Serial port closed.");
	}
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
	for (const QSerialPortInfo& portInfo : availablePorts)
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
	m_serialInfo->SerialSendMessage(serialSendMessage); // 使用 m_serialInfo
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
void USARTAss::RecvMessage_clicked(const QByteArray& data) // 接收 QByteArray 参数
{
	totalBytes += data.size(); // 累加接收到的字节数
	ShowRecvBytesCount();

	// 将接收到的数据转换为字符串
	QString receivedData = QString::fromUtf8(data).trimmed(); // 使用传入的 data 参数
	qDebug() << "Raw received data:" << receivedData;

	if (RecvCheck)
	{
		// size_t FrameIndex = -1;
		std::vector<QString>::iterator ret; // 将变量声明移到switch外部
		switch (currentState)
		{
		case WaitingForStart:
		{
			ret = std::find(ChartFrame.begin(), ChartFrame.end(), receivedData);
			if (ret != ChartFrame.end())
			{
				FrameIndex = std::distance(ChartFrame.begin(), ret);
				qDebug() << "Found FrameIndex:" << FrameIndex;
				currentStartFrame = receivedData; // 保存帧头
				currentState = WaitingForData1;	  // 切换到等待第一个数据帧状态
				ui.RecvSpace->append("Received Start Frame: " + currentStartFrame);
				qDebug() << "Received Start Frame:" << currentStartFrame;
			}
			else
			{
				this->currentState = WaitingForStart;
				this->currentStartFrame.clear();
				this->currentDataFrame1 = 0.0f;
				this->currentDataFrame2 = 0.0f;
				this->currentDataFrame3 = 0.0f;

				ui.RecvSpace->append("Invalid Start Frame: " + receivedData);
				qDebug() << "Invalid Start Frame:" << receivedData;
			}

			break;
		}
		case WaitingForData1:
		{
			bool isFloat;
			float value = receivedData.toFloat(&isFloat);
			if (isFloat)
			{
				currentDataFrame1 = value;	  // 保存第一个数据帧
				currentState = WaitingForData2; // 切换到等待第二个数据帧状态
				ui.RecvSpace->append("Received Data Frame 1: " + QString::number(currentDataFrame1));
				qDebug() << "Received Data Frame 1:" << currentDataFrame1;
			}
			else
			{
				this->currentState = WaitingForStart;
				this->currentStartFrame.clear();
				this->currentDataFrame1 = 0.0f;
				this->currentDataFrame2 = 0.0f;
				this->currentDataFrame3 = 0.0f;

				ui.RecvSpace->append("Invalid Data Frame 1: " + receivedData);
				qDebug() << "Invalid Data Frame 1:" << receivedData;
			}
			break;
		}
		case WaitingForData2:
		{
			bool isFloat;
			float value = receivedData.toFloat(&isFloat);
			if (isFloat)
			{
				currentDataFrame2 = value;	  // 保存第二个数据帧
				currentState = WaitingForData3; // 切换到等待第三个数据帧状态
				ui.RecvSpace->append("Received Data Frame 2: " + QString::number(currentDataFrame2));
				qDebug() << "Received Data Frame 2:" << currentDataFrame2;
			}
			else
			{
				this->currentState = WaitingForStart;
				this->currentStartFrame.clear();
				this->currentDataFrame1 = 0.0f;
				this->currentDataFrame2 = 0.0f;
				this->currentDataFrame3 = 0.0f;

				ui.RecvSpace->append("Invalid Data Frame 2: " + receivedData);
				qDebug() << "Invalid Data Frame 2:" << receivedData;
			}
			break;
		}
		case WaitingForData3:
		{
			bool isFloat;
			float value = receivedData.toFloat(&isFloat);
			if (isFloat)
			{
				currentDataFrame3 = value;	  // 保存第三个数据帧
				currentState = WaitingForEnd; // 切换到等待帧尾状态
				ui.RecvSpace->append("Received Data Frame 3: " + QString::number(currentDataFrame3));
				qDebug() << "Received Data Frame 3:" << currentDataFrame3;
			}
			else
			{
				this->currentState = WaitingForStart;
				this->currentStartFrame.clear();
				this->currentDataFrame1 = 0.0f;
				this->currentDataFrame2 = 0.0f;
				this->currentDataFrame3 = 0.0f;

				ui.RecvSpace->append("Invalid Data Frame 3: " + receivedData);
				qDebug() << "Invalid Data Frame 3:" << receivedData;
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
					", Data1: " + QString::number(currentDataFrame1) +
					", Data2: " + QString::number(currentDataFrame2) +
					", Data3: " + QString::number(currentDataFrame3) +
					", End: " + receivedData;
				ui.RecvSpace->append(ShowMessage);

				qDebug() << "FrameIndex:" << FrameIndex;
				qDebug() << "currentStartFrame:" << currentStartFrame;

				if (FrameIndex != -1)
				{
					PID_parameters PIDdata;
					PIDdata.Kp = currentDataFrame1;
					PIDdata.Ki = currentDataFrame2;
					PIDdata.Kd = currentDataFrame3;
					emit PIDReadyToShow(FrameIndex, PIDdata); // 发送数据到图表
					FrameIndex = -1;
				}
				else
				{
				}
			}
			else
			{
				this->currentState = WaitingForStart;
				this->currentStartFrame.clear();
				this->currentDataFrame1 = 0.0f;
				this->currentDataFrame2 = 0.0f;
				this->currentDataFrame3 = 0.0f;

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

void USARTAss::ClearRecvSpace_clicked()
{
	ui.RecvSpace->clear();
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
	// 清空接收区按钮
	connect(ui.ClearRecvSpace, &QPushButton::clicked, this, &USARTAss::ClearRecvSpace_clicked);

	// 接收消息的槽函数放在了OpenCloseUSART_clicked()中，需要指针的传递，所以在每次打开时更新
	connect(ui.OpenfraemCheck, &QRadioButton::clicked, this, &USARTAss::OpenfraemCheck_on_click);
	connect(ui.ClosefraemCheck, &QRadioButton::clicked, this, &USARTAss::ClosefraemCheck_on_click);

	// 连接 SerialInfo 的 DataReceived 信号到 USARTAss 的 RecvMessage_clicked 槽
	connect(m_serialInfo, &SerialInfo::DataReceived, this, &USARTAss::RecvMessage_clicked);
	// 连接 SerialInfo 的 SerialStateChanged 信号，用于更新 UI 状态
	connect(m_serialInfo, &SerialInfo::SerialStateChanged, this, &USARTAss::ChangeSerialButtonText);

	connect(this, &USARTAss::PIDReadyToShow, this, &USARTAss::ShowPID);
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
		// 一次性设置所有配置到 SerialInfo 对象
		m_serialInfo->SetSerialConfiguration(baudRate, DataBits, StopBits, parityStr, portName); // 使用 m_serialInfo

		qDebug() << "Serial configuration read from UI and set in SerialInfo.";
	}
	catch (const std::invalid_argument& e)
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

void USARTAss::ShowPID(size_t index, PID_parameters PIDdata)
{
	if (index == 0)
	{
		ui.PID1_P->setText(QString::number(PIDdata.Kp));
		ui.PID1_I->setText(QString::number(PIDdata.Ki));
		ui.PID1_D->setText(QString::number(PIDdata.Kd));
	}
	else if (index == 1)
	{
		ui.PID2_P->setText(QString::number(PIDdata.Kp));
		ui.PID2_I->setText(QString::number(PIDdata.Ki));
		ui.PID2_D->setText(QString::number(PIDdata.Kd));
	}
	else if (index == 2)
	{
		ui.PID3_P->setText(QString::number(PIDdata.Kp));
		ui.PID3_I->setText(QString::number(PIDdata.Ki));
		ui.PID3_D->setText(QString::number(PIDdata.Kd));
	}
	else
	{
		qDebug() << "Invalid index for PID data:" << index;
	}
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