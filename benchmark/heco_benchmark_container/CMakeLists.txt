cmake_minimum_required(VERSION 3.13)

project(heco_benchmark)
add_executable(benchmark "benchmark.cpp")
target_compile_features(benchmark PRIVATE cxx_std_17)
target_include_directories(benchmark PRIVATE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_sources(benchmark PRIVATE "benchmark.cpp" "benchmark.h" "HeteroVector.h" "entt.hpp" "entt_context.hpp" )
target_include_directories(benchmark PRIVATE ${PROJECT_SOURCE_DIR}/../src)

find_package(benchmark CONFIG REQUIRED)
target_link_libraries(benchmark PRIVATE benchmark::benchmark benchmark::benchmark_main)

find_path(ROBIN_INCLUDE_DIR NAMES robin_map.h PATH_SUFFIXES tsl)
target_include_directories(benchmark PRIVATE ${ROBIN_INCLUDE_DIR})

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
if(WIN32)
    target_compile_options(benchmark PRIVATE /clang:-frelaxed-template-template-args)
else()
    target_compile_options(benchmark PRIVATE -frelaxed-template-template-args)
endif()
endif()

