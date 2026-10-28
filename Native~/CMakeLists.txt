cmake_minimum_required(VERSION 3.15)

project(TinyExrUnity LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(TINYEXR_UNITY_BUILD_TESTS "Build native smoke tests" OFF)

if(TINYEXR_UNITY_BUILD_TESTS)
    enable_testing()
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    set(TINYEXR_UNITY_LIBRARY_TYPE STATIC)
else()
    set(TINYEXR_UNITY_LIBRARY_TYPE SHARED)
endif()

add_subdirectory(src)
