/*
 * @Description: 
 * @Version: v1.0.0
 * @Author: isidore-chen
 * @Date: 2025-06-01 23:26:24
 * @Copyright: Copyright (c) 2025 CAUC
 */
#include "MySoftware.h"
#include <QtWidgets/QApplication>
#include "USARTAss.h"

/**
 * @brief 应用程序的入口点。
 *
 * 此函数初始化 QApplication，创建并显示主窗口，然后启动应用程序事件循环。
 *
 * @param argc 命令行参数的数量。
 * @param argv 命令行参数的数组。
 * @return 应用程序的退出代码。
 */
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    USARTAss window;
    window.show();
    return app.exec();
}
