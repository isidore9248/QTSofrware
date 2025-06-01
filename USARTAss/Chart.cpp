/*
 * @Description:
 * @Version: v1.0.0
 * @Author: isidore-chen
 * @Date: 2025-05-10 10:23:05
 * @Copyright: Copyright (c) 2025 CAUC
 */
#include "Chart.h"
#include <QDebug>
#include <iostream>

 /**
  * @brief Chart类的构造函数。
  * @param parent 父QObject对象。
  */
Chart::Chart(QObject* parent) : QObject(parent)
{
	for (int i = 0; i < NUM_CHARTS; ++i)
	{
		m_charts[i] = new QChart();
		m_series[i] = new QSplineSeries(this);
		initializeChart(i, "detect " + QString::number(i + 1));

		for (int k = 0; k < PointSize; ++k)
		{
			m_series[i]->append(k, static_cast<qreal>(lineSeriesIsEmpty::LineSeriesIsEmpty));
		}

		if (i == 0)
		{
			connect(m_series[0], &QSplineSeries::hovered, this, &Chart::onSeries1Hovered);
		}
		else if (i == 1)
		{
			connect(m_series[1], &QSplineSeries::hovered, this, &Chart::onSeries2Hovered);
		}
		else if (i == 2)
		{
			connect(m_series[2], &QSplineSeries::hovered, this, &Chart::onSeries3Hovered);
		}
	}
}

/**
 * @brief Chart类的析构函数。
 */
Chart::~Chart()
{
}

/**
 * @brief 初始化指定的图表。
 * @param chartIndex 要初始化的图表的索引。
 * @param seriesName 图表系列的名称。
 */
void Chart::initializeChart(int chartIndex, const QString& seriesName)
{
	if (chartIndex < 0 || chartIndex >= NUM_CHARTS)
	{
		qWarning() << "initializeChart: Invalid chartIndex" << chartIndex;
		return;
	}

	m_series[chartIndex]->setName(seriesName);
	m_charts[chartIndex]->addSeries(m_series[chartIndex]);

	QValueAxis* axisX = new QValueAxis();
	axisX->setRange(0, PointSize - 1);
	axisX->setGridLineVisible(true);
	axisX->setTickCount(6);
	axisX->setLabelFormat("%d");

	QValueAxis* axisY = new QValueAxis();
	axisY->setRange(-1, 10);
	axisY->setGridLineVisible(true);
	axisY->setTickCount(6);
	axisY->setLabelFormat("%.1f");

	m_charts[chartIndex]->addAxis(axisX, Qt::AlignBottom);
	m_series[chartIndex]->attachAxis(axisX);

	m_charts[chartIndex]->addAxis(axisY, Qt::AlignLeft);
	m_series[chartIndex]->attachAxis(axisY);

	//m_charts[chartIndex]->legend()->hide(); // Hide legend if not needed

	qDebug() << "Chart" << chartIndex << "initialized with series:" << seriesName;
}

/**
 * @brief 获取指定索引的图表对象。
 * @param chartIndex 图表的索引。
 * @return 返回指向QChart对象的指针，如果索引无效则返回nullptr。
 */
QChart* Chart::GetChart(int chartIndex)
{
	if (chartIndex >= 0 && chartIndex < NUM_CHARTS)
	{
		return m_charts[chartIndex];
	}
	qWarning() << "GetChart: Invalid chartIndex" << chartIndex;
	return nullptr;
}

/**
 * @brief 刷新所有图表的数据到初始状态。
 */
void Chart::RefreshChartData()
{
	for (int i = 0; i < NUM_CHARTS; ++i)
	{
		if (m_series[i])
		{
			m_series[i]->clear();
			for (int k = 0; k < PointSize; ++k)
			{
				m_series[i]->append(k, static_cast<qreal>(lineSeriesIsEmpty::LineSeriesIsEmpty));
			}
		}
	}
	qDebug() << "All chart data refreshed to initial state.";
}

/**
 * @brief向指定图表添加新数据点。
 *
 * 此函数将新值添加到指定图表系列中。
 * 它首先将所有现有数据点向右移动一个位置，
 * 然后在系列的开头插入新值。
 * 如果系列的点数少于PointSize，它会首先用默认值填充系列。
 *
 * @param chartIndex 要添加数据的图表的索引。
 * @param value 要添加的浮点数值。
 */
void Chart::AddToChartData(int chartIndex, float value)
{
	if (chartIndex < 0 || chartIndex >= NUM_CHARTS || !m_series[chartIndex])
	{
		qWarning() << "AddToChartData: Invalid chartIndex or null series for index" << chartIndex;
		return;
	}

	QSplineSeries* targetSeries = m_series[chartIndex];
	QList<QPointF> points = targetSeries->points();

	//未被填充完整
	if (points.size() < PointSize) {
		points.clear();
		for (int i = 0; i < PointSize; ++i) {
			points.append(QPointF(i, static_cast<qreal>(lineSeriesIsEmpty::LineSeriesIsEmpty)));
		}
	}

	for (int i = PointSize - 1; i > 0; --i)
	{
		points[i].setY(points[i - 1].y());
	}

	if (!points.isEmpty()) {
		points[0].setY(value);
	}
	else {
		points.append(QPointF(0, value));
	}

	for (int i = 0; i < points.size() && i < PointSize; ++i) {
		points[i].setX(i);
	}

	targetSeries->replace(points);

	// qDebug() << "Chart" << chartIndex << "Data added:" << value << "New series size:" << targetSeries->count();
}

/**
 * @brief 处理第一个图表系列上鼠标悬停事件的槽函数。
 * @param point 悬停点的坐标。
 * @param state 如果鼠标在系列上悬停，则为true，否则为false。
 */
void Chart::onSeries1Hovered(const QPointF& point, bool state)
{
	if (state)
	{
		emit hoveredCoordinatesChanged(0, point);
	}
}

/**
 * @brief 处理第二个图表系列上鼠标悬停事件的槽函数。
 * @param point 悬停点的坐标。
 * @param state 如果鼠标在系列上悬停，则为true，否则为false。
 */
void Chart::onSeries2Hovered(const QPointF& point, bool state)
{
	if (state)
	{
		emit hoveredCoordinatesChanged(1, point);
	}
}

/**
 * @brief 处理第三个图表系列上鼠标悬停事件的槽函数。
 * @param point 悬停点的坐标。
 * @param state 如果鼠标在系列上悬停，则为true，否则为false。
 */
void Chart::onSeries3Hovered(const QPointF& point, bool state)
{
	if (state)
	{
		emit hoveredCoordinatesChanged(2, point);
	}
}