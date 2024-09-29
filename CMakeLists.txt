cmake_minimum_required(VERSION 3.22)

include(FetchContent)
FetchContent_Declare(vcpkg
        GIT_REPOSITORY "https://github.com/microsoft/vcpkg/"
)
FetchContent_MakeAvailable(vcpkg)

set(CMAKE_TOOLCHAIN_FILE "${vcpkg_SOURCE_DIR}/scripts/buildsystems/vcpkg.cmake" CACHE FILEPATH "")
set(VCPKG_TRACE_FIND_PACKAGE true)

project(Visualizer VERSION 0.1 LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


set(PROJECT_SOURCES
   src/main.cpp
    src/general/OpenGL_SDL/basic_window.cpp
    src/general/OpenGL_SDL/SDL_GLDebugMessageCallback.cpp
    src/general/OpenGL_SDL/file_handling.cpp
    src/general/OpenGL_SDL/generic_structs.cpp
    src/general/OpenGL_SDL/vertex_buffer_object.cpp
    src/general/OpenGL_SDL/vertex_array_object.cpp
    src/general/OpenGL_SDL/element_buffer_object.cpp
    src/general/OpenGL_SDL/shader_program.cpp
    src/main_window.cpp
)

set(HEADER_FILES
    include/general/OpenGL_SDL/basic_window.h
    include/general/OpenGL_SDL/SDL_GLDebugMessageCallback.h
    include/general/OpenGL_SDL/file_handling.h
    include/general/OpenGL_SDL/generic_structs.h
    include/general/OpenGL_SDL/vertex_buffer_object.h
    include/general/OpenGL_SDL/vertex_array_object.h
    include/general/OpenGL_SDL/element_buffer_object.h
    include/general/OpenGL_SDL/shader_program.h
    include/general/SharedMemory/info.h
    include/general/SharedMemory/threaded_multi_reader_handler.h
    include/main_window.h
    include/cartesians.h
    include/lidar_data.h
)

set(RESOURCES
    shaders/default_fragment.frag
    shaders/default_vertex.vert
)

find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(implot CONFIG REQUIRED)

add_executable(Visualizer
    ${PROJECT_SOURCES}
    ${HEADER_FILES}
)


target_link_libraries(Visualizer PRIVATE
        $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
        $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>
        imgui::imgui
        GLEW::GLEW
        glm::glm
        OpenGL::GL
        implot::implot
)

target_include_directories(Visualizer
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
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
