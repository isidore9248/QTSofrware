/*
 * @Description:
 * @Version: v1.0.0
 * @Author: isidore-chen
 * @Date: 2025-05-04 20:59:47
 * @Copyright: Copyright (c) 2025 CAUC
 */
#pragma once
#include <QtWidgets/QMainWindow>
#include "ui_USARTAss.h"
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QtWidgets/QDialog>
#include <QtCore/QtGlobal>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtCharts/QChartView> // 添加此行以包含 QChartView 的定义
#include <vector>
#include <memory>
#include <QThread>
#include "SerialInfo.h" // 添加 SerialInfo 头文件

QT_BEGIN_NAMESPACE
namespace UI
{
	class USARTAss;
}
QT_END_NAMESPACE

struct PID_parameters
{
	float Kp; /**< 比例系数。 */
	float Ki; /**< 积分系数。 */
	float Kd; /**< 微分系数。 */
};

/**
 * @brief USARTAss类是应用程序的主窗口类。
 *
 * 它继承自QMainWindow，并管理UI交互、串口通信逻辑和图表显示。
 * 此类处理用户输入，如打开/关闭串口、刷新串口列表、发送数据等，
 * 并通过SerialInfo类与物理串口交互，通过Chart类显示数据。
 */
class USARTAss : public QMainWindow
{
	Q_OBJECT

public:
	/**
	 * @brief USARTAss类的构造函数。
	 * @param parent 父QWidget对象，默认为nullptr。
	 */
	USARTAss(QWidget* parent = nullptr);
	/**
	 * @brief USARTAss类的析构函数。
	 */
	~USARTAss();

private slots:
	/**
	 * @brief 处理打开/关闭串口按钮点击事件的槽函数。
	 */
	void OpenCloseUSART_clicked();
	/**
	 * @brief 处理刷新串口列表按钮点击事件的槽函数。
	 */
	void RefreshUSART_clicked();
	/**
	 * @brief 处理清空发送区按钮点击事件的槽函数。
	 */
	void ClearSendSpace_clicked();

	void ClearRecvSpace_clicked();
	/**
	 * @brief 处理发送消息按钮点击事件的槽函数。
	 */
	void SendMessage_clicked();
	/**
	 * @brief 处理串口接收到数据时的信号的槽函数。
	 * @param data 接收到的数据。
	 */
	void RecvMessage_clicked(const QByteArray& data);

	/**
	 * @brief 处理打开帧检查复选框点击事件的槽函数。
	 */
	void OpenfraemCheck_on_click();
	/**
	 * @brief 处理关闭帧检查复选框点击事件的槽函数。
	 */
	void ClosefraemCheck_on_click();

signals:
	void DataDisposed(int chartIndex, float data);
	void PIDReadyToShow(size_t index, PID_parameters PIDdata); /**< 信号，表示PID数据已准备好显示。 */
private:
	/**
	 * @brief 连接所有UI控件的信号到相应的槽函数。
	 */
	void TotalConnect();

	/**
	 * @brief 从UI读取用户设置的串口配置信息。
	 */
	void ReadUsrSerialInfo();
	/**
	 * @brief 根据串口打开状态更改打开/关闭按钮的文本。
	 * @param serialOpened 布尔值，指示串口是否已打开。
	 */
	void ChangeSerialButtonText(bool serialOpened);
	/**
	 * @brief 在UI上显示已接收的总字节数。
	 */
	void ShowRecvBytesCount();

	void ShowPID(size_t index, PID_parameters PIDdata); /**< 显示PID数据的函数。 */

private:
	Ui::USARTAss ui; /**< 指向通过Qt Designer生成的UI类的实例。 */

	bool RecvCheck;
	QString EndFrame;

	std::vector<QString> ChartFrame;
	size_t FrameIndex; /**< 用于存储图表帧头的字符串数组。 */

	/**
	 * @brief 枚举，表示串口数据接收的状态。
	 */
	enum FrameState
	{
		WaitingForStart, /**< 等待接收帧头状态。 */
		WaitingForData1, /**< 等待接收第一个数据帧状态。 */
		WaitingForData2, /**< 等待接收第二个数据帧状态。 */
		WaitingForData3, /**< 等待接收第三个数据帧状态。 */
		WaitingForEnd	 /**< 等待接收帧尾状态。 */
	};

	FrameState currentState = WaitingForStart; /**< 当前串口数据接收状态。 */
	QString currentStartFrame;				   /**< 当前已接收到的帧头字符串。 */
	float currentDataFrame1;                   /**< 当前已接收到的第一个数据帧的浮点数值。 */
	float currentDataFrame2;                   /**< 当前已接收到的第二个数据帧的浮点数值。 */
	float currentDataFrame3;                   /**< 当前已接收到的第三个数据帧的浮点数值。 */

	bool serialOpened;		   /**< 布尔标志，指示串口是否已打开。 */
	QString serialSendMessage; /**< 存储待发送的串口消息。 */
	QByteArray buffer;		   /**< 用于存储从串口接收到的原始数据的缓冲区。 */
	qint64 totalBytes;		   /**< 记录从串口接收到的总字节数。 */

	SerialInfo* m_serialInfo;      /**< SerialInfo 对象。 */
};
