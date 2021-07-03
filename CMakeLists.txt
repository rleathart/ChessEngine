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

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS YES CACHE BOOL "Export all symbols")

project(chess C)

include(CTest)
find_package(ipc REQUIRED)

include_directories(include)

set(SRC
  src/matrix.c
  src/util.c
  src/piece.c
  src/move.c
  src/board.c
  src/message.c
  src/evaluate.c
  src/search.c
  src/tree.c
  src/logging.c
  )

add_library(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME} ipc)

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE src)
install(
  TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Config
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  )
install(DIRECTORY include/ DESTINATION include)
install(EXPORT ${PROJECT_NAME}Config DESTINATION lib/cmake/${PROJECT_NAME})
export(TARGETS ${PROJECT_NAME} FILE ${PROJECT_NAME}Config.cmake)

add_executable(main src/main.c)
target_link_libraries(main ${PROJECT_NAME} ipc)

if (CHESS_BUILD_TESTS)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(CHECK REQUIRED check)
  include_directories(include ${CHECK_INCLUDE_DIRS})
  link_directories(${CHECK_LIBRARY_DIRS})

  add_executable(test_engine test/engine.c)
  target_link_libraries(test_engine ${CHECK_LIBRARIES} ${PROJECT_NAME} ipc)
  add_test(NAME engine COMMAND test_engine)
endif()
