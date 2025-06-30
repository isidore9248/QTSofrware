/*
 * @Description:
 * @Version: v1.0.0
 * @Author: isidore-chen
 * @Date: 2025-06-14 21:06:44
 * @Copyright: Copyright (c) 2025 CAUC
 */
#pragma once

#include <QObject>
#include <QThread>
#include <QQueue>

class ThreadProcess : public QObject
{
	Q_OBJECT
public:
	ThreadProcess(QObject* parent = nullptr);
	~ThreadProcess();

signals:
	void FrameProcessedOver(int num); // 声明 FrameProcessedOver 信号

public:
	//static ThreadProcess& getInstance()
	//{
	//	static ThreadProcess instance;
	//	return instance;
	//}

private:
	// Pimpl idiom to hide implementation details
	class SerialThread;
	std::unique_ptr<SerialThread> m_serialThread;
};
