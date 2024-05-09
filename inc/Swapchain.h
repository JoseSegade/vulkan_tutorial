// Copyright (c) 2024 Meerkat
#ifndef INC_SWAPCHAIN_H_
#define INC_SWAPCHAIN_H_

#include "Common.h"
#include "Logging.h"
#include "Frame.h"
#include "QueueFamilies.h"
#include "Image.h"
#include <vector>
#include <string>
#include <algorithm>

namespace vkInit {

struct SwapChainSupportDetails {
  vk::SurfaceCapabilitiesKHR        capabilities;
  std::vector<vk::SurfaceFormatKHR> formats;
  std::vector<vk::PresentModeKHR>   presentModes;
};


struct SwapChainBundle {
  vk::SwapchainKHR                    swapchain;
  std::vector<vkUtil::SwapChainFrame> frames;
  vk::Format                          format;
  vk::Extent2D                        extent;
};

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
  for (const vk::SurfaceFormatKHR& f : formats) {
    if (f.format == vk::Format::eB8G8R8A8Unorm
      && f.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
      return f;
    }
  }
  return formats.at(0);
}

inline vk::PresentModeKHR choose_swapchain_present_mode(
  std::vector<vk::PresentModeKHR> presentModes) {
  for (const vk::PresentModeKHR m : presentModes) {
    if (m == vk::PresentModeKHR::eMailbox) {
      return m;
    } else if (m == vk::PresentModeKHR::eImmediate) {
      return m;
    }
  }
  return presentModes.at(0);
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

  vkUtil::QueueFamilyIndices indices =
    vkUtil::findQueueFamilies(physicalDevice, surface, debug);
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

  std::vector<vk::Image> images =
    device.getSwapchainImagesKHR(bundle.swapchain);
  bundle.frames.resize(images.size());
  for (size_t i = 0; i < images.size(); ++i) {
    bundle.frames[i].mDevice = device;
    bundle.frames[i].mPhysicalDevice = physicalDevice;

    bundle.frames[i].mImage = images[i];
    bundle.frames[i].mImageView = vkImage::make_image_view(
      device,
      images[i],
      format.format,
      vk::ImageAspectFlagBits::eColor,
      vk::ImageViewType::e2D,
      1
    );
  }

  bundle.format = format.format;
  bundle.extent = extent;

  return bundle;
}

}  // namespace vkInit

#endif  // INC_SWAPCHAIN_H_
