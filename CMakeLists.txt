cmake_minimum_required(VERSION 3.18)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")

project(sixaxispairer VERSION 1.1.0 LANGUAGES C)

find_package(hidapi REQUIRED)
set(HIDAPI_FIND_DEBUG ON CACHE BOOL "Enable debug output for FindHIDAPI module")

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(SOURCES main.c)

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${hidapi_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} hidapi::hidapi ${PLATFORM_LIBS})
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

