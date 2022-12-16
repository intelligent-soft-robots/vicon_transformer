cmake_minimum_required(VERSION 3.8)
project(vicon_transformer)

# Specify C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)

# libraries need to be position independent for building Python modules
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
# add mpi_cmake_modules first as it provides FindX-files for some libraries
find_package(mpi_cmake_modules REQUIRED)

find_package(cereal REQUIRED)
find_package(fmt REQUIRED)
find_package(pybind11 REQUIRED)
find_package(spdlog REQUIRED)
find_package(vicon-datastream-sdk REQUIRED)

ament_export_dependencies(fmt spdlog vicon-datastream-sdk)

ament_python_install_package(${PROJECT_NAME} PACKAGE_DIR ${PROJECT_NAME})


add_library(vicon_receiver
    src/vicon_receiver.cpp
)
target_include_directories(vicon_receiver PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(vicon_receiver
    cereal::cereal
    fmt::fmt
    spdlog::spdlog
    vicon-datastream-sdk::ViconDataStreamSDK_CPP
)


add_executable(vicon_print_data src/vicon_print_data.cpp)
target_link_libraries(vicon_print_data
    vicon_receiver
)
target_include_directories(vicon_print_data PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)


## Python Bindings
add_pybind11_module(${PROJECT_NAME}_bindings srcpy/bindings.cpp
    LINK_LIBRARIES 
        vicon_receiver
)


install(DIRECTORY include/ DESTINATION include)

install(
    TARGETS
        vicon_receiver
        vicon_print_data
    EXPORT export_${PROJECT_NAME}
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION lib/${PROJECT_NAME}
    INCLUDES DESTINATION include
)

install_scripts(
    scripts/bridge_zmq.py
    scripts/vicon_print_data.py

    DESTINATION lib/${PROJECT_NAME}
)


if(BUILD_TESTING)
    find_package(ament_cmake_pytest REQUIRED)

    # Python tests
    ament_add_pytest_test(test_transform tests/test_transform.py)
    ament_add_pytest_test(test_vicon_json tests/test_vicon_json.py)
endif()


ament_export_interfaces(export_${PROJECT_NAME} HAS_LIBRARY_TARGET)
ament_package()
