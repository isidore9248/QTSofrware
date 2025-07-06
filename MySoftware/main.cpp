/*
 * @Description:
 * @Version: v1.0.0
 * @Author: isidore-chen
 * @Date: 2025-06-01 23:26:24
 * @Copyright: Copyright (c) 2025 CAUC
 */
#include <Windows.h>
#include <DbgHelp.h>
#include <string>
#include <iostream>
#include <ctime>
#include <iomanip> // For std::put_time
#include <sstream> // For std::ostringstream

#pragma comment(lib, "Dbghelp.lib")

#include "MySoftware.h"
#include <QtWidgets/QApplication>
#include "USARTAss.h"

// 定义一个回调函数，用于在程序崩溃时生成dump文件
LONG WINAPI CreateMiniDump(EXCEPTION_POINTERS* pep)
{
    // 获取当前时间，用于命名dump文件
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << "crash_dump_" << std::put_time(&tm, "%Y%m%d_%H%M%S") << ".dmp";
    std::string dumpFileName = oss.str();

    // 创建dump文件
    HANDLE hFile = CreateFileA(
        dumpFileName.c_str(),
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile != INVALID_HANDLE_VALUE)
    {
        MINIDUMP_EXCEPTION_INFORMATION mdei;
        mdei.ThreadId = GetCurrentThreadId();
        mdei.ExceptionPointers = pep;
        mdei.ClientPointers = FALSE;

        // 写入dump文件
        MiniDumpWriteDump(
            GetCurrentProcess(),
            GetCurrentProcessId(),
            hFile,
            MiniDumpNormal, // 可以根据需要选择不同的dump类型
            &mdei,
            NULL,
            NULL
        );

        CloseHandle(hFile);
        std::cerr << "Dump file created: " << dumpFileName << std::endl;
    }
    else
    {
        std::cerr << "Failed to create dump file. Error: " << GetLastError() << std::endl;
    }

    // 返回EXCEPTION_CONTINUE_SEARCH表示继续搜索下一个异常处理程序
    // 返回EXCEPTION_EXECUTE_HANDLER表示异常已处理，程序将终止
    return EXCEPTION_EXECUTE_HANDLER;
}

 /**
  * @brief 应用程序的入口点。
  *
  * 此函数初始化 QApplication，创建并显示主窗口，然后启动应用程序事件循环。
  *
  * @param argc 命令行参数的数量。
  * @param argv 命令行参数的数组。
  * @return 应用程序的退出代码。
  */
int main(int argc, char* argv[])
{
    SetUnhandledExceptionFilter(CreateMiniDump); // 注册异常处理函数

	QApplication app(argc, argv);
	USARTAss window;
	window.show();

	return app.exec();
}
