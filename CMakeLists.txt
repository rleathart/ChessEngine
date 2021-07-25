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

if(NOT CMAKE_PREFIX_PATH)
  set(CMAKE_PREFIX_PATH $ENV{CMAKE_PREFIX_PATH})
  # Convert from Unix path sep
  string(REPLACE ":" ";" CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}")
endif()

foreach(path ${CMAKE_PREFIX_PATH})
  include_directories(${path}/include)
  link_directories(${path}/lib)
endforeach(path)

# Just dump everything in the build dir so the executables can find dlls
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

project(ChessEngine C)

add_subdirectory(extern/librgl)
add_subdirectory(extern/libipc)

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
  )

set(CHESS_LIBS ipc rgl pthread m)
if(WIN32)
  find_package(pthread REQUIRED)
endif()

add_library(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME} ${CHESS_LIBS})

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

add_executable(ChessEngineMain src/main.c)
target_link_libraries(ChessEngineMain ${PROJECT_NAME} ${CHESS_LIBS})

if (CHESS_BUILD_TESTS)
  include(CTest)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(CHECK REQUIRED check)
  include_directories(include ${CHECK_INCLUDE_DIRS})
  link_directories(${CHECK_LIBRARY_DIRS})

  add_executable(test_engine test/engine.c)
  target_link_libraries(test_engine
    ${CHECK_LIBRARIES}
    ${PROJECT_NAME}
    ${CHESS_LIBS}
    )
  add_test(NAME engine COMMAND test_engine)
endif()
