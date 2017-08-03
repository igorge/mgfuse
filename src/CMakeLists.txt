cmake_minimum_required(VERSION 3.7 FATAL_ERROR)
project(mgfuse)
set(proj_exe_name mgfuse)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")
#include(cotire)

#set(CMAKE_VERBOSE_MAKEFILE ON)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-deprecated -Wno-unused-parameter")

find_package(Boost COMPONENTS filesystem system thread serialization date_time locale)
############################################################################

add_definitions(-D_FILE_OFFSET_BITS=64)
add_definitions(-DFUSE_USE_VERSION=26)

add_definitions(-DBOOST_THREAD_PROVIDES_FUTURE_CONTINUATION)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    message("*** Debug build. ***")
    add_definitions(-DBOOST_MULTI_INDEX_ENABLE_SAFE_MODE)
    add_definitions(-DBOOST_MULTI_INDEX_ENABLE_INVARIANT_CHECKING)

    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvtable-verify=std")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=float-cast-overflow")

#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-sanitize-recover")
endif ()

############################################################################
add_subdirectory(../../libgie/ ${PROJECT_BINARY_DIR}/extlib/gie )

set(gie_include_dir ${gie_include_dir} ${libgie_SOURCE_DIR}/include)
set(gie_link_libs ${gie_link_libs} libgie)

############################################################################

include_directories(
        ${CMAKE_CURRENT_BINARY_DIR} ${Boost_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR} ${gie_include_dir}
)

############################################################################

set(HEADER_FILES
        mega_node_cache.hpp
        gie_fuse.hpp
        gie_fuse_fs_local.hpp
        mega_fuse.hpp
        path_locker.hpp)


set(SOURCE_FILES
        main.cpp)


############################################################################

add_executable(mgfuse ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(
        ${proj_exe_name}
        ${Boost_LIBRARIES}
        ${gie_link_libs}
        fuse
        #crypto++
        mega
)

#cotire(mgfuse)

