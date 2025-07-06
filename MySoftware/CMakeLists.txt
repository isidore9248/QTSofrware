cmake_minimum_required(VERSION 3.16)
project(MySoftware LANGUAGES CXX)

include(qt.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 全局包含路径（所有目标生效）
include_directories(
    "${CMAKE_CURRENT_BINARY_DIR}/MySoftware_autogen/include"
)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR}
    COMPONENTS
    Core
    Gui
    Widgets
    Charts
    SerialPort
)
set(CMAKE_AUTOUIC ON)
qt_standard_project_setup()

set(PROJECT_SOURCES
    main.cpp
    USARTAss.ui
    MySoftware.h
    MySoftware.cpp

    SerialInfo.cpp
    SerialInfo.h
    USARTAss.cpp
    USARTAss.h
    # ui_USARTAss.h
)

qt_add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

# 显式设置链接器标志以启用控制台
if(WIN32)
    target_link_options(${PROJECT_NAME} PRIVATE "/SUBSYSTEM:CONSOLE")
endif()

target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_autogen/include
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Widgets
    Qt::Charts
    Qt::SerialPort
)
