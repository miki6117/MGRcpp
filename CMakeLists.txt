cmake_minimum_required (VERSION 2.6)
project(PerformTest)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set (CMAKE_CXX_STANDARD 11)
# set (CMAKE_CXX_COMPILER /usr/bin/c++)

set (CPP_SOURCE main.cpp config.cpp results.cpp transfer.cpp timer.cpp performance.h)

### Libconfig libray
if(WIN32)
    find_library (LIBCONFIG_LIBRARY NAMES libconfig++.lib PATHS C:/Users/Mik/Downloads/libconfig-master/Release)
    find_path (LIBCONFIG_INCLUDE libconfig.h++ C:/Users/Mik/Downloads/libconfig-master/lib)
elseif(UNIX)
    find_library (LIBCONFIG_LIBRARY NAMES libconfig++.so PATHS /usr/lib/ /usr/lib64/)
    find_path (LIBCONFIG_INCLUDE libconfig.h++ /usr/include)
endif(WIN32)

if (LIBCONFIG_LIBRARY)
    message(STATUS "Libconfig library found")
else()
    message(FATAL_ERROR "Libconfig not found! You should define your own path!")
endif()

include_directories(${LIBCONFIG_INCLUDE})

### Glog library
if(WIN32)
    ## NOTE: there should be glog source in project directory!
    add_subdirectory(glog)
    set(GLOG_LIBRARY glog::glog)
elseif(UNIX)
    find_library (GLOG_LIBRARY NAMES libglog.so PATHS /usr/local/lib/)
    find_path (GLOG_INCLUDE glog /usr/local/include)
endif(WIN32)

if (GLOG_LIBRARY)
    message(STATUS "GLOG library found")
else()
    message(FATAL_ERROR "GLOG not found! You should define your own path!")
endif()

include_directories(${GLOG_INCLUDE})

### Frontpanel library
if(WIN32)
    find_library(FRONTPANEL_LIBRAY okFrontPanel.lib C:/Program\ Files/Opal\ Kelly/FrontPanelUSB/API/lib/Win32)
    find_path (FRONTPANEL_INCLUDE okFrontPanelDLL.h C:/Program\ Files/Opal\ Kelly/FrontPanelUSB/API/include)
elseif(UNIX)
    find_library(FRONTPANEL_LIBRAY libokFrontPanel.so /usr/local/lib)
endif(WIN32)

if (FRONTPANEL_LIBRAY)
    message(STATUS "FrontPanel library found")
else()
    message(FATAL_ERROR "FrontPanel not found! You should define your own path!")
endif()

include_directories(${FRONTPANEL_INCLUDE})

include_directories("${PROJECT_BINARY_DIR}")
set (LIBS ${LIBS} ${LIBCONFIG_LIBRARY})
set (LIBS ${LIBS} ${GLOG_LIBRARY})
set (LIBS ${LIBS} ${FRONTPANEL_LIBRAY})

add_executable(opalkelly_test_performance ${CPP_SOURCE})
target_link_libraries (opalkelly_test_performance ${LIBS})
