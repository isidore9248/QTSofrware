/*
 * @Description:
 * @Version: v1.0.0
 * @Author: isidore-chen
 * @Date: 2025-05-04 22:15:09
 * @Copyright: Copyright (c) 2025 CAUC
 */
#include "SerialInfo.h"
#include <QDialog>
#include <stdexcept>
#include <QMessageBox>
#include <QRegularExpression> // Added for QRegularExpression

 /**
  * @brief SerialInfo类的构造函数。
  * 初始化串口参数为默认值。
  */
SerialInfo::SerialInfo(QObject* parent) : QObject(parent), dataBits(QSerialPort::Data8), stopBits(QSerialPort::OneStop),
parity(QSerialPort::NoParity), serialPort(nullptr), serialReadThread(new QThread(this))
{
	// 将 SerialInfo 对象移动到新线程
	this->moveToThread(serialReadThread);

	// 连接线程的 started 信号到 SerialInfo 的槽，以便在线程启动时进行初始化
	connect(serialReadThread, &QThread::started, this, [this]() {
		// 确保 serialPort 在这个线程中被创建和使用
		if (serialPort == nullptr) {
			serialPort = new QSerialPort();
			ConfigureSerialPort(); // 重新配置串口，确保在正确线程中
		}
		// 连接 readyRead 信号
		connect(serialPort, &QSerialPort::readyRead, this, &SerialInfo::handleReadyRead);
		});

	// 连接线程的 finished 信号，以便在线程结束时进行清理
	connect(serialReadThread, &QThread::finished, serialReadThread, &QThread::deleteLater);
	// 注意：serialPort 的 deleteLater 应该在 SerialInfo 的析构函数中处理，或者在线程停止时处理
	// 这里不直接 delete serialPort，因为它可能在其他地方被引用
}

/**
 * @brief SerialInfo类的析构函数。
 * 如果串口已打开，则关闭串口并释放资源。
 */
SerialInfo::~SerialInfo()
{
	if (serialReadThread->isRunning()) {
		serialReadThread->quit();
		serialReadThread->wait(); // 等待线程结束
	}
	if (serialPort) {
		if (serialPort->isOpen()) {
			serialPort->close();
		}
		delete serialPort;
		serialPort = nullptr;
	}
	// serialReadThread 会在 finished 信号中 deleteLater
}

/**
 * @brief 设置串口名称。
 * @param SerialName 包含串口名称的字符串 (例如 "COM3  Some description")。
 */
void SerialInfo::SetSerialPort(QString SerialName)
{
	this->portName = ExtractPortName(SerialName);
}

/**
 * @brief 配置串口参数。
 * 如果serialPort对象为空，则创建一个新的QSerialPort实例。
 * 然后设置波特率、数据位、停止位、校验位和端口名称。
 */
void SerialInfo::ConfigureSerialPort()
{
	if (serialPort == nullptr)
	{
		serialPort = new QSerialPort();
	}
	serialPort->setBaudRate(baudRate);
	serialPort->setDataBits(dataBits);
	serialPort->setStopBits(stopBits);
	serialPort->setParity(parity);
	serialPort->setPortName(portName);
}

/**
 * @brief 设置串口波特率。
 * @param baudRate 要设置的波特率值 (例如 9600, 19200 等)。
 * @throw std::invalid_argument 如果提供的波特率无效。
 */
void SerialInfo::SetBaudRate(qint32 baudRate)
{
	if (baudRate == 9600)
	{
		this->baudRate = QSerialPort::Baud9600;
	}
	else if (baudRate == 19200)
	{
		this->baudRate = QSerialPort::Baud19200;
	}
	else if (baudRate == 38400)
	{
		this->baudRate = QSerialPort::Baud38400;
	}
	else if (baudRate == 57600)
	{
		this->baudRate = QSerialPort::Baud57600;
	}
	else if (baudRate == 115200)
	{
		this->baudRate = QSerialPort::Baud115200;
	}
	else
	{
		// 抛出异常
		throw std::invalid_argument("Invalid baud rate provided.");
	}

	qDebug() << "BaudRate:" << static_cast<int>(this->baudRate);
}

/**
 * @brief 设置串口数据位。
 * @param dataBits 要设置的数据位值 (5, 6, 7, 或 8)。
 */
void SerialInfo::SetDataBits(qint32 dataBits)
{
	this->dataBits = QSerialPort::Data8; // 默认值
	if (dataBits == 5)
		this->dataBits = QSerialPort::Data5;
	else if (dataBits == 6)
		this->dataBits = QSerialPort::Data6;
	else if (dataBits == 7)
		this->dataBits = QSerialPort::Data7;
	else if (dataBits == 8)
		this->dataBits = QSerialPort::Data8;
	qDebug() << "dataBits:" << static_cast<int>(this->dataBits);
}

/**
 * @brief 设置串口停止位。
 * @param stopBits 要设置的停止位值 (1, 1.5, 或 2)。
 */
void SerialInfo::SetStopBits(qint32 stopBits)
{
	this->stopBits = QSerialPort::OneStop; // 默认值
	if (stopBits == 1)
		this->stopBits = QSerialPort::OneStop;
	else if (stopBits == 1.5)
		this->stopBits = QSerialPort::OneAndHalfStop;
	else if (stopBits == 2)
		this->stopBits = QSerialPort::TwoStop;
	qDebug() << "stopBits:" << static_cast<int>(this->stopBits);
}

/**
 * @brief 设置串口校验位。
 * @param parityStr 要设置的校验位字符串 ("None", "Even", "Odd", "Space", "Mark")。
 */
void SerialInfo::SetParity(QString parityStr)
{
	parity = QSerialPort::NoParity; // 默认值
	this->parity = QSerialPort::NoParity;
	if (parityStr == "None")
		parity = QSerialPort::NoParity;
	else if (parityStr == "Even")
		parity = QSerialPort::EvenParity;
	else if (parityStr == "Odd")
		parity = QSerialPort::OddParity;
	else if (parityStr == "Space")
		parity = QSerialPort::SpaceParity;
	else if (parityStr == "Mark")
		parity = QSerialPort::MarkParity;
}

/**
 * @brief 一次性设置所有串口配置信息。
 * @param baudRate 波特率。
 * @param dataBits 数据位。
 * @param stopBits 停止位。
 * @param parity 校验位字符串。
 * @param SerialName 串口名称字符串。
 */
void SerialInfo::SetSerialConfiguration(qint32 baudRate, qint32 dataBits,
	qint32 stopBits, QString parity, QString SerialName)
{
	// 调用各个单独的设置函数来更新成员变量
	// SetBaudRate 内部包含对波特率有效性的检查
	SetBaudRate(baudRate);
	SetDataBits(dataBits);
	SetStopBits(stopBits);
	SetParity(parity);
	SetSerialPort(SerialName);
	// 应用新的配置到当前的串口对象 (如果存在)
	ConfigureSerialPort();
}

/**
 * @brief 获取当前的QSerialPort对象。
 * @return 返回指向QSerialPort对象的指针。
 */
QSerialPort* SerialInfo::GetSerialPort()
{
	return serialPort;
}

/**
 * @brief 更改串口的打开/关闭状态。
 * @param currentState 当前串口是否打开的状态 (true表示已打开, false表示已关闭)。
 * @return 返回更新后的串口状态 (true表示已打开, false表示已关闭)。
 * @throw std::runtime_error 如果打开串口失败。
 */
bool SerialInfo::SerialChangestate(bool currentState)
{
	if (serialPort == nullptr)
	{
		// serialPort 应该在线程启动时创建，这里只是一个备用检查
		serialPort = new QSerialPort();
		ConfigureSerialPort();
	}

	if (currentState == false) // 尝试打开串口
	{
		if (serialPort->open(QIODevice::ReadWrite | QIODevice::ExistingOnly))
		{
			qDebug() << "Serial port opened successfully.";
			// 启动线程来处理 readyRead 信号
			if (!serialReadThread->isRunning()) {
				serialReadThread->start();
			}
			emit SerialStateChanged(true); // 发出串口状态改变信号
			return true;
		}
		qDebug() << "Failed to open serial port:" << serialPort->errorString();
		throw std::runtime_error("Failed to open serial port.");
		// serialPort->close(); // 如果打开失败，不需要关闭，因为它可能根本没打开
		return false;
	}
	else // currentState == true，尝试关闭串口
	{
		if (serialPort->isOpen())
		{
			// 断开 readyRead 信号连接，防止在关闭过程中继续触发
			disconnect(serialPort, &QSerialPort::readyRead, this, &SerialInfo::handleReadyRead);

			serialPort->close();
			qDebug() << "Serial port closed.";

			// 停止线程
			if (serialReadThread->isRunning()) {
				serialReadThread->quit();
				serialReadThread->wait(); // 等待线程结束
			}
			emit SerialStateChanged(false); // 发出串口状态改变信号
			return false;
		}

		qDebug() << "Serial port is already closed.";
		emit SerialStateChanged(false); // 即使已经关闭，也发出信号
		return false;
	}
}

/**
 * @brief 通过串口发送消息。
 * @param Mess 要发送的字符串消息。
 * @throw std::runtime_error 如果串口未初始化、未打开或写入失败。
 */
void SerialInfo::SerialSendMessage(QString Mess)
{
	// 检查串口是否已初始化
	if (serialPort == nullptr)
	{
		throw std::runtime_error("Serial port is not initialized.");
	}

	// 检查串口是否已打开
	if (!serialPort->isOpen())
	{
		throw std::runtime_error("Serial port is not open.");
	}

	// 将消息转换为字节数组并发送
	auto data = Mess.toLatin1();
	qint64 bytesWritten = serialPort->write(data);

	// 检查是否成功写入
	if (bytesWritten == -1)
	{
		throw std::runtime_error("Failed to write to the serial port.");
	}

	qDebug() << "Message sent:" << Mess;
}

/**
 * @brief 处理串口 readyRead 信号的槽函数。
 * 读取所有可用数据并通过 DataReceived 信号发出。
 */
void SerialInfo::handleReadyRead()
{
	if (serialPort && serialPort->isOpen() && serialPort->isReadable())
	{
		QByteArray data = serialPort->readAll();
		if (!data.isEmpty())
		{
			emit DataReceived(data);
			qDebug() << "Data received from serial port:" << data;
		}
	}
}

/**
 * @brief 启动串口读取线程的槽函数。
 * 此槽函数将由外部调用，以启动串口数据接收线程。
 */
void SerialInfo::startSerialReadThread()
{
	// 实际的线程启动逻辑已经在构造函数和 SerialChangestate 中处理
	// 这个槽函数可以作为外部触发线程启动的接口，但其内部逻辑可能很简单，
	// 或者用于确保对象在正确线程中被初始化。
	// 由于 SerialInfo 已经移动到 serialReadThread，这个槽函数将在 serialReadThread 中执行。
	// 确保 serialPort 在此线程中被创建和使用。
	if (serialPort == nullptr) {
		serialPort = new QSerialPort();
		ConfigureSerialPort();
	}
	// 连接 readyRead 信号到 handleReadyRead 槽
	connect(serialPort, &QSerialPort::readyRead, this, &SerialInfo::handleReadyRead);
	qDebug() << "Serial read thread started and readyRead connected.";
}

/**
 * @brief 从包含描述的完整串口信息字符串中提取端口名称。
 * 例如：
 *  - 输入字符串为 "COM3  Some description" 或 "COM13  Another description"。
 *  - 提取的端口名称为 "COM3" 或 "COM13"。
 * @param input 包含串口名称和可选描述的输入字符串。
 * @return 提取的端口名称 (例如 "COM3")，如果未找到匹配则返回空字符串或输入字符串本身（取决于具体实现）。
 */
QString SerialInfo::ExtractPortName(const QString& input)
{
	// 定义正则表达式，匹配从字符串开头到第一个空格之前的内容
	QRegularExpression regex(R"(^\S+)");
	QRegularExpressionMatch match = regex.match(input);

	if (match.hasMatch())
	{
		QString portName = match.captured(0); // 提取匹配的内容
		qDebug() << "Extracted Port Name:" << portName;
		return portName;
	}
	else
	{
		qDebug() << "No match found!";
	}
	return ""; // Added return statement for no match
}