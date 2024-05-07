// Copyright (c) 2024 Meerkat
#ifndef INC_QUEUEFAMILIES_H_
#define INC_QUEUEFAMILIES_H_

#include "Common.h"
#include <vector>
#include <optional>

namespace vkUtil {

struct QueueFamilyIndices {
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;

  bool isComplete() {
    return graphicsFamily.has_value()
      && presentFamily.has_value();
  }
};

inline QueueFamilyIndices findQueueFamilies(
  vk::PhysicalDevice device,
  vk::SurfaceKHR surface,
  bool debug) {
  QueueFamilyIndices indices;

  std::vector<vk::QueueFamilyProperties> queueFamilies =
    device.getQueueFamilyProperties();

  if (debug) {
    printf("System can support %ld queue families.\n",
           queueFamilies.size());
  }

  uint32_t i = 0;
  for (const vk::QueueFamilyProperties& qfp : queueFamilies) {
    if (qfp.queueFlags & vk::QueueFlagBits::eGraphics) {
      indices.graphicsFamily = i;

      if (debug) {
        printf("Queue family %d is suitable for graphics.\n", i);
      }
    }

    if (device.getSurfaceSupportKHR(i, surface)) {
      indices.presentFamily = i;

      if (debug) {
        printf("Queue family %d is suitable for presenting.\n", i);
      }
    }

    if (indices.isComplete()) {
      return indices;
    }
    ++i;
  }

  return indices;
}

}  // namespace vkUtil

#endif  // INC_QUEUEFAMILIES_H_

