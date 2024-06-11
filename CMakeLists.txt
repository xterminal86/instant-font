cmake_minimum_required(VERSION 3.12)

set (TARGET_NAME instant-font)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED On)

project(${TARGET_NAME})

add_compile_options(-Wall
                      #-Wextra
                      -Wno-switch
                      -Wno-format
                      -Werror=return-type
                      -Wfatal-errors)

file(GLOB SOURCES "*.cpp" "*.h")

add_executable(${TARGET_NAME} ${SOURCES})

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

if (WIN32)
  target_link_libraries(${TARGET_NAME} ${MINGW32_LIBRARY}
                                         ${SDL2MAIN_LIBRARY}
                                         ${SDL2_LIBRARY})
else()
  target_link_libraries(${TARGET_NAME} SDL2)
endif()

