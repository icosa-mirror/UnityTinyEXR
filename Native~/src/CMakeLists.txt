set(TINYEXR_ROOT "${CMAKE_SOURCE_DIR}/tinyexr")

set(TINYEXR_UNITY_SOURCES
    ${TINYEXR_ROOT}/deps/miniz/miniz.c
    ${TINYEXR_ROOT}/deps/miniz/miniz.h
    ${TINYEXR_ROOT}/exr_reader.hh
    ${TINYEXR_ROOT}/streamreader.hh
    ${TINYEXR_ROOT}/tinyexr.cc
    ${TINYEXR_ROOT}/tinyexr.h
    tinyexr_wrapper.cpp
)

if(WIN32)
    list(APPEND TINYEXR_UNITY_SOURCES tinyexr.def)
endif()

add_library(TinyExr ${TINYEXR_UNITY_LIBRARY_TYPE} ${TINYEXR_UNITY_SOURCES})

target_include_directories(TinyExr PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${TINYEXR_ROOT}
    ${TINYEXR_ROOT}/deps/miniz
)
target_compile_definitions(TinyExr PRIVATE TINYEXR_USE_MINIZ=1)

if(ANDROID)
    # Support 16 KB memory pages with NDK r27 and older toolchains.
    target_link_options(TinyExr PRIVATE
        "-Wl,-z,max-page-size=16384"
        "-Wl,-z,common-page-size=16384"
    )
endif()

if(MSVC)
    target_compile_definitions(TinyExr PRIVATE NOMINMAX _CRT_SECURE_NO_WARNINGS)
endif()

set(TINYEXR_UNITY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugin" CACHE PATH
    "Directory in which to place the Unity native plugin")

set_target_properties(TinyExr PROPERTIES
    OUTPUT_NAME "TinyExr"
    POSITION_INDEPENDENT_CODE ON
    RUNTIME_OUTPUT_DIRECTORY "${TINYEXR_UNITY_OUTPUT_DIRECTORY}"
    LIBRARY_OUTPUT_DIRECTORY "${TINYEXR_UNITY_OUTPUT_DIRECTORY}"
    ARCHIVE_OUTPUT_DIRECTORY "${TINYEXR_UNITY_OUTPUT_DIRECTORY}"
)

foreach(CONFIG_TYPE DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    string(TOUPPER "${CONFIG_TYPE}" CONFIG_TYPE_UPPER)
    set_target_properties(TinyExr PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} "${TINYEXR_UNITY_OUTPUT_DIRECTORY}"
        LIBRARY_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} "${TINYEXR_UNITY_OUTPUT_DIRECTORY}"
        ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} "${TINYEXR_UNITY_OUTPUT_DIRECTORY}"
    )
endforeach()

if(TINYEXR_UNITY_BUILD_TESTS AND NOT CMAKE_CROSSCOMPILING)
    add_executable(TinyExrSmokeTest tinyexr_smoke_test.cpp)
    target_link_libraries(TinyExrSmokeTest PRIVATE TinyExr)
    set_target_properties(TinyExrSmokeTest PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${TINYEXR_UNITY_OUTPUT_DIRECTORY}"
    )
    foreach(CONFIG_TYPE DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
        string(TOUPPER "${CONFIG_TYPE}" CONFIG_TYPE_UPPER)
        set_target_properties(TinyExrSmokeTest PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER}
                "${TINYEXR_UNITY_OUTPUT_DIRECTORY}"
        )
    endforeach()
    add_test(NAME TinyExrSmokeTest COMMAND TinyExrSmokeTest)
endif()
