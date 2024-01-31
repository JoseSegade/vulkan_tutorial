// Copyright (c) 2024 Meerkat
#ifndef INC_SHADERS_H_
#define INC_SHADERS_H_

#include "Common.h"
#include <fstream>
#include <vector>
#include <string>

namespace vkUtil {

inline std::vector<char> readFile(std::string filename, bool debug) {
  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  if (debug && !file.is_open()) {
    printf("Failed to load %s.\n", filename.c_str());
  }

  size_t filesize = static_cast<size_t>(file.tellg());

  std::vector<char> buffer(filesize);
  file.seekg(0);
  file.read(buffer.data(), filesize);

  file.close();
  return buffer;
}

}  // namespace vkUtil

#endif  // INC_SHADERS_H_
