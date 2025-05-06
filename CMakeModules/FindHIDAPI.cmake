#.rst:
# FindHIDAPI
# ----------
#
# Try to find HIDAPI library, from http://www.signal11.us/oss/hidapi/
#
# Cache Variables: (probably not for direct use in your scripts)
#  HIDAPI_INCLUDE_DIR
#  HIDAPI_LIBRARY
#
# Non-cache variables you might use in your CMakeLists.txt:
#  HIDAPI_FOUND
#  HIDAPI_INCLUDE_DIRS
#  HIDAPI_LIBRARIES
#
# COMPONENTS
# ^^^^^^^^^^
#
# This module respects several COMPONENTS specifying the backend you prefer:
# ``any`` (the default), ``libusb``, and ``hidraw``.
# The availablility of the latter two depends on your platform.
#
#
# IMPORTED Targets
# ^^^^^^^^^^^^^^^^
#
# This module defines :prop_tgt:`IMPORTED` target ``HIDAPI::hidapi`` (in all cases or
# if no components specified), ``HIDAPI::hidapi-libusb`` (if you requested the libusb component),
# and ``HIDAPI::hidapi-hidraw`` (if you requested the hidraw component),
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# ``HIDAPI_FOUND``
#   True if HIDAPI or the requested components (if any) were found.
#
# We recommend using the imported targets instead of the following.
#
# ``HIDAPI_INCLUDE_DIRS``
# ``HIDAPI_LIBRARIES``
#
# Debug Output
# ^^^^^^^^^^^
#
# Enable verbose debug output by setting HIDAPI_FIND_DEBUG to ON in your CMake configuration:
#   set(HIDAPI_FIND_DEBUG ON CACHE BOOL "Enable debug output for FindHIDAPI module")
#
# Original Author:
# 2009-2021 Rylie Pavlik <rylie.pavlik@collabora.com> <rylie@ryliepavlik.com>
# https://ryliepavlik.com/
#
# Copyright 2009-2010, Iowa State University
# Copyright 2019-2021, Collabora, Ltd.
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#
# SPDX-License-Identifier: BSL-1.0

# Debug option
option(HIDAPI_FIND_DEBUG "Enable debug output for FindHIDAPI module" OFF)

# Debug message macro
macro(hidapi_debug_message)
    if(HIDAPI_FIND_DEBUG)
        message(STATUS "FindHIDAPI: ${ARGN}")
    endif()
endmacro()

hidapi_debug_message("Starting search for HIDAPI library")

set(HIDAPI_ROOT_DIR
    "${HIDAPI_ROOT_DIR}"
    CACHE PATH "Root to search for HIDAPI")

if(HIDAPI_ROOT_DIR)
    hidapi_debug_message("HIDAPI_ROOT_DIR specified as ${HIDAPI_ROOT_DIR}")
endif()

# Clean up components
if(HIDAPI_FIND_COMPONENTS)
    hidapi_debug_message("Components requested: ${HIDAPI_FIND_COMPONENTS}")
    
    if(WIN32 OR APPLE)
        # This makes no sense on Windows or Mac, which have native APIs
        list(REMOVE HIDAPI_FIND_COMPONENTS libusb)
        hidapi_debug_message("Removed 'libusb' component on Windows/Mac (using native APIs instead)")
    endif()

    if(NOT ${CMAKE_SYSTEM} MATCHES "Linux")
        # hidraw is only on linux
        list(REMOVE HIDAPI_FIND_COMPONENTS hidraw)
        hidapi_debug_message("Removed 'hidraw' component on non-Linux platform")
    endif()
endif()

if(NOT HIDAPI_FIND_COMPONENTS)
    # Default to any
    set(HIDAPI_FIND_COMPONENTS any)
    hidapi_debug_message("No components specified, defaulting to 'any'")
endif()

hidapi_debug_message("Components after platform filtering: ${HIDAPI_FIND_COMPONENTS}")

# Ask pkg-config for hints
if(NOT ANDROID)
    find_package(PkgConfig QUIET)
    if(PKG_CONFIG_FOUND)
        hidapi_debug_message("PkgConfig found, checking for HIDAPI packages")
        set(_old_prefix_path "${CMAKE_PREFIX_PATH}")
        # So pkg-config uses HIDAPI_ROOT_DIR too.
        if(HIDAPI_ROOT_DIR)
            list(APPEND CMAKE_PREFIX_PATH ${HIDAPI_ROOT_DIR})
            hidapi_debug_message("Added HIDAPI_ROOT_DIR to CMAKE_PREFIX_PATH for pkg-config")
        endif()
        
        pkg_check_modules(PC_HIDAPI_LIBUSB QUIET hidapi-libusb)
        if(PC_HIDAPI_LIBUSB_FOUND)
            hidapi_debug_message("pkg-config found hidapi-libusb:")
            hidapi_debug_message("  version: ${PC_HIDAPI_LIBUSB_VERSION}")
            hidapi_debug_message("  libraries: ${PC_HIDAPI_LIBUSB_LIBRARIES}")
            hidapi_debug_message("  library dirs: ${PC_HIDAPI_LIBUSB_LIBRARY_DIRS}")
            hidapi_debug_message("  include dirs: ${PC_HIDAPI_LIBUSB_INCLUDE_DIRS}")
        else()
            hidapi_debug_message("pkg-config did not find hidapi-libusb")
        endif()
        
        pkg_check_modules(PC_HIDAPI_HIDRAW QUIET hidapi-hidraw)
        if(PC_HIDAPI_HIDRAW_FOUND)
            hidapi_debug_message("pkg-config found hidapi-hidraw:")
            hidapi_debug_message("  version: ${PC_HIDAPI_HIDRAW_VERSION}")
            hidapi_debug_message("  libraries: ${PC_HIDAPI_HIDRAW_LIBRARIES}")
            hidapi_debug_message("  library dirs: ${PC_HIDAPI_HIDRAW_LIBRARY_DIRS}")
            hidapi_debug_message("  include dirs: ${PC_HIDAPI_HIDRAW_INCLUDE_DIRS}")
        else()
            hidapi_debug_message("pkg-config did not find hidapi-hidraw")
        endif()
        
        # Restore
        set(CMAKE_PREFIX_PATH "${_old_prefix_path}")
    else()
        hidapi_debug_message("PkgConfig not found, skipping pkg-config lookup")
    endif()
else()
    hidapi_debug_message("Android platform detected, skipping pkg-config lookup")
endif()

# Actually search
hidapi_debug_message("Searching for undecorated HIDAPI library")
find_library(
    HIDAPI_UNDECORATED_LIBRARY
    NAMES hidapi
    PATHS "${HIDAPI_ROOT_DIR}"
    PATH_SUFFIXES lib)

if(HIDAPI_UNDECORATED_LIBRARY)
    hidapi_debug_message("Found undecorated HIDAPI library: ${HIDAPI_UNDECORATED_LIBRARY}")
else()
    hidapi_debug_message("Could not find undecorated HIDAPI library")
endif()

hidapi_debug_message("Searching for HIDAPI libusb library")
find_library(
    HIDAPI_LIBUSB_LIBRARY
    NAMES hidapi hidapi-libusb
    PATHS "${HIDAPI_ROOT_DIR}"
    PATH_SUFFIXES lib
    HINTS ${PC_HIDAPI_LIBUSB_LIBRARY_DIRS})

if(HIDAPI_LIBUSB_LIBRARY)
    hidapi_debug_message("Found HIDAPI libusb library: ${HIDAPI_LIBUSB_LIBRARY}")
else()
    hidapi_debug_message("Could not find HIDAPI libusb library")
endif()

if(CMAKE_SYSTEM MATCHES "Linux")
    hidapi_debug_message("Linux system detected, searching for HIDAPI hidraw library")
    find_library(
        HIDAPI_HIDRAW_LIBRARY
        NAMES hidapi-hidraw
        HINTS ${PC_HIDAPI_HIDRAW_LIBRARY_DIRS})
        
    if(HIDAPI_HIDRAW_LIBRARY)
        hidapi_debug_message("Found HIDAPI hidraw library: ${HIDAPI_HIDRAW_LIBRARY}")
    else()
        hidapi_debug_message("Could not find HIDAPI hidraw library")
    endif()
endif()

hidapi_debug_message("Searching for HIDAPI header file")
find_path(
    HIDAPI_INCLUDE_DIR
    NAMES hidapi.h
    PATHS "${HIDAPI_ROOT_DIR}"
    PATH_SUFFIXES hidapi include include/hidapi
    HINTS ${PC_HIDAPI_HIDRAW_INCLUDE_DIRS} ${PC_HIDAPI_LIBUSB_INCLUDE_DIRS})

if(HIDAPI_INCLUDE_DIR)
    hidapi_debug_message("Found HIDAPI header: ${HIDAPI_INCLUDE_DIR}/hidapi.h")
else()
    hidapi_debug_message("Could not find HIDAPI header file 'hidapi.h'")
    message(STATUS "HIDAPI header not found. Please install HIDAPI development package or specify HIDAPI_ROOT_DIR.")
endif()

find_package(Threads QUIET)
if(Threads_FOUND)
    hidapi_debug_message("Found Threads package which is required by HIDAPI")
else()
    hidapi_debug_message("Could not find Threads package which is required by HIDAPI")
    message(WARNING "Threads package not found. HIDAPI requires threading support.")
endif()

###
# Compute the "I don't care which backend" library
###
set(HIDAPI_LIBRARY)
hidapi_debug_message("Selecting preferred backend based on components")

# First, try to use a preferred backend if supplied
if("${HIDAPI_FIND_COMPONENTS}" MATCHES "libusb"
   AND HIDAPI_LIBUSB_LIBRARY
   AND NOT HIDAPI_LIBRARY)
    set(HIDAPI_LIBRARY ${HIDAPI_LIBUSB_LIBRARY})
    hidapi_debug_message("Selected libusb backend: ${HIDAPI_LIBRARY}")
endif()
if("${HIDAPI_FIND_COMPONENTS}" MATCHES "hidraw"
   AND HIDAPI_HIDRAW_LIBRARY
   AND NOT HIDAPI_LIBRARY)
    set(HIDAPI_LIBRARY ${HIDAPI_HIDRAW_LIBRARY})
    hidapi_debug_message("Selected hidraw backend: ${HIDAPI_LIBRARY}")
endif()

# Then, if we don't have a preferred one, settle for anything.
if(NOT HIDAPI_LIBRARY)
    hidapi_debug_message("No preferred backend or preferred backend not found, trying alternatives")
    if(HIDAPI_LIBUSB_LIBRARY)
        set(HIDAPI_LIBRARY ${HIDAPI_LIBUSB_LIBRARY})
        hidapi_debug_message("Selected libusb backend (fallback): ${HIDAPI_LIBRARY}")
    elseif(HIDAPI_HIDRAW_LIBRARY)
        set(HIDAPI_LIBRARY ${HIDAPI_HIDRAW_LIBRARY})
        hidapi_debug_message("Selected hidraw backend (fallback): ${HIDAPI_LIBRARY}")
    elseif(HIDAPI_UNDECORATED_LIBRARY)
        set(HIDAPI_LIBRARY ${HIDAPI_UNDECORATED_LIBRARY})
        hidapi_debug_message("Selected undecorated library (fallback): ${HIDAPI_LIBRARY}")
    else()
        hidapi_debug_message("Could not find any HIDAPI backend library")
        message(STATUS "Could not find any HIDAPI library. Please install HIDAPI or specify HIDAPI_ROOT_DIR.")
    endif()
endif()

###
# Determine if the various requested components are found.
###
set(_hidapi_component_required_vars)
hidapi_debug_message("Checking requested components")

foreach(_comp IN LISTS HIDAPI_FIND_COMPONENTS)
    if("${_comp}" STREQUAL "any")
        list(APPEND _hidapi_component_required_vars HIDAPI_INCLUDE_DIR
             HIDAPI_LIBRARY)
        if(HIDAPI_INCLUDE_DIR AND EXISTS "${HIDAPI_LIBRARY}")
            set(HIDAPI_any_FOUND TRUE)
            hidapi_debug_message("Component 'any' found (include dir and library exist)")
            mark_as_advanced(HIDAPI_INCLUDE_DIR)
        else()
            set(HIDAPI_any_FOUND FALSE)
            hidapi_debug_message("Component 'any' NOT found")
            if(NOT HIDAPI_INCLUDE_DIR)
                message(STATUS "HIDAPI 'any' component not found: missing header file")
            endif()
            if(NOT EXISTS "${HIDAPI_LIBRARY}")
                message(STATUS "HIDAPI 'any' component not found: missing library file")
            endif()
        endif()

    elseif("${_comp}" STREQUAL "libusb")
        list(APPEND _hidapi_component_required_vars HIDAPI_INCLUDE_DIR
             HIDAPI_LIBUSB_LIBRARY)
        if(HIDAPI_INCLUDE_DIR AND EXISTS "${HIDAPI_LIBUSB_LIBRARY}")
            set(HIDAPI_libusb_FOUND TRUE)
            hidapi_debug_message("Component 'libusb' found (include dir and library exist)")
            mark_as_advanced(HIDAPI_INCLUDE_DIR HIDAPI_LIBUSB_LIBRARY)
        else()
            set(HIDAPI_libusb_FOUND FALSE)
            hidapi_debug_message("Component 'libusb' NOT found")
            if(NOT HIDAPI_INCLUDE_DIR)
                message(STATUS "HIDAPI 'libusb' component not found: missing header file")
            endif()
            if(NOT EXISTS "${HIDAPI_LIBUSB_LIBRARY}")
                message(STATUS "HIDAPI 'libusb' component not found: missing library file")
                if(CMAKE_SYSTEM MATCHES "Linux")
                    message(STATUS "  On Linux, you may need to install libhidapi-libusb0 or equivalent package")
                endif()
            endif()
        endif()

    elseif("${_comp}" STREQUAL "hidraw")
        list(APPEND _hidapi_component_required_vars HIDAPI_INCLUDE_DIR
             HIDAPI_HIDRAW_LIBRARY)
        if(HIDAPI_INCLUDE_DIR AND EXISTS "${HIDAPI_HIDRAW_LIBRARY}")
            set(HIDAPI_hidraw_FOUND TRUE)
            hidapi_debug_message("Component 'hidraw' found (include dir and library exist)")
            mark_as_advanced(HIDAPI_INCLUDE_DIR HIDAPI_HIDRAW_LIBRARY)
        else()
            set(HIDAPI_hidraw_FOUND FALSE)
            hidapi_debug_message("Component 'hidraw' NOT found")
            if(NOT HIDAPI_INCLUDE_DIR)
                message(STATUS "HIDAPI 'hidraw' component not found: missing header file")
            endif()
            if(NOT EXISTS "${HIDAPI_HIDRAW_LIBRARY}")
                message(STATUS "HIDAPI 'hidraw' component not found: missing library file")
                if(CMAKE_SYSTEM MATCHES "Linux")
                    message(STATUS "  On Linux, you may need to install libhidapi-hidraw0 or equivalent package")
                endif()
            endif()
        endif()

    else()
        message(WARNING "${_comp} is not a recognized HIDAPI component (valid: any, libusb, hidraw)")
        set(HIDAPI_${_comp}_FOUND FALSE)
    endif()
endforeach()
unset(_comp)

###
# FPHSA call
###
hidapi_debug_message("Required vars for find_package_handle_standard_args: ${_hidapi_component_required_vars}")
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    HIDAPI
    REQUIRED_VARS ${_hidapi_component_required_vars} THREADS_FOUND
    HANDLE_COMPONENTS)

if(HIDAPI_FOUND)
    hidapi_debug_message("HIDAPI found")
    set(HIDAPI_LIBRARIES "${HIDAPI_LIBRARY}")
    set(HIDAPI_INCLUDE_DIRS "${HIDAPI_INCLUDE_DIR}")
    
    # Check for version if header exists
    if(HIDAPI_INCLUDE_DIR AND EXISTS "${HIDAPI_INCLUDE_DIR}/hidapi.h")
        file(STRINGS "${HIDAPI_INCLUDE_DIR}/hidapi.h" _hidapi_version_str
             REGEX "^#define[ \t]+HIDAPI_.*VERSION")
        
        string(REGEX REPLACE ".*MAJOR_VERSION[ \t]+([0-9]+).*" "\\1" 
               HIDAPI_MAJOR_VERSION "${_hidapi_version_str}")
        string(REGEX REPLACE ".*MINOR_VERSION[ \t]+([0-9]+).*" "\\1" 
               HIDAPI_MINOR_VERSION "${_hidapi_version_str}")
        string(REGEX REPLACE ".*MICRO_VERSION[ \t]+([0-9]+).*" "\\1" 
               HIDAPI_MICRO_VERSION "${_hidapi_version_str}")
        
        set(HIDAPI_VERSION "${HIDAPI_MAJOR_VERSION}.${HIDAPI_MINOR_VERSION}.${HIDAPI_MICRO_VERSION}")
        hidapi_debug_message("HIDAPI version: ${HIDAPI_VERSION}")
    endif()
    
    if(NOT TARGET HIDAPI::hidapi)
        hidapi_debug_message("Creating HIDAPI::hidapi imported target")
        add_library(HIDAPI::hidapi UNKNOWN IMPORTED)
        set_target_properties(
            HIDAPI::hidapi
            PROPERTIES IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                       IMPORTED_LOCATION "${HIDAPI_LIBRARY}"
                       INTERFACE_INCLUDE_DIRECTORIES "${HIDAPI_INCLUDE_DIR}"
                       IMPORTED_LINK_INTERFACE_LIBRARIES Threads::Threads)
    endif()
else()
    hidapi_debug_message("HIDAPI NOT found")
    if(HIDAPI_FIND_REQUIRED)
        message(FATAL_ERROR "Could not find HIDAPI library. Please install HIDAPI or specify HIDAPI_ROOT_DIR.")
    endif()
endif()

if(HIDAPI_libusb_FOUND AND NOT TARGET HIDAPI::hidapi-libusb)
    hidapi_debug_message("Creating HIDAPI::hidapi-libusb imported target")
    add_library(HIDAPI::hidapi-libusb UNKNOWN IMPORTED)
    set_target_properties(
        HIDAPI::hidapi-libusb
        PROPERTIES IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                   IMPORTED_LOCATION "${HIDAPI_LIBUSB_LIBRARY}"
                   INTERFACE_INCLUDE_DIRECTORIES "${HIDAPI_INCLUDE_DIR}"
                   IMPORTED_LINK_INTERFACE_LIBRARIES Threads::Threads)
endif()

if(HIDAPI_hidraw_FOUND AND NOT TARGET HIDAPI::hidapi-hidraw)
    hidapi_debug_message("Creating HIDAPI::hidapi-hidraw imported target")
    add_library(HIDAPI::hidapi-hidraw UNKNOWN IMPORTED)
    set_target_properties(
        HIDAPI::hidapi-hidraw
        PROPERTIES IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                   IMPORTED_LOCATION "${HIDAPI_HIDRAW_LIBRARY}"
                   INTERFACE_INCLUDE_DIRECTORIES "${HIDAPI_INCLUDE_DIR}"
                   IMPORTED_LINK_INTERFACE_LIBRARIES Threads::Threads)
endif()

# Final summary
if(HIDAPI_FIND_DEBUG)
    message(STATUS "FindHIDAPI results:")
    message(STATUS "  HIDAPI_FOUND: ${HIDAPI_FOUND}")
    if(HIDAPI_FOUND)
        message(STATUS "  HIDAPI_VERSION: ${HIDAPI_VERSION}")
        message(STATUS "  HIDAPI_INCLUDE_DIRS: ${HIDAPI_INCLUDE_DIRS}")
        message(STATUS "  HIDAPI_LIBRARIES: ${HIDAPI_LIBRARIES}")
        message(STATUS "  Components:")
        foreach(_comp IN LISTS HIDAPI_FIND_COMPONENTS)
            message(STATUS "    ${_comp}: ${HIDAPI_${_comp}_FOUND}")
        endforeach()
    endif()
endif()