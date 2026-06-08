cmake_minimum_required(VERSION 3.22)
cmake_policy(SET CMP0146 OLD)

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
#set(CMAKE_CUDA_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CUDA_STANDARD_REQUIRED ON)
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

set(ENV{NINJA_STATUS} "[%p ] ")

set(BUILD_GRAPHICS_CAMERA ON)
set(BUILD_MATH ON)
set(USE_SIMD ON)
set(BUILD_RHI ON)
set(BUILD_WINDOW ON)
set(BUILD_CUDA ON)
add_subdirectory(lib/HUH)

add_executable(Visualizer
        ${PROJECT_SOURCES}
        ${HEADER_FILES}
)

find_package(CUDA)
if (CUDA_FOUND)
    enable_language(CUDA)
    set(CMAKE_CUDA23_STANDARD_COMPILE_OPTION "-std=c++23")
    set(CMAKE_CUDA23_EXTENSION_COMPILE_OPTION "")
    #    set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/cuda_kernels/plane_ransac.cu PROPERTIES LANGUAGE CUDA)
    #    add_custom_target(plane_ransac ALL COMMAND ${CUDA_NVCC_EXECUTABLE}
    #            -ptx --std=c++23 -I/home/palnit/Desktop/code/ElteCar/ElteCar-Visualizer/lib/HUH/modules/Math/include/public/ -I/home/palnit/Desktop/code/ElteCar/ElteCar-Visualizer/lib/HUH/include/public/ ${CMAKE_CURRENT_SOURCE_DIR}/cuda_kernels/plane_ransac.cu SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/cuda_kernels/plane_ransac.cu WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

    add_library(CudaPtxFiles OBJECT cuda_kernels/plane_ransac.cu)
    set_property(TARGET CudaPtxFiles PROPERTY CUDA_PTX_COMPILATION ON)
    target_compile_options(CudaPtxFiles PRIVATE --std=c++23 -O0)
    target_link_libraries(CudaPtxFiles PUBLIC HUH::Math)

    add_custom_command(TARGET Visualizer POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_OBJECTS:CudaPtxFiles>
            ${CMAKE_CURRENT_BINARY_DIR}/
            COMMENT "Copying data"
    )

endif ()


target_link_libraries(Visualizer PRIVATE
        HUH::Core
        HUH::RHI
        HUH::Math
        HUH::Window
        HUH::Graphics
        HUH::Cuda
        HUH::FileHandling::Image
)

target_include_directories(Visualizer
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)


install(TARGETS Visualizer
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)


#add_custom_command(TARGET Visualizer POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy
#        "$<LIST:TRANSFORM,${RESOURCES},REPLACE,.*\/(.*),\\1>"
#        "${CMAKE_BINARY_DIR}/VulkanShader/"
#        COMMENT "Copying data"
#)
# TODO SHADERS
foreach (file ${RESOURCES})
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/${file} DESTINATION ${CMAKE_BINARY_DIR}/VulkanShader/)
endforeach ()

