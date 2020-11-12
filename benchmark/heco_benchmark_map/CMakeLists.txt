cmake_minimum_required(VERSION 3.13)

project(benchmark_map)

add_executable(benchmark_map "benchmark_map.cpp")
target_compile_features(benchmark_map PRIVATE cxx_std_17)
target_include_directories(benchmark_map PRIVATE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

find_package(Boost REQUIRED)
target_include_directories(benchmark_map PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(benchmark_map PRIVATE ${Boost_LIBRARIES})
find_package(absl CONFIG REQUIRED)
target_link_libraries(benchmark_map PRIVATE absl::base absl::hash absl::flat_hash_map)
find_path(Hopscotch_INCLUDE_DIR NAMES hopscotch_map.h PATH_SUFFIXES tsl)
target_include_directories(benchmark_map PRIVATE ${Hopscotch_INCLUDE_DIR})
find_path(ROBIN_INCLUDE_DIR NAMES robin_map.h PATH_SUFFIXES tsl)
target_include_directories(benchmark_map PRIVATE ${ROBIN_INCLUDE_DIR})
find_path(SPP_INCLUDE_DIR NAMES spp PATH_SUFFIXES sparsepp)
target_include_directories(benchmark_map PRIVATE ${SPP_INCLUDE_DIR})

find_package(benchmark REQUIRED)
target_link_libraries(benchmark_map PRIVATE benchmark::benchmark benchmark::benchmark_main)

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
if(WIN32)
    target_compile_options(benchmark_map PRIVATE /clang:-frelaxed-template-template-args)
else()
    target_compile_options(benchmark_map PRIVATE -frelaxed-template-template-args)
endif()
endif()