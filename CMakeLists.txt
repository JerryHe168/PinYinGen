cmake_minimum_required(VERSION 3.10)
project(PinYinGen VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTS "Build test suite" OFF)
option(BUILD_EXAMPLES "Build examples" ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

if(MSVC)
    add_compile_options(/W4 /WX- /utf-8)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -DUNICODE -D_UNICODE)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)

add_library(pinyingen STATIC
    src/pinyin_data.cpp
    src/pinyin_converter.cpp
    src/tone_handler.cpp
    src/polyphone_handler.cpp
    src/name_generator.cpp
    src/encoding_utils.cpp
)

target_include_directories(pinyingen PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if(BUILD_EXAMPLES)
    add_executable(pinyin_demo examples/main.cpp)
    target_link_libraries(pinyin_demo PRIVATE pinyingen)
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

install(TARGETS pinyingen
    EXPORT pinyingen-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/ DESTINATION include)

install(EXPORT pinyingen-targets
    FILE pinyingen-config.cmake
    NAMESPACE pinyingen::
    DESTINATION lib/cmake/pinyingen
)
