# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.8)
project(vicon_transformer)

# Specify C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)

# libraries need to be position independent for building Python modules
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic -Wfatal-errors -Werror=return-type)
endif()

include(GNUInstallDirs)

# find dependencies
find_package(ament_cmake REQUIRED)
# add mpi_cmake_modules first as it provides FindX-files for some libraries
find_package(mpi_cmake_modules REQUIRED)

find_package(cli_utils REQUIRED)
find_package(o80 REQUIRED)
find_package(serialization_utils REQUIRED)
find_package(spatial_transformation REQUIRED)

find_package(cereal REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(fmt REQUIRED)
find_package(pybind11 REQUIRED)
find_package(spdlog REQUIRED)
find_package(vicon-datastream-sdk REQUIRED)

ament_export_dependencies(fmt spdlog vicon-datastream-sdk)

ament_python_install_package(${PROJECT_NAME} PACKAGE_DIR ${PROJECT_NAME})


add_library(vicon_receiver
    src/vicon_receiver.cpp
    src/types.cpp
)
target_include_directories(vicon_receiver PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(vicon_receiver
    cereal::cereal
    Eigen3::Eigen
    fmt::fmt
    spdlog::spdlog
    vicon-datastream-sdk::ViconDataStreamSDK_CPP
    serialization_utils::serialization_utils
    spatial_transformation::transformation
)


add_library(vicon_transformer
    src/vicon_transformer.cpp
)
target_include_directories(vicon_transformer PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(vicon_transformer
    spdlog::spdlog
    vicon_receiver
)


add_executable(vicon_print_data src/vicon_print_data.cpp)
target_link_libraries(vicon_print_data
    vicon_receiver
    cli_utils::program_options
)

add_executable(vicon_record src/record.cpp)
target_link_libraries(vicon_record
    vicon_receiver
    cli_utils::program_options
)


## Python Bindings
add_pybind11_module(${PROJECT_NAME}_bindings srcpy/bindings.cpp
    LINK_LIBRARIES 
        vicon_receiver
        vicon_transformer
        serialization_utils::serialization_utils
        o80::o80
)


install(DIRECTORY include/ DESTINATION include)

install(
    TARGETS
        vicon_receiver
        vicon_transformer

        vicon_print_data
        vicon_record
    EXPORT export_${PROJECT_NAME}
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install_scripts(
    scripts/vicon_print_data_py.py

    DESTINATION bin
)


if(BUILD_TESTING)
    find_package(ament_cmake_pytest REQUIRED)
    find_package(ament_cmake_gmock REQUIRED)

    # Python tests
    # Note: The default working directory for tests run by colon is the package root.
    # Python tests that use the C++ bindings need to be run in a different directory,
    # otherwise the local Python source will shadow the installed package and thus the
    # pybind11 modules will be missing.
    ament_add_pytest_test(test_vicon_transformer_py tests/test_vicon_transformer.py
        WORKING_DIRECTORY /tmp)

    ament_add_gmock(test_vicon_receiver_cpp
        tests/test_vicon_receiver.cpp
    )
    target_include_directories(test_vicon_receiver_cpp PRIVATE include)
    target_link_libraries(test_vicon_receiver_cpp vicon_receiver)
    target_compile_definitions(test_vicon_receiver_cpp PRIVATE
        TEST_DATA_FILE_DIR=${TEST_DATA_FILE_DIR})

    ament_add_gmock(test_vicon_transformer_cpp
        tests/test_vicon_transformer.cpp
    )
    target_include_directories(test_vicon_transformer_cpp PRIVATE include)
    target_link_libraries(test_vicon_transformer_cpp
        vicon_transformer
        fmt::fmt
    )
    target_compile_definitions(test_vicon_transformer_cpp PRIVATE
        TEST_DATA_FILE_DIR=${TEST_DATA_FILE_DIR})

endif()


ament_export_interfaces(export_${PROJECT_NAME} HAS_LIBRARY_TARGET)
ament_package()
