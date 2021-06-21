cmake_minimum_required(VERSION 3.10)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if (WIN32)
  add_compile_definitions(
    _CRT_SECURE_NO_WARNINGS
    _CRT_NONSTDC_NO_DEPRECATE
    )
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "" FORCE)
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
  add_compile_definitions(_DEBUG=1 DEBUG=1)
endif()

project(main C)

find_package(ipc REQUIRED)

set(SRC
  src/main.c
  src/matrix.c
  src/util.c
  src/piece.c
  src/move.c
  src/board.c
  )

add_executable(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME} ipc)
