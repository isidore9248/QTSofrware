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
SerialInfo::SerialInfo() : dataBits(QSerialPort::Data8), stopBits(QSerialPort::OneStop),
parity(QSerialPort::NoParity), serialPort(nullptr)
{
}

/**
 * @brief SerialInfo类的析构函数。
 * 如果串口已打开，则关闭串口并释放资源。
 */
SerialInfo::~SerialInfo()
{
	if (serialPort)
	{ // 检查指针是否有效
		if (serialPort->isOpen())
		{
			serialPort->close(); // 在删除前关闭串口（如果已打开）
		}
		delete serialPort;      // 释放内存
		serialPort = nullptr;   // 将指针置空，避免悬挂指针 (好习惯)
	}
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
	if (dataBits == 5) this->dataBits = QSerialPort::Data5;
	else if (dataBits == 6) this->dataBits = QSerialPort::Data6;
	else if (dataBits == 7) this->dataBits = QSerialPort::Data7;
	else if (dataBits == 8) this->dataBits = QSerialPort::Data8;
	qDebug() << "dataBits:" << static_cast<int>(this->dataBits);
}

/**
 * @brief 设置串口停止位。
 * @param stopBits 要设置的停止位值 (1, 1.5, 或 2)。
 */
void SerialInfo::SetStopBits(qint32 stopBits)
{
	this->stopBits = QSerialPort::OneStop; // 默认值
	if (stopBits == 1) this->stopBits = QSerialPort::OneStop;
	else if (stopBits == 1.5) this->stopBits = QSerialPort::OneAndHalfStop;
	else if (stopBits == 2) this->stopBits = QSerialPort::TwoStop;
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
	if (parityStr == "None") parity = QSerialPort::NoParity;
	else if (parityStr == "Even") parity = QSerialPort::EvenParity;
	else if (parityStr == "Odd") parity = QSerialPort::OddParity;
	else if (parityStr == "Space") parity = QSerialPort::SpaceParity;
	else if (parityStr == "Mark") parity = QSerialPort::MarkParity;
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
		serialPort = new QSerialPort();
	}
	if (currentState == false)
	{
		if (serialPort->open(QIODevice::ReadWrite|QIODevice::ExistingOnly))
		{
			qDebug() << "Serial port opened successfully.";
			return true;
		}
		qDebug() << "Failed to open serial port:" << serialPort->errorString();
		throw std::runtime_error("Failed to open serial port.");
		serialPort->close();
		return false;
	}
	if (currentState == true)
	{
		if (serialPort->isOpen())
		{
			serialPort->close();
			qDebug() << "Serial port closed.";
			return false;
		}

		qDebug() << "Serial port is already closed.";
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
 * @brief 从串口接收消息。
 * @param totalBytes 用于累加接收到的总字节数的引用。
 * @return 返回包含接收数据的QByteArray。
 */
QByteArray SerialInfo::SerialRecvMessage(qint64& totalBytes)
{
	QByteArray retbuffer;
	// 检查串口指针是否有效并且串口是否已打开且可读
	if (serialPort && serialPort->isOpen() && serialPort->isReadable())
	{
		// 获取接收到的总字节数
		totalBytes += serialPort->bytesAvailable();

		retbuffer = serialPort->readAll();
		qDebug() << "Data read from serial port:" << retbuffer; // 添加日志，方便调试
	}
	else
	{
		// 如果串口未打开或不可读，可以记录一个警告或错误
		if (serialPort)
		{
			// 检查 serialPort 是否为 nullptr
			qDebug() << "Serial port is not open or not readable. isOpen:" << serialPort->isOpen() << "isReadable:" << serialPort->isReadable();
		}
		else
		{
			qDebug() << "Serial port object is null.";
		}
	}
	return retbuffer;
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
