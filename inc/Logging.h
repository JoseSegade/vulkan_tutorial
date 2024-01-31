// Copyright (c) 2024 Meerkat
#ifndef INC_LOGGING_H_
#define INC_LOGGING_H_

#include "Common.h"
#include <vector>
#include <string>

namespace vkInit {

inline VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
  VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
  VkDebugUtilsMessageTypeFlagsEXT messageType,
  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
  void* pUserData) {
  printf("Validation layer: %s\n", pCallbackData->pMessage);
  return VK_FALSE;
}

inline vk::DebugUtilsMessengerEXT make_debug_messenger(vk::Instance instance,
                          vk::DispatchLoaderDynamic dldi) {
  vk::DebugUtilsMessengerCreateInfoEXT createInfo =
    vk::DebugUtilsMessengerCreateInfoEXT(
      vk::DebugUtilsMessengerCreateFlagsEXT(),
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose,
      vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
      vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
      vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
      debugCallback,
      nullptr);

  vk::DebugUtilsMessengerEXT messenger = nullptr;
  try {
    messenger = instance.createDebugUtilsMessengerEXT(
      createInfo, nullptr, dldi);
  } catch (vk::SystemError error) {
    printf("Error while creating debug messenger: %s", error.what());
  }
  return messenger;
}

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

inline std::vector<std::string> log_transform_bits(
  vk::SurfaceTransformFlagsKHR bits) {
  std::vector<std::string> result;

  if (bits & vk::SurfaceTransformFlagBitsKHR::eIdentity) {
    result.push_back("Identity");
  }
  if (bits & vk::SurfaceTransformFlagBitsKHR::eRotate90) {
    result.push_back("90 degree rotation");
  }
  if (bits & vk::SurfaceTransformFlagBitsKHR::eRotate180) {
    result.push_back("180 degree rotation");
  }
  if (bits & vk::SurfaceTransformFlagBitsKHR::eRotate270) {
    result.push_back("270 degree rotation");
  }
  if (bits & vk::SurfaceTransformFlagBitsKHR::eHorizontalMirror) {
    result.push_back("Horizontal mirror");
  }
  if (bits & vk::SurfaceTransformFlagBitsKHR::eHorizontalMirrorRotate90) {
    result.push_back("Horizontal mirror and 90 degree rotation");
  }
  if (bits & vk::SurfaceTransformFlagBitsKHR::eHorizontalMirrorRotate180) {
    result.push_back("Horizontal mirror and 180 degree rotation");
  }
  if (bits & vk::SurfaceTransformFlagBitsKHR::eHorizontalMirrorRotate270) {
    result.push_back("Horizontal mirror and 270 degree rotation");
  }
  if (bits & vk::SurfaceTransformFlagBitsKHR::eInherit) {
    result.push_back("Inherit");
  }

  return result;
}

inline std::vector<std::string> log_alpha_composite_bits(
  vk::CompositeAlphaFlagsKHR bits) {
  std::vector<std::string> result;

  if (bits & vk::CompositeAlphaFlagBitsKHR::eOpaque) {
    result.push_back("Opaque (alpha ignored)");
  }
  if (bits & vk::CompositeAlphaFlagBitsKHR::ePreMultiplied) {
    result.push_back("Pre multiplied");
  }
  if (bits & vk::CompositeAlphaFlagBitsKHR::ePostMultiplied) {
    result.push_back("Post multiplied");
  }
  if (bits & vk::CompositeAlphaFlagBitsKHR::eInherit) {
    result.push_back("Inherit");
  }

  return result;
}

inline std::vector<std::string> log_image_usage_bits(
  vk::ImageUsageFlags bits) {
  std::vector<std::string> result;

  if (bits & vk::ImageUsageFlagBits::eTransferSrc) {
    result.push_back("Transfer SRC: image can be used as the source of a"
                     " transfer command.");
  }
  if (bits & vk::ImageUsageFlagBits::eTransferDst) {
    result.push_back("Transfer DST: image can be used as the destination of a"
                     " transfer command.");
  }
  if (bits & vk::ImageUsageFlagBits::eSampled) {
    result.push_back("Sampled: image can be used to create a VkImageView"
                     " suitable for occupying a VkDescriptorSet slot either"
                     " of type VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE or"
                     " VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, and be"
                     " sampled by a shader.");
  }
  if (bits & vk::ImageUsageFlagBits::eStorage) {
    result.push_back("Storage: image can be used to create a VkImageView"
                     " suitable for occupying a VkDescriptorSet"
                     " slot of type VK_DESCRIPTOR_TYPE_STORAGE_IMAGE.");
  }
  if (bits & vk::ImageUsageFlagBits::eColorAttachment) {
    result.push_back("Color Attachment: Image can be used to create a "
                     "VkImageView suitable for use as a color or"
                     " resolve attachment in a VkFramebuffer.");
  }
  if (bits & vk::ImageUsageFlagBits::eDepthStencilAttachment) {
    result.push_back("Depth/Stencil Attachment: image can be used to create"
                     " a VkImageView suitable for use a a depth/stencil or"
                     " depth/stencil resolve attachment in a VkFramebuffer.");
  }
  if (bits & vk::ImageUsageFlagBits::eTransientAttachment) {
    result.push_back("Transient Attachment: implementations may support using"
                     " memory allocations with the"
                     " VK_MEMORY_PORPERTY_LAZILY_ALLOCATED_BIT to back an"
                     " image with this usage. This bit can be set for any"
                     " image that can be used to create a VkImageView suitable"
                     " for use as a color, resolve, depth/stencil, or input"
                     " attachment.");
  }
  if (bits & vk::ImageUsageFlagBits::eInputAttachment) {
    result.push_back("Input Attachment: image can be used to create a"
                     " VkImageView suitable for occupying VkDescriptorSet"
                     " slot of type VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;"
                     " be read from a shader as an input attachment;"
                     " and be used as an input attachment in a framebuffer.");
  }
  if (bits & vk::ImageUsageFlagBits::eFragmentDensityMapEXT) {
    result.push_back("Fragment Density Map: image can be used to create a"
                     " VkImageView suitable for use as a fragment density"
                     " map image.");
  }
  if (bits & vk::ImageUsageFlagBits::eFragmentShadingRateAttachmentKHR) {
    result.push_back("Fragment Shading Rate Attachment: image can be used to"
                     " create a VkImageView suitable for use as a fragment"
                     " shading rate attachment or shading rate image");
  }

  return result;
}

inline std::string log_present_mode(vk::PresentModeKHR presentMode) {
  std::string result = "None/Undefined";

  if (presentMode == vk::PresentModeKHR::eImmediate) {
    result = "Imediate: the presentation engine does not wait for a vertical"
      " blanking period to update the current image, meaning this mode may"
      " result in visible tearing. No internal queuing of presentation requests"
      " is needed, as the requests are applied immediately.";
  }
  if (presentMode == vk::PresentModeKHR::eMailbox) {
    result = "Mailbox: the presentation engine waits for the next vertical"
      " blaking period to update the current image. Tearing cannot be observed."
      " An internal single-entry queue is used to hold pending presentation"
      " requests. If the queue is full when a new presentation request is"
      " received, the new request replaces the existing entry, and any images"
      " associated with the prior entry become available for re-use by the"
      " application. One request is removed from the queue and processed"
      " during each vertical blanking period in which the queue is non-empty.";
  }
  if (presentMode == vk::PresentModeKHR::eFifo) {
    result = "Fifo: the presentation engine waits for the next vertical"
      " blanking period to update the current image. Tearing cannot be"
      " observed. An internal queue is used to hold pending presenntation"
      " requests. New requests are appended to the end of the queue, and one"
      " request is removed from the beginning of the queue and processed during"
      " each vertical blanking period in which the queue is non-empty. This is"
      " the only value of PresentMode that is required to be supported.";
  }
  if (presentMode == vk::PresentModeKHR::eFifoRelaxed) {
    result = "Fifo Relaxed: The presentation engine generally waiths for the"
      " next vertical blanking period to update the current image. If a "
      " vertical blanking period has already passed since the last update of"
      " the current image then the presentation engine does not wait for"
      " another vertical blanking period for the update, meaning this mode may"
      " result in visible tearing in this case. This mode is useful for "
      " reducing visual stutter with an application that will mostly present"
      " a new image before the next vertical blanking period, but may"
      " occasionally be late, and present a new image just after the next"
      " vertical blanking period. An internal queue is used to hold pending"
      " presentation requests. New requests are appended to the end of the"
      " queue, and one request is removed from the beginning of the queue and"
      " processed during or after each vertical blanking period in which the"
      " queue is non-emtpy.";
  }
  if (presentMode == vk::PresentModeKHR::eSharedDemandRefresh) {
    result = "Shared Demand Refresh: the presentation engine and application"
      " have concurrency access to a single image, which is referred to as a"
      " shared presentable image. The pressentation engine is only required to"
      " update the current image after a new presentation request is received."
      " However, the presentation engine may update the current image at any"
      " point, meaning this mode may result in visible tearning.";
  }
  if (presentMode == vk::PresentModeKHR::eSharedContinuousRefresh) {
    result = "Shared Continuous Refresh: the pressentation engine and"
      " application have concurrent access to a single image, which is referred"
      " to as a shared presentable image. The application engine periodically"
      " updates the current image on its regular refresh cycle. The"
      " application is only required to make one initial pressentation request,"
      " after which the presentation engine must update the current image"
      " without any need for further presentation request. The application can"
      " indicate the image contents have been updated by making a presentation"
      " request, but this does not guarantee the timing of when it will be"
      " updated. This mode may result in visible tearing if rendering to the"
      " image is not timed correctly.";
  }

  return result;
}


}  // namespace vkInit

#endif  // INC_LOGGING_H_
