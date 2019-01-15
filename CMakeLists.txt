cmake_minimum_required(VERSION 2.8.12)

project(ORM1_Projekat)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5Core)

file(GLOB HEADERS include/*.h include/debugging.h)
file(GLOB SOURCES source/*.c)
file(GLOB APP_SOURCES *.c)
add_library(ORM1_Projekat SHARED ${HEADERS} ${SOURCES})
foreach(executable ${APP_SOURCES})
    string(REPLACE ".c" "" name ${executable})
    add_executable(${name} ${executable})
    target_link_libraries(${name} ORM1_Projekat)
endforeach(executable ${APP_SOURCES})
