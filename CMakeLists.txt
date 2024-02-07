cmake_minimum_required(VERSION 3.12)
project(VulkanTutorial CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRCS_DIR ${PROJECT_SOURCE_DIR}/src)
set(HDRS_DIR ${PROJECT_SOURCE_DIR}/inc)
set(SHDS_DIR ${SRCS_DIR}/shaders)
set(EXT_DIR ${PROJECT_SOURCE_DIR}/ext)

file(GLOB_RECURSE SRCS ${SRCS_DIR}/*.cpp)
file(GLOB_RECURSE HDRS ${HDRS_DIR}/*.h)
file(GLOB_RECURSE SHDS ${SHDS_DIR}/*.vert ${SHDS_DIR}/*.frag)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_SOURCE_DIR}/bin>)
set(BIN_DIR ${PROJECT_SOURCE_DIR}/bin)
set(SHADERS_OUT_DIR ${BIN_DIR}/shaders)
file(MAKE_DIRECTORY ${BIN_DIR})
file(MAKE_DIRECTORY ${SHADERS_OUT_DIR})

# CHECK DEPENDENCIES
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

if(NOT EXISTS ${EXT_DIR}/glfw/CMakeLists.txt)
  message(FATAL_ERROR "The glfw submodule was not downloaded! GIT_SUBMODULE was truned off or failed. Please update submodules and try again.")
endif()
if(NOT EXISTS ${EXT_DIR}/glm/CMakeLists.txt)
  message(FATAL_ERROR "The glm submodule was not downloaded! GIT_SUBMODULE was truned off or failed. Please update submodules and try again.")
endif()
if(NOT EXISTS ${EXT_DIR}/stb/README.md)
  message(FATAL_ERROR "The stb submodule was not downloaded! GIT_SUBMODULE was truned off or failed. Please update submodules and try again.")
endif()

add_subdirectory("ext/glfw")
add_subdirectory("ext/glm")

# STB module
set(STB_DIR ${EXT_DIR}/stb)
# include_directories(${STB_DIR})
file(GLOB STB_FILES ${STB_DIR}/stb_image.h)
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${STB_DIR})
target_sources(stb INTERFACE ${STB_FILES})

# VULKAN SDK
if(NOT DEFINED {VULKAN_SDK})
  if(NOT DEFINED ENV{VULKAN_SDK})
    message(FATAL_ERROR "VULKAN_SDK not defined. Please provide a valid environment variable to the vulkan SDK.")
  endif()
  set(VULKAN_SDK "$ENV{VULKAN_SDK}")
endif()
message("Vulkan SDK path: ${VULKAN_SDK}")

# SHADERS COMPILE
# set(GLSL_DEBUG_FLAG $<$<CONFIG:Debug>:-g>)
set(GLSL_COMPILER ${VULKAN_SDK}/bin/glslc)

foreach(SHADER ${SHDS})
  get_filename_component(SHADER_NAME ${SHADER} NAME)
  set(SHADER_OUT_NAME ${SHADERS_OUT_DIR}/${SHADER_NAME}.spv)
  list(APPEND SHADER_OUT_NAMES ${SHADER_OUT_NAME})
  add_custom_command(
    MAIN_DEPENDENCY ${SHADER}
    OUTPUT ${SHADER_OUT_NAME}
    COMMAND ${GLSL_COMPILER} ${SHADER} "-o" ${SHADER_OUT_NAME}
    VERBATIM
  )
endforeach()

add_custom_target(build_shaders DEPENDS ${SHADER_OUT_NAMES})

# PROJECT COMPILE
add_executable(${PROJECT_NAME} ${HDRS} ${SRCS})

find_path(VULKAN_INC vulkan/vulkan.h
          PATHS ${VULKAN_SDK}/Include ${VULKAN_SDK}/include
          NOT_DEFAULT_PATH
)

target_include_directories(${PROJECT_NAME}
  PRIVATE
  "ext/glfw"
  "ext/glm"
  ${VULKAN_INC}
  ${STB_DIR}
)

set(VULKAN_LIB_DIR ${VULKAN_SDK}/lib ${VULKAN_SDK}/Lib)
find_library(VULKAN_LIB
             NAMES vulkan vulkan-1
             PATHS ${VULKAN_LIB_DIR}
             NOT_DEFAULT_PATH
)

target_link_libraries(${PROJECT_NAME}
  ${VULKAN_LIB}
  glfw
  glm
  stb
)

add_dependencies(${PROJECT_NAME} build_shaders)
