// Copyright (c) 2024 Meerkat
#ifndef INC_DEVICE_H_
#define INC_DEVICE_H_

#include "Common.h"
#include "Logging.h"
#include "QueueFamilies.h"
#include <array>
#include <vector>
#include <string>

namespace vkInit {

inline bool checkDeviceExtensionSupport(
  const vk::PhysicalDevice& device,
  const std::vector<const char*> extensions,
  bool debug) {
  const std::vector<vk::ExtensionProperties> extensionsSupported =
    device.enumerateDeviceExtensionProperties();

  if (debug) {
    printf("Device can support the following extensions:\n");
    for (const vk::ExtensionProperties& e : extensionsSupported) {
      printf("\t%s\n", e.extensionName.data());
    }
  }

  for (const char* e : extensions) {
    bool found = false;
    for (const vk::ExtensionProperties& es : extensionsSupported) {
      if (strcmp(e, es.extensionName.data()) == 0) {
        found = true;
        printf("Device extension \"%s\" is supported\n", e);
        break;
      }
    }
    if (!found) {
      return false;
    }
  }

  return true;
}

inline bool isSuitable(const vk::PhysicalDevice& device, bool debug) {
  if (debug) {
    printf("Checking if device is suitable.\n");
  }

  const std::vector<const char*> requestedExtensions {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
  };

  if (debug) {
    printf("Requesting device extensions:\n");
    for (const char* e : requestedExtensions) {
      printf("\t%s\n", e);
    }
  }

  bool extensionSupported =
    checkDeviceExtensionSupport(device, requestedExtensions, debug);

  return extensionSupported;
}

inline vk::PhysicalDevice choose_physical_device(
  vk::Instance instance, bool debug) {
  if (debug) {
    printf("Choosing physical device...\n");
  }

  std::vector<vk::PhysicalDevice> availableDevices =
    instance.enumeratePhysicalDevices();

  if (debug) {
    printf("There are %ld physical devices available\n",
           availableDevices.size());
  }

  vk::PhysicalDevice result{};
  for (const vk::PhysicalDevice& device : availableDevices) {
    if (debug) {
      vkInit::log_device_properties(device);
    }
    if (isSuitable(device, debug)) {
      result = device;
      break;
    }
  }

  return result;
}


inline vk::Device create_logical_device(
  vk::PhysicalDevice physicalDevice,
  vk::SurfaceKHR surface,
  bool debug) {
  vkUtils::QueueFamilyIndices indices =
    vkUtils::findQueueFamilies(physicalDevice, surface, debug);

  std::vector<uint32_t> uniqueIndices;
  uniqueIndices.push_back(indices.graphicsFamily.value());
  if (indices.graphicsFamily.value() != indices.presentFamily.value()) {
    uniqueIndices.push_back(indices.presentFamily.value());
  }

  float queuePriority = 1.0f;

  std::vector<vk::DeviceQueueCreateInfo> queueCreateInfo;
  for (const uint32_t& index : uniqueIndices) {
    queueCreateInfo.push_back(
      vk::DeviceQueueCreateInfo(
        vk::DeviceQueueCreateFlags(),
        index,
        1, &queuePriority));
  }

  std::vector<const char*> deviceExtensions {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
  };

  vk::PhysicalDeviceFeatures deviceFeatures = vk::PhysicalDeviceFeatures();

  std::vector<const char*> enabledLayers;
  if (debug) {
    enabledLayers.push_back("VK_LAYER_KHRONOS_validation");
  }

  vk::DeviceCreateInfo deviceInfo = vk::DeviceCreateInfo(
    vk::DeviceCreateFlags(),
    queueCreateInfo.size(), queueCreateInfo.data(),
    enabledLayers.size(), enabledLayers.data(),
    deviceExtensions.size(), deviceExtensions.data(),
    &deviceFeatures);

  vk::Device device = nullptr;
  try {
    device = physicalDevice.createDevice(deviceInfo);

    if (debug) {
      printf("GPU device has been successfully created.\n");
    }
  } catch (vk::SystemError err) {
    if (debug) {
      printf("Device creation failed: %s\n.", err.what());
    }
  }

  return device;
}

inline std::array<vk::Queue, 2> get_queue(vk::PhysicalDevice physicalDevice,
                                        vk::Device device,
                                        vk::SurfaceKHR surface,
                                        bool debug) {
  vkUtils::QueueFamilyIndices indices =
    vkUtils::findQueueFamilies(physicalDevice, surface, debug);

  std::array queues = {
    device.getQueue(indices.graphicsFamily.value(), 0),
    device.getQueue(indices.presentFamily.value(), 0)
  };

  return queues;
}

}  // namespace vkInit

#endif  // INC_DEVICE_H_

