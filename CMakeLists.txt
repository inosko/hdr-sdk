cmake_minimum_required(VERSION 3.17)
project(hdr_converter)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)


find_package(OpenCV REQUIRED)

if(UNIX)
    find_package(Threads REQUIRED)
endif(UNIX)



add_subdirectory(extern)

add_subdirectory(hdr)

add_executable(hdr_converter main.cpp)


target_include_directories(hdr_converter PRIVATE ${HDR_LIB_INCLUDE_DIR})
target_link_libraries(hdr_converter
        PRIVATE
        cxxopts
        elpp
        ${HDR_LIB}
        ${OpenCV_LIBS})




if(CMAKE_THREAD_LIBS_INIT)
    target_link_libraries(hdr_converter PRIVATE ${CMAKE_THREAD_LIBS_INIT})
endif()