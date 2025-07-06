/*
 * @Description:
 * @Version: v1.0.0
 * @Author: isidore-chen
 * @Date: 2025-05-04 22:15:09
 * @Copyright: Copyright (c) 2025 CAUC
 */
#pragma once
#include "ui_USARTAss.h"
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QtWidgets/QDialog>
#include <QtCore/QtGlobal>
#include <vector>
#include <QtCore/QDebug>
#include <QtCharts/QChartView> // 添加此行以包含 QChartView 的定义
#include <QThread>

 /**
  * @brief SerialInfo类用于管理串口通信的配置和操作。
  *
  * 此类提供设置串口参数（如波特率、数据位、停止位、校验位）、
  * 打开/关闭串口、发送和接收数据的功能。
  * 它采用单例模式确保全局只有一个串口配置实例。
  */
class SerialInfo : public QObject
{
	Q_OBJECT // 添加 Q_OBJECT 宏

public:
	/**
	 * @brief SerialInfo类的构造函数。
	 */
	SerialInfo(QObject* parent = nullptr);
	/**
	 * @brief SerialInfo类的析构函数。
	 */
	~SerialInfo();

	/**
	 * @brief 一次性设置所有串口配置信息，并保存在内部变量中。
	 * @param baudRate 波特率。
	 * @param dataBits 数据位。
	 * @param stopBits 停止位。
	 * @param parity 校验位字符串。
	 * @param SerialName 串口名称字符串 (例如 "COM3  Some description")。
	 */
	void SetSerialConfiguration(qint32 baudRate, qint32 dataBits, qint32 stopBits,
		QString parity, QString SerialName);
	/**
	 * @brief 获取当前的QSerialPort对象。
	 * @return 返回指向QSerialPort对象的指针。
	 */
	QSerialPort* GetSerialPort();

	/**
	 * @brief 更改串口的打开/关闭状态。
	 * @param currentState 当前串口是否打开的状态 (true表示已打开, false表示已关闭)。
	 * @return 返回更新后的串口状态 (true表示已打开, false表示已关闭)。
	 */
	bool SerialChangestate(bool currentState);

	/**
	 * @brief 通过串口发送消息。
	 * @param Mess 要发送的字符串消息。
	 */
	void SerialSendMessage(QString Mess);
	// 删除 SerialRecvMessage 方法，数据接收将通过 readyRead 信号触发，并在槽函数中处理

signals:
	/**
	 * @brief 串口数据接收完成后发出的信号。
	 * @param data 接收到的数据。
	 */
	void DataReceived(const QByteArray& data);
	// 添加一个信号，用于通知外部串口已打开/关闭
	void SerialStateChanged(bool isOpen);

public slots:
	//void SerialDatadisposed(float data);
	// 添加一个槽函数，用于处理串口的 readyRead 信号
	void handleReadyRead();
	// 添加一个槽函数，用于在单独线程中启动数据接收循环
	void startSerialReadThread();

private:
	/**
	 * @brief 从包含描述的完整串口信息字符串中提取端口名称。
	 * @param input 包含串口名称和可选描述的输入字符串。
	 * @return 提取的端口名称。
	 */
	QString ExtractPortName(const QString& input);

	/**
	 * @brief 配置串口参数。
	 */
	void ConfigureSerialPort();
	/**
	 * @brief 设置串口名称。
	 * @param SerialName 包含串口名称的字符串。
	 */
	void SetSerialPort(QString SerialName);
	/**
	 * @brief 设置串口波特率。
	 * @param baudRate 波特率值。
	 */
	void SetBaudRate(qint32 baudRate);
	/**
	 * @brief 设置串口数据位。
	 * @param dataBits 数据位值。
	 */
	void SetDataBits(qint32 dataBits);
	/**
	 * @brief 设置串口停止位。
	 * @param stopBits 停止位值。
	 */
	void SetStopBits(qint32 stopBits);
	/**
	 * @brief 设置串口校验位。
	 * @param parity 校验位字符串。
	 */
	void SetParity(QString parity);

public:
	// 串口配置相关成员变量
	QSerialPort* serialPort;        /**< 指向QSerialPort对象的指针。 */
	QString portName;               /**< 串口名称 (例如 "COM3")。 */
	QSerialPort::DataBits dataBits; /**< 数据位。 */
	QSerialPort::StopBits stopBits; /**< 停止位。 */
	QSerialPort::Parity parity;     /**< 奇偶校验位。 */
	QSerialPort::BaudRate baudRate; /**< 波特率。 */

private:
	QThread* serialReadThread; // 用于串口读取的线程
};
