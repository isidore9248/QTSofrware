<!--
 * @Description: 
 * @Version: v1.0.0
 * @Author: isidore-chen
 * @Date: 2025-05-11 16:06:47
 * @Copyright: Copyright (c) 2025 CAUC
-->
# QT Software
## 开发环境
- QT6.8.3 + VS2022 + C++20

## 环境搭建
- 更换安装源
    ```
    .\qt-online-installer-windows-x64-4.8.0.exe --mirror https://mirrors.ustc.edu.cn/qtproject/
    ```
- - ```QT VS　Tools->QT versions->import->D:\QT\install\6.8.3\msvc2022_64```
- vs2022细节微调：
    1. VS打开Qt的ui界面几秒后闪退（ui无法打开文件）的解决办法
    ```
    https://blog.csdn.net/weixin_32155265/article/details/114905744
    ```
    2. vs + Qt开发时qDebug()打印到“输出”窗口
    CMakeLists.txt中添加
    ``` CMakeLists
    if(WIN32)
        target_link_options(${PROJECT_NAME} PRIVATE "/SUBSYSTEM:CONSOLE")
    endif()    
    ```
