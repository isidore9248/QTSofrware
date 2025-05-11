/*
 * @Description:
 * @Version: v1.0.0
 * @Author: isidore-chen
 * @Date: 2025-05-04 22:15:09
 * @Copyright: Copyright (c) 2025 CAUC
 */
#pragma once
#include <QtCharts/QChartView> // 添加此行以包含 QChartView 的定义

QT_CHARTS_USE_NAMESPACE // Qt Charts 命名空间

#include "ui_USARTAss.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDialog>
#include <QtGlobal>
#include <vector>
#include <QDebug>
#include <QtCharts/QChartView> // 添加此行以包含 QChartView 的定义

QT_CHARTS_USE_NAMESPACE // Qt Charts 命名空间

/**
 * @brief SerialInfo类用于管理串口通信的配置和操作。
 *
 * 此类提供设置串口参数（如波特率、数据位、停止位、校验位）、
 * 打开/关闭串口、发送和接收数据的功能。
 * 它采用单例模式确保全局只有一个串口配置实例。
 */
class SerialInfo
{
public:
	/**
	 * @brief SerialInfo类的构造函数。
	 */
	SerialInfo();
	/**
	 * @brief SerialInfo类的析构函数。
	 */
	~SerialInfo();

	/**
	 * @brief 获取SerialInfo类的单例实例。
	 * @return 返回SerialInfo类的静态实例引用。
	 */
	static SerialInfo& getInstance()
	{
		static SerialInfo instance;
		return instance;
	}

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
	/**
	 * @brief 从串口接收消息。
	 * @param totalBytes 用于累加接收到的总字节数的引用。
	 * @return 返回包含接收数据的QByteArray。
	 */
	QByteArray SerialRecvMessage(qint64& totalBytes);

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
};
