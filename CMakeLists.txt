cmake_minimum_required(VERSION 3.12)
project(VulkanTutorial CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE HDRS ${CMAKE_CURRENT_SOURCE_DIR}/inc/*.h)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_SOURCE_DIR}/bin>)

add_executable(${PROJECT_NAME} ${HDRS} ${SRCS})

target_link_libraries(VulkanTutorial vulkan glfw)
