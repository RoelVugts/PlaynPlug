cmake_minimum_required(VERSION 3.22)
# Change name to PlugNPlay
set(TARGET_NAME PlaynPlug)
set(CMAKE_CXX_STANDARD 17)

project(${TARGET_NAME} LANGUAGES C CXX)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Backend/Libraries)

add_subdirectory(Backend)

function(add_plugin_library index name)
    set(id_name "${index}_${name}")
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${id_name}")
        set(directory ${CMAKE_CURRENT_SOURCE_DIR}/${id_name})
    else()
        message(WARNING "Could not locate: ${CMAKE_CURRENT_SOURCE_DIR}/${id_name}")
        message(WARNING "Skipping course: ${id_name}")
        return()
    endif()

    file(GLOB_RECURSE sources ${directory}/*.cpp)
    add_library(${id_name} SHARED ${sources})

    set(src $<TARGET_FILE:${id_name}>)

    if(NOT EXISTS "${directory}/Bin")
        file(MAKE_DIRECTORY "${directory}/Bin")
    endif()

    set(dest "${directory}/Bin/${id_name}${CMAKE_SHARED_LIBRARY_SUFFIX}")

    # Add post-build command to copy the library
    add_custom_command(TARGET ${id_name}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${src} ${dest}
            COMMAND ${CMAKE_COMMAND} -E echo "Successfully copied ${src} to ${dest}")

    add_dependencies(${id_name} ${TARGET_NAME}_VST3)

    message(STATUS "Added project: ${id_name}")

endfunction()

add_plugin_library(1 Gain)
add_plugin_library(2 Panning)