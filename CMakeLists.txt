cmake_minimum_required(VERSION 3.10)

project(8086)

set(CMAKE_CXX_FLAGS "-O3")
set(SOURCE_EXE main.cpp)

set(SOURCE_LIB 8086.cpp)	

add_library(fcpu STATIC ${SOURCE_LIB})

add_executable(main ${SOURCE_EXE})

target_link_libraries(main fcpu)	