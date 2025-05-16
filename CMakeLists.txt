cmake_minimum_required(VERSION 3.22)

include(FetchContent)
FetchContent_Declare(vcpkg
        GIT_REPOSITORY "https://github.com/microsoft/vcpkg/"
)
FetchContent_MakeAvailable(vcpkg)

set(ENV{VCPKG_MAX_CONCURRENCY} 6)
set(CMAKE_TOOLCHAIN_FILE "${vcpkg_SOURCE_DIR}/scripts/buildsystems/vcpkg.cmake" CACHE FILEPATH "")
set(VCPKG_TRACE_FIND_PACKAGE true)

project(Visualizer VERSION 0.1 LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


set(PROJECT_SOURCES
        src/main.cpp
        src/main_window.cpp
        src/general/slam_creater.cpp
)

set(HEADER_FILES
        include/general/SharedMemory/info.h
        include/general/SharedMemory/threaded_multi_reader_handler.h
        include/main_window.h
        include/cartesians.h
        include/lidar_data.h
        include/general/slam_creater.h
)

set(RESOURCES
        shaders/default_fragment.frag
        shaders/default_vertex.vert
        shaders/lidar.frag
        shaders/lidar.vert
)

set(BUILD_GRAPHICS ON)
add_subdirectory(lib/HUH)
set(Boost_USE_DEBUG_RUNTIME OFF)
set(VTK_MODULE_ENABLE_VTK_FiltersExtraction ON)
find_package(VTK)
find_package(PCL CONFIG REQUIRED)
find_path(OSMIUM_INCLUDE_DIRS "osmium/version.hpp")

add_executable(Visualizer
        ${PROJECT_SOURCES}
        ${HEADER_FILES}
)


target_link_libraries(Visualizer PRIVATE
        HUH::Graphics
        ${PCL_LIBRARIES}
)

target_include_directories(Visualizer
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE ${OSMIUM_INCLUDE_DIRS}
)


install(TARGETS Visualizer
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)


foreach (file ${RESOURCES})
    string(REPLACE "/" "_" n_file ${file})
    ADD_CUSTOM_TARGET(
            copy_${n_file}_file
            ${CMAKE_COMMAND}
            -D SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
            -D DESTINATION_DIR=${CMAKE_CURRENT_BINARY_DIR}
            -D file=${file}
            -P copy.cmake
            COMMENT "Running custom target for file: ${file} in project: ${CMAKE_PROJECT_NAME}" VERBATIM
    )
    ADD_DEPENDENCIES(${CMAKE_PROJECT_NAME} copy_${n_file}_file)
endforeach ()

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/copy.cmake DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
