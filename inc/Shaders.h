// Copyright (c) 2024 Meerkat
#ifndef INC_SHADERS_H_
#define INC_SHADERS_H_

#include "Common.h"
#include <unistd.h>
#include <limits.h>
#include <fstream>
#include <vector>
#include <string>

namespace vkUtil {

inline std::vector<char> readFile(const std::string& filename, bool debug) {
  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  if (debug && !file.is_open()) {
    char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));
    printf("Failed to load %s, from working dir %s. Error: %s\n",
           filename.c_str(),
           cwd,
           strerror(errno));
  }

  size_t filesize = static_cast<size_t>(file.tellg());

  std::vector<char> buffer(filesize);
  file.seekg(0);
  file.read(buffer.data(), filesize);

  file.close();
  return buffer;
}

inline vk::ShaderModule createModule(
  const std::string& filename, vk::Device device, bool debug) {
  const std::vector<char> sourceCode = readFile(filename, debug);
  vk::ShaderModuleCreateInfo moduleInfo = {};
  moduleInfo.flags    = vk::ShaderModuleCreateFlags(),
  moduleInfo.codeSize = sourceCode.size();
  moduleInfo.pCode    = reinterpret_cast<const uint32_t*>(sourceCode.data());

  vk::ShaderModule sm = nullptr;
  try {
    sm = device.createShaderModule(moduleInfo);
  } catch (vk::SystemError err) {
    if (debug) {
      printf("Error while creating shader module %s. Error: %s\n",
             filename.c_str(), err.what());
    }
  }

  return sm;
}

}  // namespace vkUtil

#endif  // INC_SHADERS_H_
