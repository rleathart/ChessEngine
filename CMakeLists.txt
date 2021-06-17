cmake_minimum_required(VERSION 3.10)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(main C)

find_package(ipc REQUIRED)

set(SRC src/main.c)

add_executable(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME} ipc)
