cmake_minimum_required(VERSION 3.20)

set(CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_CMAKE})

project(BlueVK)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

find_program(GLSL_VALIDATOR glslangValidator HINTS /usr/bin /usr/local/bin $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/)

file(GLOB GLSL_SOURCE_FILES
    "${PROJECT_SOURCE_DIR}/assets/shaders/*.vert"
    "${PROJECT_SOURCE_DIR}/assets/shaders/*.frag"
    "${PROJECT_SOURCE_DIR}/assets/shaders/*.comp"
    "${PROJECT_SOURCE_DIR}/assets/shaders/*.glsl"
)

foreach(GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${CMAKE_CURRENT_BINARY_DIR}/assets/shaders/${FILE_NAME}.spv")
    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND
        ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
        DEPENDS ${GLSL}
    )
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(Shaders DEPENDS ${SPIRV_BINARY_FILES})

file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})

find_package(Vulkan REQUIRED)
find_package(SFML COMPONENTS system window graphics CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(ImGui-SFML CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(vk-bootstrap CONFIG REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(fastgltf CONFIG REQUIRED)

find_path(STB_INCLUDE_DIR NAMES stb_image.h)

include_directories(include)
include_directories(${STB_INCLUDE_DIR})

file(GLOB SOURCES "src/*.cpp")

set(TARGET BlueVK)

add_executable(${TARGET} ${SOURCES})

add_dependencies(${TARGET} Shaders)

target_link_libraries(${TARGET}
    PRIVATE Vulkan::Vulkan
    PRIVATE sfml-system sfml-network sfml-graphics sfml-window
    PRIVATE imgui::imgui
    PRIVATE ImGui-SFML::ImGui-SFML
    PRIVATE glm::glm
    PRIVATE fmt::fmt
    PRIVATE vk-bootstrap::vk-bootstrap
    PRIVATE GPUOpen::VulkanMemoryAllocator
    PRIVATE fastgltf::fastgltf
)
