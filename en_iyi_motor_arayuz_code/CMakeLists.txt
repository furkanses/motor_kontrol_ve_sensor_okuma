cmake_minimum_required(VERSION 3.16)

project(MotorInterface VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets SerialPort Charts)

set(PROJECT_SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
)

add_executable(MotorInterface
    ${PROJECT_SOURCES}
)

target_link_libraries(MotorInterface PRIVATE
    Qt6::Widgets
    Qt6::SerialPort
    Qt6::Charts
)

# Handle Windows subsystem (hide console window in release)
if(WIN32)
    set_target_properties(MotorInterface PROPERTIES WIN32_EXECUTABLE ON)
endif()

file(COPY 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dark_bg.png 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/light_bg.png 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/christmas_bg.png 
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
