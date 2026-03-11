cmake_minimum_required(VERSION 3.10)
project(Tiler VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

add_library(${PROJECT_NAME} SHARED Tiler.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE include)

target_link_libraries(${PROJECT_NAME} PRIVATE
    user32
    kernel32
)

target_sources(${PROJECT_NAME} PRIVATE
    Config.h
    TilingRoutine.h
    TilingRule.h
    Utils.h
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "Tiler"
    PREFIX ""
)
