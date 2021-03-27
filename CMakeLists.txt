cmake_minimum_required(VERSION 3.7)

project(twopi)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

# setup vcpkg toolchain
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake)

include(${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake)

find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_path(STB_INCLUDE_DIRS "stb.h")
find_package(Vulkan REQUIRED)

# Assimp is installed via homebrew
find_package(ASSIMP REQUIRED)
set_target_properties(assimp::assimp PROPERTIES MAP_IMPORTED_CONFIG_DEBUG Release)

set(twopi_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(twopi_SOURCE_FILES
  # applicaiton
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/application/application.cc
  # geometry
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/geometry/image.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/geometry/image_loader.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/geometry/mesh.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/geometry/mesh_loader.cc
  # scene
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/scene/camera.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/scene/camera_control.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/scene/camera_orbit_control.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/scene/color_material.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/scene/light.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/scene/material.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/scene/scene.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/scene/scene_node.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/scene/vr_camera.cc
  # vk
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/vk/vk_command_buffer.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/vk/vk_command_pool.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/vk/vk_device.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/vk/vk_engine.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/vk/vk_fence.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/vk/vk_framebuffer.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/vk/vk_graphics_pipeline.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/vk/vk_image.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/vk/vk_image_view.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/vk/vk_instance.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/vk/vk_physical_device.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/vk/vk_pipeline_layout.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/vk/vk_queue.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/vk/vk_render_pass.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/vk/vk_semaphore.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/vk/vk_shader_module.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/vk/vk_surface.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/vk/vk_swapchain.cc
  # window
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/window/event/event.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/window/event/keyboard_event.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/window/event/mouse_button_event.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/window/event/mouse_move_event.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/window/event/mouse_wheel_event.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/window/event/resize_event.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/window/glfw_window.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/window/window.cc
  # main
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/main.cc
)

add_executable(twopi ${twopi_SOURCE_FILES})
target_include_directories(twopi PRIVATE ${twopi_INCLUDE_DIRS})

target_link_libraries(twopi PRIVATE glad::glad)
target_link_libraries(twopi PRIVATE glfw)
target_link_libraries(twopi PRIVATE glm::glm)
target_include_directories(twopi PRIVATE ${STB_INCLUDE_DIRS})
target_link_libraries(twopi PRIVATE assimp::assimp)
target_include_directories(twopi PRIVATE ${Vulkan_INCLUDE_DIR})
target_link_libraries(twopi PRIVATE ${Vulkan_LIBRARIES})
