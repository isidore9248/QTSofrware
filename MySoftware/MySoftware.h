/*
 * @Description: 
 * @Version: v1.0.0
 * @Author: isidore-chen
 * @Date: 2025-06-01 23:26:24
 * @Copyright: Copyright (c) 2025 CAUC
 */
#pragma once

#include <QtWidgets/QMainWindow>

/**
 * @brief MySoftware 类的主窗口。
 *
 * 此类继承自 QMainWindow，是应用程序的主界面。
 */
class MySoftware : public QMainWindow
{
	Q_OBJECT

public:
	/**
	 * @brief MySoftware 类的构造函数。
	 * @param parent 父 QWidget 对象。
	 */
	MySoftware(QWidget* parent = nullptr);
	/**
	 * @brief MySoftware 类的析构函数。
	 */
	~MySoftware();

private:
};
