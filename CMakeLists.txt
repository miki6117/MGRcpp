cmake_minimum_required (VERSION 2.6)
project(PerformTest)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_COMPILER /usr/bin/c++)

# set (CPP_SOURCE fifo_test_performance.cpp performance.hpp config.cpp data.cpp)
set (CPP_SOURCE main.cpp)

if(WIN32)
include_directories ("/usr/local/include" "/usr/lib64")
link_directories ("${PerformTest_SOURCE_DIR}/FrontPanel/lib/Win32" "${PerformTest_SOURCE_DIR}/libconfig")

set (LIBS ${LIBS} ${PerformTest_SOURCE_DIR}/libconfig/libconfig++.lib)
set (LIBS ${LIBS} ${PerformTest_SOURCE_DIR}/FrontPanel/lib/Win32/okFrontPanel.lib)
elseif(UNIX)
    # include_directories ("${PerformTest_SOURCE_DIR}/FrontPanel/include")
    # link_directories ("${PerformTest_SOURCE_DIR}/FrontPanel/lib/Mac")

    # YOU HAVE TO DEFINE YOUR OWN PATH FOR LIBCONFIG LIBRARY
    # find_path(LIBCONFIG_INCLUDE_DIR libconfig.h++ /usr/lib64)
    find_library(LIBCONFIG_LIBRAY libconfig++.so.9 /usr/lib64)
    find_library(FRONTPANEL_LIBRAY libokFrontPanel.so /usr/local/lib)

    # if (LIBCONFIG_INCLUDE_DIR AND LIBCONFIG_LIBRARY)
    if (FRONTPANEL_LIBRAY)
        message(STATUS "FrontPanel library found")
    else()
        message(FATAL_ERROR "FrontPanel not found! You should define your own path!")
    endif()

    # if (LIBCONFIG_LIBRARY)
    #     message(STATUS "Libconfig library found")
    # else()
    #     message(FATAL_ERROR "Libconfig not found! You should define your own path!")
    # endif()

    set (LIBS ${LIBS} ${LIBCONFIG_LIBRARY})
    set (LIBS ${LIBS} ${FRONTPANEL_LIBRAY})
endif(WIN32)

add_executable(opalkelly_test_performance ${CPP_SOURCE})
target_link_libraries (opalkelly_test_performance ${LIBS})
