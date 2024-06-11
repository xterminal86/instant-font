cmake_minimum_required(VERSION 3.12)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED On)

project(instant-text)

find_package(SDL2 REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS})

file(GLOB SOURCES "*.cpp" "*.h")

add_executable(main ${SOURCES})

target_link_libraries(main ${SDL2_LIBRARIES})
