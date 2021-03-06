cmake_minimum_required(VERSION 2.8.9)

# Default install location. Must be set here, before setting the project.
if (NOT DEFINED CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "" FORCE)
endif()

project(unity8 C CXX)

if(${PROJECT_BINARY_DIR} STREQUAL ${PROJECT_SOURCE_DIR})
   message(FATAL_ERROR "In-tree build attempt detected, aborting. Set your build dir outside your source dir, delete CMakeCache.txt from source root and try again.")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

# Instruct CMake to run moc and rcc automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# enable QML debugging
string(TOLOWER "${CMAKE_BUILD_TYPE}" cmake_build_type_lower) # Build types should always be lowercase but sometimes they are not.
if(cmake_build_type_lower MATCHES "debug")
    add_definitions(-DQT_QML_DEBUG)
endif()

include(EnableCoverageReport)
#####################################################################
# Enable code coverage calculation with gcov/gcovr/lcov
# Usage:
#  * Switch build type to coverage (use ccmake or cmake-gui)
#  * Invoke make, make test, make coverage (or ninja if you use that backend)
#  * Find html report in subdir coveragereport
#  * Find xml report feasible for jenkins in coverage.xml
#####################################################################
if(cmake_build_type_lower MATCHES coverage)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage" )
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage" )
  set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} --coverage" )
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} --coverage" )
  ENABLE_COVERAGE_REPORT(TARGETS ${SHELL_APP} FILTER /usr/include ${CMAKE_SOURCE_DIR}/tests/* ${CMAKE_BINARY_DIR}/*)
endif()

find_package (ECM 1.7.0 QUIET NO_MODULE)
if (ECM_FOUND)
    # Provides us with -DECM_ENABLE_SANITIZERS='X'
    # Where X can be address, thread, memory, leak, undefined
    include("${ECM_MODULE_DIR}/ECMEnableSanitizers.cmake")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "5.0.0")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wsuggest-override" )
  endif()
endif()

# Make sure we have all the needed symbols
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-z,defs")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,-z,defs")

# Static C++ checks
add_custom_target(cppcheck COMMAND cppcheck --enable=all -q --error-exitcode=2
                                       ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/tests)

include(FindPkgConfig)
find_package(Qt5Core 5.6 REQUIRED)
find_package(Qt5Qml 5.6 REQUIRED)
find_package(Qt5Quick 5.6 REQUIRED)
find_package(Qt5Gui 5.6 REQUIRED)
find_package(Qt5DBus 5.6 REQUIRED)
find_package(Qt5Concurrent 5.6 REQUIRED)
find_package(Qt5Sql 5.6 REQUIRED)
find_package(Qt5Test 5.6 REQUIRED)

pkg_check_modules(APPLICATION_API REQUIRED unity-shell-application=27)
pkg_check_modules(GEONAMES REQUIRED geonames>=0.2)
pkg_check_modules(GIO REQUIRED gio-2.0>=2.32)
pkg_check_modules(GLIB REQUIRED glib-2.0>=2.32)
pkg_check_modules(LAUNCHER_API REQUIRED unity-shell-launcher=13)
pkg_check_modules(QMENUMODEL REQUIRED qmenumodel)
pkg_check_modules(GD3 REQUIRED gnome-desktop-3.0)
pkg_check_modules(UAL REQUIRED ubuntu-app-launch-2)
pkg_check_modules(UBUNTUGESTURES REQUIRED UbuntuGestures)
pkg_check_modules(QTDBUSTEST REQUIRED
    libqtdbustest-1
    libqtdbusmock-1
    )

### Check UbuntuGestures private headers. No pkg-config (.pc) file is provided for them
find_path(UBUNTUGESTUREPRIV
    NAMES UbuntuGestures/private/damper_p.h UbuntuGestures/private/ucswipearea_p.h UbuntuGestures/private/ucswipearea_p_p.h
    PATHS ${UBUNTUGESTURES_INCLUDEDIR}/UbuntuGestures/${UBUNTUGESTURES_VERSION}
    NO_DEFAULT_PATH)
if (${UBUNTUGESTUREPRIV} STREQUAL UBUNTUGESTUREPRIV-NOTFOUND)
   message(FATAL_ERROR "UbuntuGestures private headers not found.")
endif()
# end of UbuntuGesture private headers check

# Standard install paths
include(GNUInstallDirs)

set(SHELL_APP_DIR ${CMAKE_INSTALL_DATADIR}/unity8)
set(SHELL_PRIVATE_LIBDIR ${CMAKE_INSTALL_LIBDIR}/unity8)

execute_process(COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=plugin_private_module_dir SystemSettings OUTPUT_VARIABLE USS_PRIVATE_PLUGINDIR OUTPUT_STRIP_TRAILING_WHITESPACE)
if(USS_PRIVATE_PLUGINDIR STREQUAL "")
    message(FATAL_ERROR "Could not determine USS private plugin installation dir.")
endif()

execute_process(COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=plugindir_suffix unity-shell-api OUTPUT_VARIABLE SHELL_INSTALL_QML OUTPUT_STRIP_TRAILING_WHITESPACE)
if(SHELL_INSTALL_QML STREQUAL "")
    message(FATAL_ERROR "Could not determine plugin installation dir.")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")

execute_process(COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=plugindir unity-shell-api OUTPUT_VARIABLE SHELL_PLUGINDIR OUTPUT_STRIP_TRAILING_WHITESPACE)
if(SHELL_PLUGINDIR STREQUAL "")
    message(FATAL_ERROR "Could not determine plugin import dir.")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-permissive -pedantic -Wall -Wextra")

if ("${CMAKE_BUILD_TYPE}" STREQUAL "release" OR "${CMAKE_BUILD_TYPE}" STREQUAL "relwithdebinfo")
    option(Werror "Treat warnings as errors" ON)
else()
    option(Werror "Treat warnings as errors" OFF)
endif()

if (Werror)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
endif()

# gtk and unity-core (actually sigc++) do not like Qt defining macros named
# "signals" and "slots"
add_definitions(-DQT_NO_KEYWORDS)

# Save a few container detach and conversions
add_definitions(-DQT_STRICT_ITERATORS)

# Use the fast string builder
add_definitions(-DQT_USE_QSTRINGBUILDER)

# Autopilot tests
include(autopilot)
declare_autopilot_test(shell unity8.shell ${CMAKE_SOURCE_DIR}/tests/autopilot/)

set(SHELL_APP unity8)
set(INDICATORS_CLIENT_APP indicators-client)
set(GREETER_APP unity8-greeter)
set(MOCK_INDICATOR_SERVICE_APP unity-mock-indicator-service)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/include
    ${APPLICATION_API_INCLUDE_DIRS}
)

# Quiet logger
set(QUIET_LOGGER OFF CACHE BOOL "Disable test output on stdout.")
if(NOT QUIET_LOGGER)
    set(STDOUT_LOGGER "-o" "-,txt")
endif()

execute_process(COMMAND dpkg-architecture -qDEB_HOST_ARCH OUTPUT_VARIABLE ARCH)
if (NOT ARCH MATCHES "arm*")
    set(ENABLE_TOUCH_EMULATION true)
    add_definitions(-DUNITY8_ENABLE_TOUCH_EMULATION)
    pkg_check_modules(MOUSETOUCHADAPTOR_LIBS REQUIRED x11 xcb xi)
else()
    set(ENABLE_TOUCH_EMULATION false)
    message(STATUS "Touch emulation support disabled.")
endif()

# add subdirectories to build
add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(tools)
add_subdirectory(qml)

# Tests
set(NO_TESTS OFF CACHE BOOL "Disable tests.")
if (NOT NO_TESTS)
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
else()
    message(STATUS "Tests disabled")
endif()

add_subdirectory(plugins)

#
# Data files
#
add_subdirectory(data)

#
# Translation
#
add_subdirectory(po)

#
# Documentation
#
add_subdirectory(doc)
