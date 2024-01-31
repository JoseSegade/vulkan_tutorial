// Copyright (c) 2024 Meerkat
#ifndef INC_INSTANCE_H_
#define INC_INSTANCE_H_

#include "Common.h"

namespace vkInit {

inline bool supported(const std::vector<const char*>& extensions,
               const std::vector<const char*>& layers,
               bool debug) {
  std::vector<vk::ExtensionProperties> supportedExtensions =
    vk::enumerateInstanceExtensionProperties();

  if (debug) {
    printf("Instance can support the following extensions:\n");
    for (const vk::ExtensionProperties& e : supportedExtensions) {
      printf("\t%s\n", e.extensionName.data());
    }
  }

  for (const char* extension : extensions) {
    bool found = false;
    for (const vk::ExtensionProperties& e : supportedExtensions) {
      if (strcmp(extension, e.extensionName) == 0) {
        found = true;
        if (debug) {
          printf("Extension \"%s\" is supported\n", extension);
        }
        break;
      }
    }
    if (!found) {
      if (debug) {
        printf("Extension \"%s\" not supported\n", extension);
      }
      return false;
    }
  }

  std::vector<vk::LayerProperties> supportedLayers =
    vk::enumerateInstanceLayerProperties();

  if (debug) {
    printf("Instance can support the following layers:\n");
    for (const vk::LayerProperties& l : supportedLayers) {
      printf("\t%s\n", l.layerName.data());
    }
  }

  for (const char* layer : layers) {
    bool found = false;
    for (const vk::LayerProperties& l : supportedLayers) {
      if (strcmp(layer, l.layerName) == 0) {
        found = true;
        if (debug) {
          printf("Layer \"%s\" is supported\n", layer);
        }
        break;
      }
    }
    if (!found) {
      if (debug) {
        printf("Layer \"%s\" not supported\n", layer);
      }
      return false;
    }
  }

  return true;
}

inline vk::Instance make_instance(bool debug, const char* appname) {
  if (debug) {
    printf("Creating instance...\n");
  }
  uint32_t version = 0;
  vkEnumerateInstanceVersion(&version);

  if (debug) {
    printf("System can support vulkan variant: %d, "
           "Version: %d.%d.%d\n",
           VK_API_VERSION_VARIANT(version),
           VK_API_VERSION_MAJOR(version),
           VK_API_VERSION_MINOR(version),
           VK_API_VERSION_PATCH(version));
  }

  version = VK_MAKE_API_VERSION(0, 1, 0, 0);

  vk::ApplicationInfo appInfo = vk::ApplicationInfo(
    appname,
    version,
    "Doing it the hard way",
    version,
    version);

  uint32_t glfwExtensionCount = 0;
  const char** glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector<const char*> extensions(glfwExtensions,
                                      glfwExtensions + glfwExtensionCount);

  if (debug) {
    extensions.push_back("VK_EXT_debug_utils");
  }

  if (debug) {
    printf("Instance extensions to be requested:\n");
    for (const char* extensionName : extensions) {
      printf("\t%s\n", extensionName);
    }
  }

  std::vector<const char*> layers {};
  if (debug) {
    layers.push_back("VK_LAYER_KHRONOS_validation");
  }

  if (debug) {
    printf("Instance layers to be requested:\n");
    for (const char* layerName : layers) {
      printf("\t%s\n", layerName);
    }
  }

  if (!supported(extensions, layers, debug)) {
    return nullptr;
  }

  vk::InstanceCreateInfo createInfo = vk::InstanceCreateInfo(
    vk::InstanceCreateFlags(),
    &appInfo,
    static_cast<uint32_t>(layers.size()),
    layers.data(),
    static_cast<uint32_t>(extensions.size()),
    extensions.data());

  vk::Instance result = nullptr;
  try {
    result = vk::createInstance(createInfo);
  } catch (vk::SystemError error) {
    if (debug) {
      printf("Failed to create instance\n"
             "%s\n", error.what());
    }
  }

  return result;
}

}  // namespace vkInit

#endif  // INC_INSTANCE_H_
