// Copyright (c) 2024 Meerkat
#ifndef INC_DEVICE_H_
#define INC_DEVICE_H_

#include "Common.h"
#include "Logging.h"
#include <array>
#include <vector>
#include <string>
#include <optional>
#include <algorithm>

namespace vkInit {

struct QueueFamilyIndices {
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;

  bool isComplete() {
    return graphicsFamily.has_value()
      && presentFamily.has_value();
  }
};

struct SwapChainSupportDetails {
  vk::SurfaceCapabilitiesKHR        capabilities;
  std::vector<vk::SurfaceFormatKHR> formats;
  std::vector<vk::PresentModeKHR>   presentModes;
};

struct SwapChainBundle {
  vk::SwapchainKHR       swapchain;
  std::vector<vk::Image> images;
  vk::Format             format;
  vk::Extent2D           extent;
};

inline void log_device_properties(const vk::PhysicalDevice& device) {
  const vk::PhysicalDeviceProperties& properties = device.getProperties();

  std::string deviceType = "";
  switch (properties.deviceType) {
    case (vk::PhysicalDeviceType::eCpu):
      deviceType = "CPU";
      break;
    case (vk::PhysicalDeviceType::eDiscreteGpu):
      deviceType = "discrete CPU";
      break;
    case (vk::PhysicalDeviceType::eVirtualGpu):
      deviceType = "virtual GPU";
      break;
    case (vk::PhysicalDeviceType::eIntegratedGpu):
      deviceType = "integrated GPU";
      break;
    case (vk::PhysicalDeviceType::eOther):
    default:
      deviceType = "other";
      break;
  }
  printf("Device name: %s\n"
         "Device type: %s\n",
         properties.deviceName.data(),
         deviceType.c_str());
}

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
      log_device_properties(device);
    }
    if (isSuitable(device, debug)) {
      result = device;
      break;
    }
  }

  return result;
}

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
      break;
    }
    ++i;
  }

  return indices;
}

inline vk::Device create_logical_device(
  vk::PhysicalDevice physicalDevice,
  vk::SurfaceKHR surface,
  bool debug) {
  QueueFamilyIndices indices =
    findQueueFamilies(physicalDevice, surface, debug);

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
  QueueFamilyIndices indices =
    findQueueFamilies(physicalDevice, surface, debug);

  std::array queues = {
    device.getQueue(indices.graphicsFamily.value(), 0),
    device.getQueue(indices.presentFamily.value(), 0)
  };

  return queues;
}

inline SwapChainSupportDetails query_swap_chain_support(
  vk::PhysicalDevice device,
  vk::SurfaceKHR surface,
  bool debug) {
  SwapChainSupportDetails support;

  support.capabilities = device.getSurfaceCapabilitiesKHR(surface);

  if (debug) {
    printf("SwapChain can support the following surface capabilities:\n"
           "\tMinimum image count: %d\n"
           "\tMaximum image count: %d\n"
           "\tMinimum supported extent:\n"
           "\t\twidth: %d\n"
           "\t\theight: %d\n"
           "\tMaximum supported extent:\n"
           "\t\twidth: %d\n"
           "\t\theight: %d\n"
           "\tMaximum array layers: %d\n"
           ,
           support.capabilities.minImageCount,
           support.capabilities.maxImageCount,
           support.capabilities.minImageExtent.width,
           support.capabilities.minImageExtent.height,
           support.capabilities.maxImageExtent.width,
           support.capabilities.maxImageExtent.height,
           support.capabilities.maxImageArrayLayers);
    printf("\tSupported transforms:\n");
    std::vector<std::string> list =
      log_transform_bits(support.capabilities.supportedTransforms);
    for (const std::string& l : list) {
      printf("\t\t%s\n", l.c_str());
    }
    printf("\tCurrent transforms:\n");
    list =
      log_transform_bits(support.capabilities.currentTransform);
    for (const std::string& l : list) {
      printf("\t\t%s\n", l.c_str());
    }
    printf("\tSupported alpha opperations:\n");
    list =
      log_alpha_composite_bits(support.capabilities.supportedCompositeAlpha);
    for (const std::string& l : list) {
      printf("\t\t%s\n", l.c_str());
    }
    printf("\tSupported image usage:\n");
    list =
      log_image_usage_bits(support.capabilities.supportedUsageFlags);
    for (const std::string& l : list) {
      printf("\t\t%s\n", l.c_str());
    }
  }

  support.formats = device.getSurfaceFormatsKHR(surface);

  if (debug) {
    for (const vk::SurfaceFormatKHR& f : support.formats) {
      printf("Supported pixel format: %s\n",
             vk::to_string(f.format).c_str());
      printf("Supported color space: %s\n",
             vk::to_string(f.colorSpace).c_str());
    }
  }

  support.presentModes = device.getSurfacePresentModesKHR(surface);

  if (debug) {
    printf("Supported present modes:\n");
    for (const vk::PresentModeKHR m : support.presentModes) {
      printf("\t%s\n",
             log_present_mode(m).c_str());
    }
  }

  return support;
}

inline vk::SurfaceFormatKHR choose_swapchain_surface_format(
  const std::vector<vk::SurfaceFormatKHR>& formats) {
  vk::SurfaceFormatKHR result = formats[0];

  for (const vk::SurfaceFormatKHR& f : formats) {
    if (f.format == vk::Format::eB8G8R8A8Unorm
      && f.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
      result = f;
      break;
    }
  }

  return result;
}

inline vk::PresentModeKHR choose_swapchain_present_mode(
  std::vector<vk::PresentModeKHR> presentModes) {
  vk::PresentModeKHR result = vk::PresentModeKHR::eFifo;

  for (const vk::PresentModeKHR m : presentModes) {
    if (m == vk::PresentModeKHR::eMailbox) {
      result = m;
      break;
    }
  }

  return result;
}

inline vk::Extent2D choose_swapchain_extent(
  uint32_t width, uint32_t height,
  const vk::SurfaceCapabilitiesKHR& capabilities) {
  vk::Extent2D result { 1, 1 };

  if (capabilities.currentExtent.width != UINT32_MAX) {
    result = capabilities.currentExtent;
  } else {
    result = vk::Extent2D{ width, height };

    result.width = std::min(
      capabilities.maxImageExtent.width,
      std::max(capabilities.minImageExtent.width, width));
    result.height = std::min(
      capabilities.maxImageExtent.height,
      std::max(capabilities.minImageExtent.height, height));
  }

  return result;
}

inline SwapChainBundle create_swapchain(
  vk::PhysicalDevice physicalDevice, vk::Device device,
  vk::SurfaceKHR surface, uint32_t width, uint32_t height,
  bool debug) {
  SwapChainSupportDetails support =
    query_swap_chain_support(physicalDevice, surface, debug);

  vk::SurfaceFormatKHR format =
    choose_swapchain_surface_format(support.formats);
  vk::PresentModeKHR presentMode =
    choose_swapchain_present_mode(support.presentModes);
  vk::Extent2D extent =
    choose_swapchain_extent(width, height, support.capabilities);

  uint32_t imageCount = std::min(
    support.capabilities.maxImageCount,
    support.capabilities.minImageCount + 1);

  vk::SwapchainCreateInfoKHR createInfo = vk::SwapchainCreateInfoKHR(
    vk::SwapchainCreateFlagsKHR(),
    surface,
    imageCount,
    format.format,
    format.colorSpace,
    extent,
    1,
    vk::ImageUsageFlagBits::eColorAttachment);

  QueueFamilyIndices indices =
    findQueueFamilies(physicalDevice, surface, debug);
  uint32_t queueFamilyIndices[] = {
    indices.graphicsFamily.value(),
    indices.presentFamily.value() };

  if (indices.graphicsFamily != indices.presentFamily.value()) {
    createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    createInfo.imageSharingMode = vk::SharingMode::eExclusive;
  }

  createInfo.preTransform = support.capabilities.currentTransform;
  createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;

  createInfo.oldSwapchain = vk::SwapchainKHR(nullptr);

  SwapChainBundle bundle{};
  try {
    bundle.swapchain = device.createSwapchainKHR(createInfo);
  } catch (vk::SystemError err) {
    throw std::runtime_error(
      "Failed to create swapchain: " + std::string(err.what()));
  }

  bundle.images = device.getSwapchainImagesKHR(bundle.swapchain);
  bundle.format = format.format;
  bundle.extent = extent;

  return bundle;
}

}  // namespace vkInit

#endif  // INC_DEVICE_H_

