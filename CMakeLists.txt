cmake_minimum_required(VERSION 3.9)
project(hw1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_FLAGS: "${CMAKE_CXX_FLAGS} -Wall -std=c++17 -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address,undefined -D_GLIBCXX_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

find_package(Qt5Widgets REQUIRED)
# find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

add_executable(hw1
        hw1/main.cpp
        hw1/mainwindow.cpp
        hw1/mainwindow.h
        hw1/mainwindow.ui
        directory_scanner/scanner.h
        directory_scanner/scanner.cpp
        directory_scanner/file_comparator.cpp
        directory_scanner/file_comparator.h
        directory_scanner/exceptions.h
        directory_scanner/worker.cpp
        directory_scanner/worker.h)
qt5_use_modules(hw1 Widgets)
target_link_libraries(hw1 -lpthread)

add_executable(util util/main.cpp
        directory_scanner/scanner.h
        directory_scanner/scanner.cpp
        directory_scanner/file_comparator.cpp
        directory_scanner/file_comparator.h
        directory_scanner/exceptions.h
        directory_scanner/worker.cpp
        directory_scanner/worker.h)
qt5_use_modules(util Widgets)
target_link_libraries(util -lpthread)