cmake_minimum_required(VERSION 3.12)
project(VulkanTutorial CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE HDRS ${CMAKE_CURRENT_SOURCE_DIR}/inc/*.h)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_SOURCE_DIR}/bin>)

add_subdirectory("ext/glfw")

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS ${PROJECT_SOURCE_DIR}/.git)
  option(GIT_SUBMODULE "Check submodules during build" ON)
  if(GIT_SUBMODULE)
    message(STATUS "Submodule update")
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    RESULT_VARIABLE GIT_SUBMOD_RESULT)
    if(NOT GIT_SUBMOD_RESULT EQUAL "0")
      message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
    endif()
  endif()
endif()

if(NOT EXISTS ${PROJECT_SOURCE_DIR}/ext/glfw/CMakeLists.txt)
  message(FATAL_ERROR "The glfw submodule was not downloaded! GIT_SUBMODULE was truned off or failed. Please update submodules and try again.")
endif()

if(NOT DEFINED {VULKAN_SDK})
  if(NOT DEFINED ENV{VULKAN_SDK})
    message(FATAL_ERROR "VULKAN_SDK not defined. Please provide a valid environment variable to the vulkan SDK.")
  endif()
  set(VULKAN_SDK "$ENV{VULKAN_SDK}")
endif()
message("Vulkan SDK path: ${VULKAN_SDK}")

add_executable(${PROJECT_NAME} ${HDRS} ${SRCS})

find_path(VULKAN_INC vulkan/vulkan.h
          PATHS ${VULKAN_SDK}/Include ${VULKAN_SDK}/include
          NOT_DEFAULT_PATH
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
  "ext/glfw"
  ${VULKAN_INC}
)

set(VULKAN_LIB_DIR ${VULKAN_SDK}/lib ${VULKAN_SDK}/Lib)
find_library(VULKAN_LIB
             NAMES vulkan vulkan-1
             PATHS ${VULKAN_LIB_DIR}
             NOT_DEFAULT_PATH
)

target_link_libraries(VulkanTutorial ${VULKAN_LIB} glfw)
