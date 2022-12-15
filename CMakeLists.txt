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

#find_package(Eigen3 REQUIRED)
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
#find_package(pybind11 REQUIRED)

ament_export_dependencies(fmt spdlog)

ament_python_install_package(${PROJECT_NAME} PACKAGE_DIR ${PROJECT_NAME})


#add_library(solo12_config
#    src/solo12_config.cpp
#)
#target_include_directories(solo12_config PUBLIC
#    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#    $<INSTALL_INTERFACE:include>
#)
#target_link_libraries(solo12_config
#    Eigen3::Eigen
#    yaml_utils::yaml_utils
#    fmt::fmt
#)


## Python Bindings
#add_pybind11_module(solo12 srcpy/solo12.cpp
#    LINK_LIBRARIES 
#        solo12_driver
#        solo12_pybullet_driver
#)


install(DIRECTORY include/ DESTINATION include)

#install(
#    TARGETS
#        demo_solo12_hold
#    EXPORT export_${PROJECT_NAME}
#    LIBRARY DESTINATION lib
#    ARCHIVE DESTINATION lib
#    RUNTIME DESTINATION lib/${PROJECT_NAME}
#    INCLUDES DESTINATION include
#)

#install_scripts(
#    demos/demo_solo12_sine.py
#    demos/demo_solo12_simulation.py
#    scripts/solo12_show_data.py
#
#    DESTINATION lib/${PROJECT_NAME}
#)


if(BUILD_TESTING)
    find_package(ament_cmake_pytest REQUIRED)

    # Python tests
    ament_add_pytest_test(test_transform tests/test_transform.py)
    ament_add_pytest_test(test_vicon_json tests/test_vicon_json.py)
endif()


#ament_export_interfaces(export_${PROJECT_NAME} HAS_LIBRARY_TARGET)
ament_package()