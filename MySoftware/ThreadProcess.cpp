/*
 * @Description:
 * @Version: v1.0.0
 * @Author: isidore-chen
 * @Date: 2025-06-14 21:06:44
 * @Copyright: Copyright (c) 2025 CAUC
 */
#include "ThreadProcess.h"
#include <QDebug>
#include <QThread>
#include <QQueue>
#include <QMutex>
#include <QWaitCondition>
#include <vector>

ThreadProcess::ThreadProcess(QObject* parent)
	: QObject(parent), m_serialThread(std::make_unique<SerialThread>())
{
	m_serialThread->moveToThread(new QThread(this));
	connect(m_serialThread.get(), &SerialThread::FrameProcessedOver, this, &ThreadProcess::FrameProcessedOver);
	m_serialThread->thread()->start();
}

ThreadProcess::~ThreadProcess()
{
}

//Pimple
class ThreadProcess::SerialThread : public QObject
{
	Q_OBJECT
signals:
	void FrameProcessedOver(int num);

public:
	SerialThread(QObject* parent = nullptr) : QObject(parent), running(true), EndFrame("END"),
		ChartFrame{ "START1", "START2", "START3" }, ChartFrameIndex(-1)
	{
	}
	~SerialThread() {}

	void SerialProcess();

private:
	void enqueueData(const QByteArray& data);

	void DataMainProcess(QByteArray Currentdata);

private:
	bool running;

	static const int MAX_QUEUE_SIZE = 100; // 队列最大容量
	QWaitCondition producerCond; // 生产者条件变量
	QWaitCondition consumerCond; // 消费者条件变量
	QMutex ByteArrayMutex; // 互斥锁保护队列
	QQueue<QByteArray> SerialDataQueue;

	// 状态机相关变量
	enum FrameState
	{
		WaitingForStart, /**< 等待接收帧头状态。 */
		WaitingForData,	 /**< 等待接收数据帧状态。 */
		WaitingForEnd	 /**< 等待接收帧尾状态。 */
	};
	FrameState currentState = WaitingForStart; /**< 当前串口数据接收状态。 */
	std::vector<QString> ChartFrame;
	size_t ChartFrameIndex; /**< 用于存储图表帧头的字符串数组。 */
	QString currentStartFrame;				   /**< 当前已接收到的帧头字符串。 */
	float currentDataFrame;					   /**< 当前已接收到的数据帧的浮点数值。 */
	QString EndFrame;
};

void ThreadProcess::SerialThread::enqueueData(const QByteArray& data)
{
	QMutexLocker locker(&ByteArrayMutex); // 锁定互斥锁

	// 如果队列已满，等待直到有空间
	while (SerialDataQueue.size() >= MAX_QUEUE_SIZE) { producerCond.wait(&ByteArrayMutex); }
	SerialDataQueue.enqueue(data);
	consumerCond.wakeOne(); // 唤醒一个等待数据的消费者
}

void ThreadProcess::SerialThread::DataMainProcess(QByteArray receivedData)
{
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
			//TODO
			//ui.RecvSpace->append("Received Start Frame: " + currentStartFrame);
			emit FrameProcessedOver(1); // 发出信号表示帧头处理完成
			qDebug() << "Received Start Frame:" << currentStartFrame;
		}
		else
		{
			this->currentState = WaitingForStart;
			this->currentStartFrame.clear();
			this->currentDataFrame = 0.0f;
			//TODO
			//ui.RecvSpace->append("Invalid Start Frame: " + receivedData);
			emit FrameProcessedOver(0); // 发出信号表示数据帧处理完成
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
			//TODO
			//ui.RecvSpace->append("Received Data Frame: " + QString::number(currentDataFrame));
			emit FrameProcessedOver(2); // 发出信号表示数据帧处理完成
			qDebug() << "Received Data Frame:" << currentDataFrame;
		}
		else
		{
			this->currentState = WaitingForStart;
			this->currentStartFrame.clear();
			this->currentDataFrame = 0.0f;
			//TODO
			//ui.RecvSpace->append("Invalid Data Frame: " + receivedData);
			emit FrameProcessedOver(0); // 发出信号表示无效数据帧处理完成
			qDebug() << "Invalid Data Frame:" << receivedData;
		}
		break;
	}

	case WaitingForEnd:
	{
		if (receivedData == EndFrame)
		{
			currentState = WaitingForStart; // 切换回等待帧头状态
			//TODO
			//ui.RecvSpace->append("Received End Frame: " + receivedData);
			qDebug() << "Received End Frame:" << receivedData;

			// 处理完整数据包
			QString ShowMessage = "Complete Packet - Start: " + currentStartFrame +
				", Data: " + QString::number(currentDataFrame) +
				", End: " + receivedData;
			//TODO
			//ui.RecvSpace->append(ShowMessage);
			emit FrameProcessedOver(3);

			qDebug() << "ChartFrameIndex:" << ChartFrameIndex;
			qDebug() << "currentStartFrame:" << currentStartFrame;

			//if (ChartFrameIndex != -1)
			//{
			//	Chart::GetInstance().AddToChartData(ChartFrameIndex, currentDataFrame);
			//	ChartFrameIndex = -1;
			//}
			//else
			//{
			//}
		}
		else
		{
			this->currentState = WaitingForStart;
			this->currentStartFrame.clear();
			this->currentDataFrame = 0.0f;
			//TODO
			//ui.RecvSpace->append("Invalid End Frame: " + receivedData);
			emit FrameProcessedOver(0); // 发出信号表示无效帧尾处理完成

			qDebug() << "Invalid End Frame:" << receivedData;
		}
		break;

	default:
		//TODO
		//ui.RecvSpace->append("Unknown State");
		emit FrameProcessedOver(-1); // 发出信号表示未知状态
		qDebug() << "Unknown State";
		break;
	}
	}
}

void ThreadProcess::SerialThread::SerialProcess()
{
	while (running)
	{
		QByteArray receivedData;
		{
			QMutexLocker locker(&ByteArrayMutex);
			// 如果队列为空，等待直到有数据
			while (SerialDataQueue.isEmpty() && running)
			{ // 增加running条件，以便在停止时退出等待
				consumerCond.wait(&ByteArrayMutex);
			}
			if (!running) { // 再次检查running状态，以防被wakeAll唤醒后running变为false
				producerCond.wakeAll(); // 确保所有等待的生产者都能退出
				break;
			}

			if (!SerialDataQueue.isEmpty()) {
				receivedData = SerialDataQueue.dequeue();
				producerCond.wakeOne(); // 唤醒一个等待队列空间的生产者
			}
		}
		DataMainProcess(receivedData);
	}
}
