cmake_minimum_required(VERSION 3.1)

set (CMAKE_CXX_STANDARD 11)

project(PaintPrint)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5Core)
find_package(Qt5Gui)

add_executable(${PROJECT_NAME} "src/main.cpp" "src/pp_layer.hpp" "src/pp_layerdiagonal.hpp" "src/pp_layermorph.hpp" "src/pp_project.hpp" "src/pp_tool.hpp" "src/pp_utils.hpp" "README.md")

target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Gui)
