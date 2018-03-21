cmake_minimum_required (VERSION 2.6)
project(PerformTest)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_COMPILER /usr/bin/c++)

set (CPP_SOURCE config.cpp)

find_library (LIBCONFIG_LIBRARY NAMES libconfig++.so PATHS /usr/lib/ /usr/lib64/)
find_path (LIBCONFIG_INCLUDE libconfig.h++ /usr/include)
if (LIBCONFIG_LIBRARY)
    message(STATUS "Libconfig library found")
else()
    message(FATAL_ERROR "Libconfig not found! You should define your own path!")
endif()

include_directories(${LIBCONFIG_INCLUDE})

find_library (GLOG_LIBRARY NAMES libglog.so PATHS /usr/local/lib/)
find_path (GLOG_INCLUDE glog /usr/local/include)
if (GLOG_LIBRARY)
    message(STATUS "GLOG library found")
else()
    message(FATAL_ERROR "GLOG not found! You should define your own path!")
endif()
find_library(FRONTPANEL_LIBRAY libokFrontPanel.so /usr/local/lib)
include_directories("${PROJECT_BINARY_DIR}")

if (FRONTPANEL_LIBRAY)
    message(STATUS "FrontPanel library found")
else()
    message(FATAL_ERROR "FrontPanel not found! You should define your own path!")
endif()

set (LIBS ${LIBS} ${LIBCONFIG_LIBRARY})
set (LIBS ${LIBS} ${GLOG_LIBRARY})
set (LIBS ${LIBS} ${FRONTPANEL_LIBRAY})

add_executable(opalkelly_test_performance ${CPP_SOURCE})
target_link_libraries (opalkelly_test_performance ${LIBS})
