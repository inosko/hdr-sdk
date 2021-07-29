cmake_minimum_required(VERSION 3.5)
project(hdr_lib)

set(CMAKE_CXX_STANDARD 17)



find_path(HDR_LIB_INCLUDE_DIR NAMES hdr/hdr_lib.hpp PATHS ../include)
set(HDR_LIB hdr_lib CACHE FILEPATH "")


add_library(hdr_lib STATIC
        src/hdrlib_tonemap_durand.cpp
        src/hdrlib_temporal_tonemap_durand.cpp
        src/hdrlib_tonemap_reinhard05.cpp)

target_include_directories(hdr_lib
        PUBLIC
        ${OpenCV_INCLUDE_DIRS}
        ${HDR_LIB_INCLUDE_DIR})

target_link_libraries(hdr_lib
        PRIVATE
        ${OpenCV_LIBS})


message("## HDR_LIB_INCLUDE_DIR: " ${HDR_LIB_INCLUDE_DIR})
message("## HDR_LIB: " ${HDR_LIB})

mark_as_advanced(${HDR_LIB_INCLUDE_DIR} ${HDR_LIB})

