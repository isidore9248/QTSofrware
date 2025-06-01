/*
 * @Description:
 * @Version: v1.0.0
 * @Author: isidore-chen
 * @Date: 2025-05-10 10:23:03
 * @Copyright: Copyright (c) 2025 CAUC
 */
#pragma once

#include <QtCore/QObject> // Added for QObject inheritance
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QValueAxis>
 //using namespace QtCharts; // Qt Charts 命名空间

 /**
  * @brief Chart类用于管理和显示图表。
  *
  * 此类提供创建、更新和访问多个图表的功能。
  * 它还处理图表上的鼠标悬停事件。
  */
class Chart : public QObject // Inherit from QObject
{
	Q_OBJECT // Add Q_OBJECT macro

public:
	/**
	 * @brief Chart类的构造函数。
	 * @param parent 父QObject对象。
	 */
	Chart(QObject* parent = nullptr);
	/**
	 * @brief Chart类的析构函数。
	 */
	~Chart();

signals:
	/**
	 * @brief 当鼠标悬停在图表系列上时发出的信号。
	 * @param chartIndex 发生悬停事件的图表的索引。
	 * @param point 悬停点的坐标。
	 */
	void hoveredCoordinatesChanged(int chartIndex, QPointF point);

public:
	/**
	 * @brief 获取Chart类的单例实例。
	 * @return 返回Chart类的静态实例引用。
	 */
	static Chart& GetInstance()
	{
		static Chart instance; // 使用局部静态变量实现单例
		return instance;
	}

	/**
	 * @brief 获取指定索引的图表对象。
	 * @param chartIndex 图表的索引。
	 * @return 返回指向QChart对象的指针，如果索引无效则返回nullptr。
	 */
	QChart* GetChart(int chartIndex);
	/**
	 * @brief 刷新所有图表的数据到初始状态。
	 */
	void RefreshChartData();
	/**
	 * @brief 向指定图表添加新数据点。
	 * @param chartIndex 要添加数据的图表的索引。
	 * @param value 要添加的浮点数值。
	 */
	void AddToChartData(int chartIndex, float value);

private slots:
	/**
	 * @brief 处理第一个图表系列上鼠标悬停事件的槽函数。
	 * @param point 悬停点的坐标。
	 * @param state 如果鼠标在系列上悬停，则为true，否则为false。
	 */
	void onSeries1Hovered(const QPointF& point, bool state);
	/**
	 * @brief 处理第二个图表系列上鼠标悬停事件的槽函数。
	 * @param point 悬停点的坐标。
	 * @param state 如果鼠标在系列上悬停，则为true，否则为false。
	 */
	void onSeries2Hovered(const QPointF& point, bool state);

	void onSeries3Hovered(const QPointF& point, bool state);

private:
	/**
	 * @brief 枚举，用于表示折线图系列是否为空的特殊值。
	 */
	enum lineSeriesIsEmpty
	{
		LineSeriesIsEmpty = -1, /**< 表示系列为空的值。 */
	};
#define PointSize 60 /**< 定义图表上点的数量。 */

	/**
	 * @brief 初始化指定的图表。
	 * @param chartIndex 要初始化的图表的索引。
	 * @param seriesName 图表系列的名称。
	 */
	void initializeChart(int chartIndex, const QString& seriesName);

private:
	static const int NUM_CHARTS = 3; /**< 管理的图表数量。 */
	QChart* m_charts[NUM_CHARTS];
	QSplineSeries* m_series[NUM_CHARTS];
};
