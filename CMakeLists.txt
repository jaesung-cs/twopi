cmake_minimum_required(VERSION 3.0)

project(twopi)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED true)

# setup vcpkg toolchain
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake)

include(${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake)

find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_path(STB_INCLUDE_DIRS "stb.h")

# Assimp is installed via homebrew
find_package(assimp CONFIG REQUIRED)

set(twopi_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(twopi_SOURCE_FILES
  # applicaiton
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/application/application.cc
  # geometry
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/geometry/mesh.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/geometry/mesh_loader.cc
  # gl
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/gl/gl_engine.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/gl/gl_geometry.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/gl/gl_shader.cc
  # scene
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/scene/camera.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/scene/camera_control.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/scene/camera_orbit_control.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/scene/color_material.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/scene/material.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/scene/scene.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/scene/scene_node.cc
  # window
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/window/event.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/window/keyboard_event.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/window/mouse_button_event.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/window/mouse_move_event.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/window/mouse_wheel_event.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/window/glfw_window.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/window/window.cc
  # main
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/main.cc
)

set(twopi_SOURCE_FILES
  # applicaiton
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/application/application.cc
  # geometry
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/geometry/mesh.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/geometry/mesh_loader.cc
  # gl
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/gl/gl_engine.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/gl/gl_geometry.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/gl/gl_shader.cc
  # scene
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/scene/camera.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/scene/camera_control.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/scene/camera_orbit_control.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/scene/color_material.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/scene/material.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/scene/scene.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/scene/scene_node.cc
  # window
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/window/event/event.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/window/event/keyboard_event.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/window/event/mouse_button_event.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/window/event/mouse_move_event.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/twopi/window/event/mouse_wheel_event.cc
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
target_link_libraries(twopi PUBLIC assimp::assimp)
