/**
 * platform_compat.h - Platform compatibility definitions
 * 
 * This file provides platform-specific definitions and macros to ensure
 * cross-platform compatibility between Windows, macOS, and Linux.
 */

#ifndef PLATFORM_COMPAT_H
#define PLATFORM_COMPAT_H

/* Platform detection */
#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
    #define PLATFORM_WINDOWS
#elif defined(__APPLE__) || defined(__MACH__)
    #define PLATFORM_MACOS
#elif defined(__linux__) || defined(__unix__)
    #define PLATFORM_LINUX
#else
    #error "Unsupported platform"
#endif

/* HIDAPI include path handling */
#ifdef PLATFORM_WINDOWS
    #include "hidapi-win/include/hidapi.h"
#else
    #include <hidapi/hidapi.h>
#endif

/* Platform-specific function mappings */
#ifdef PLATFORM_WINDOWS
    #define popen _popen
    #define pclose _pclose
    /* Add other Windows-specific mappings as needed */
#endif

/* Platform-specific command definitions */
#ifdef PLATFORM_WINDOWS
    #define PATH_SEPARATOR "\\"
    #define FIND_COMMAND "dir /b"
#else
    #define PATH_SEPARATOR "/"
    #define FIND_COMMAND "find"
#endif

#endif /* PLATFORM_COMPAT_H */