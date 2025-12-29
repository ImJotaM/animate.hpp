cmake_minimum_required(VERSION 3.10)
project(test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(test_app test/test.cpp)

target_include_directories(test_app PRIVATE 
    . 
    ${CMAKE_CURRENT_SOURCE_DIR}/raylib/include
)

target_link_directories(test_app PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/raylib/lib
)

target_link_libraries(test_app PRIVATE raylib)

if(WIN32)
    target_link_libraries(test_app PRIVATE winmm gdi32)
endif()

set_target_properties(test_app PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    OUTPUT_NAME "test"
)