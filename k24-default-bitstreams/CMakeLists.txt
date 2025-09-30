cmake_minimum_required(VERSION 3.20)
project(k24-default-bitstreams VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GIO REQUIRED gio-2.0)

add_executable(k24_default_bitstreams
        main.cpp)


install(TARGETS k24_default_bitstreams
        RUNTIME DESTINATION bin)

target_include_directories(k24_default_bitstreams PRIVATE ${GIO_INCLUDE_DIRS})
target_link_libraries(k24_default_bitstreams PRIVATE ${GIO_LIBRARIES})