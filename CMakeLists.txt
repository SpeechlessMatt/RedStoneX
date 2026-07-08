cmake_minimum_required(VERSION 3.15)

project(
    RedStoneX
    VERSION 0.1.1
    DESCRIPTION "High-performance Redstone simulation library"
    LANGUAGES C
)

option(REDSTONEX_INSTALL "Install RedStoneX library" ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Library
add_library(redstonex SHARED)

target_sources(redstonex
    PRIVATE
        src/redstonex_sim.c
        src/redstonex_obj.c
        src/redstonex_components.c
)

target_include_directories(redstonex
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_features(redstonex
    PUBLIC
        c_std_11
)

set_target_properties(redstonex PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Tests
enable_testing()

add_executable(torch_relay_test
    tests/torch_relay_test.c
)

target_link_libraries(torch_relay_test
    PRIVATE
        redstonex
)

add_test(
    NAME torch_relay_test
    COMMAND torch_relay_test
)

add_executable(comparator_test
    tests/comparator_test.c
)

target_link_libraries(comparator_test
    PRIVATE
        redstonex
)

add_test(
    NAME comparator_test
    COMMAND comparator_test
)

# Benchmarks
add_executable(long_chain_benchmark
    benchmarks/long_chain_benchmark.c
)

target_link_libraries(long_chain_benchmark
    PRIVATE
        redstonex
)

add_executable(high_fan_out_benchmark
    benchmarks/high_fan_out_benchmark.c
)

target_link_libraries(high_fan_out_benchmark
    PRIVATE
        redstonex
)

add_executable(high_churn_torch_oscillators_benchmark
    benchmarks/high_churn_torch_oscillators_benchmark.c
)

target_link_libraries(high_churn_torch_oscillators_benchmark
    PRIVATE
        redstonex
)

# Install
if(REDSTONEX_INSTALL)
    install(
        TARGETS redstonex
        EXPORT RedStoneXTargets

        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    install(
        DIRECTORY include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(
        EXPORT RedStoneXTargets
        FILE RedStoneXTargets.cmake
        NAMESPACE RedStoneX::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/RedStoneX
    )

    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/RedStoneXConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/RedStoneXConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/RedStoneXConfig.cmake"

        INSTALL_DESTINATION
            ${CMAKE_INSTALL_LIBDIR}/cmake/RedStoneX
    )

    install(
        FILES
            "${CMAKE_CURRENT_BINARY_DIR}/RedStoneXConfig.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/RedStoneXConfigVersion.cmake"

        DESTINATION
            ${CMAKE_INSTALL_LIBDIR}/cmake/RedStoneX
    )
endif()
