cmake_minimum_required(VERSION 3.20)
project(HideMe LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_APP "Build the user-mode app" OFF)
option(BUILD_DRIVER "Build the kernel driver" OFF)

if(BUILD_APP)
    add_subdirectory(app)
endif()

if(BUILD_DRIVER)
    add_subdirectory(driver)
endif()