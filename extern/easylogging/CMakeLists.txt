cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)

option(${PROJECT_NAME}_DISABLE_DEBUG_LOGS "Disable debug logs" OFF)
option(${PROJECT_NAME}_DISABLE_INFO_LOGS "Disable info logs" OFF)
option(${PROJECT_NAME}_DISABLE_WARNING_LOGS "Disable warning logs" OFF)
option(${PROJECT_NAME}_DISABLE_ERROR_LOGS "Disable error logs" OFF)
option(${PROJECT_NAME}_DISABLE_FATAL_LOGS "Disable fatal logs" OFF)
option(${PROJECT_NAME}_DISABLE_TRACE_LOGS "Disable trace logs" OFF)
option(${PROJECT_NAME}_DISABLE_VERBOSE_LOGS "Disable verbose logs" OFF)




add_library(elpp STATIC src/easylogging.cpp)


target_include_directories(elpp
                           PUBLIC
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                           $<INSTALL_INTERFACE:include>)

# The following properties are useful when you want to have an 'install' target
set_target_properties(elpp
        PROPERTIES
        DEBUG_POSTFIX d # We had a postfix so that we can install debug and release libraries side by side (Windows way)
        )

message(STATUS "EASYLOGGING elpp (elpp): elpp")

if (${PROJECT_NAME}_DISABLE_DEBUG_LOGS)
    message("### Disabling debug logs")
    target_compile_definitions(elpp PUBLIC ELPP_DISABLE_DEBUG_LOGS)
endif()
if (${PROJECT_NAME}_DISABLE_INFO_LOGS)
    message("### Disabling debug logs")
    target_compile_definitions(elpp PUBLIC ELPP_DISABLE_INFO_LOGS)
endif()
if (${PROJECT_NAME}_DISABLE_WARNING_LOGS)
    message("### Disabling warning logs")
    target_compile_definitions(elpp PUBLIC ELPP_DISABLE_WARNING_LOGS)
endif()
if (${PROJECT_NAME}_DISABLE_ERROR_LOGS)
    message("### Disabling error logs")
    target_compile_definitions(elpp PUBLIC ELPP_DISABLE_ERROR_LOGS)
endif()
if (${PROJECT_NAME}_DISABLE_FATAL_LOGS)
    message("### Disabling fatal logs")
    target_compile_definitions(elpp PUBLIC ELPP_DISABLE_FATAL_LOGS)
endif()
if (${PROJECT_NAME}_DISABLE_TRACE_LOGS)
    message("### Disabling trace logs")
    target_compile_definitions(elpp PUBLIC ELPP_DISABLE_TRACE_LOGS)
endif()
if (${PROJECT_NAME}_DISABLE_VERBOSE_LOGS)
    message("### Disabling verbose logs")
    target_compile_definitions(elpp PUBLIC ELPP_DISABLE_VERBOSE_LOGS)
endif()

