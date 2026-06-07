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


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-O0)


set(PROJECT_SOURCES
        src/main.cpp
        src/window.cpp
)

set(HEADER_FILES
        include/window.h
        include/general/SharedMemory/info.h
        include/general/SharedMemory/threaded_multi_reader_handler.h
        include/cartesians.h
        include/lidar_data.h
)

set(RESOURCES
        shaders/im_frag.spv
        shaders/im_vert.spv
        shaders/lidar_frag.spv
        shaders/lidar_vert.spv
)

set(BUILD_GRAPHICS_CAMERA ON)
set(BUILD_MATH ON)
set(USE_SIMD ON)
set(BUILD_RHI ON)
set(BUILD_WINDOW ON)
add_subdirectory(lib/HUH)

add_executable(Visualizer
        ${PROJECT_SOURCES}
        ${HEADER_FILES}
)


target_link_libraries(Visualizer PRIVATE
        HUH::Core
        HUH::RHI
        HUH::Math
        HUH::Window
        HUH::Graphics
        HUH::FileHandling::Image
)

target_include_directories(Visualizer
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)


install(TARGETS Visualizer
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)


# TODO SHADERS
foreach (file ${RESOURCES})
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/${file} DESTINATION ${CMAKE_BINARY_DIR}/VulkanShader/)
endforeach ()

