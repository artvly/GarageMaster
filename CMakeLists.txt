cmake_minimum_required(VERSION 3.21)

project(GarageMaster
    VERSION 0.1.0
    DESCRIPTION "Учёт заказ-нарядов, клиентов и запчастей для СТО"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(GM_BUILD_TESTS "Собирать модульные тесты" ON)

# Целевая версия на машине разработчика — Qt 6.5.3 (MinGW 64).
find_package(Qt6 6.4 REQUIRED COMPONENTS Core Sql Network Widgets PrintSupport)
qt_standard_project_setup()

if(MSVC)
    add_compile_options(/W4 /utf-8)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_subdirectory(src/core)
add_subdirectory(src/app)

if(GM_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
