cmake_minimum_required(VERSION 3.18)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")

project(sixaxispairer VERSION 1.1.0 LANGUAGES C)

set(HIDAPI_FIND_DEBUG ON CACHE BOOL "Enable debug output for FindHIDAPI module")

# Platform-specific settings
if(WIN32)
    # For Windows, use the local hidapi-win directory
    set(HIDAPI_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/hidapi-win" CACHE PATH "Path to HIDAPI library")
    add_compile_definitions(PLATFORM_WINDOWS)
elseif(APPLE)
    add_compile_definitions(PLATFORM_MACOS)
elseif(UNIX)
    add_compile_definitions(PLATFORM_LINUX)
endif()

find_package(hidapi REQUIRED)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(SOURCES 
    main.c
    mac_utils.c
    controller_info.c
    controller_connection.c
    ui.c
    platform_compat.h
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${hidapi_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} hidapi::hidapi ${PLATFORM_LIBS})
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

# Copy DLL to output directory on Windows
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${HIDAPI_ROOT_DIR}/x64/hidapi.dll"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMENT "Copying hidapi.dll to output directory"
    )
endif()