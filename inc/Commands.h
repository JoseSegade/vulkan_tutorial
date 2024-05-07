// Copyright (c) 2024 Meerkat
#ifndef INC_COMMANDS_H_
#define INC_COMMANDS_H_

#include "Common.h"
#include "Frame.h"
#include "QueueFamilies.h"
#include <vector>

namespace vkInit {

struct CommandBufferInputChunk {
  vk::Device device;
  vk::CommandPool commandPool;
  std::vector<vkUtil::SwapChainFrame>* frames;
};

inline vk::CommandPool make_command_pool(
  vk::Device device,
  vk::PhysicalDevice physicalDevice,
  vk::SurfaceKHR surface,
  bool debug) {
  vkUtil::QueueFamilyIndices queueFamilyIndices =
    vkUtil::findQueueFamilies(physicalDevice, surface, debug);

  vk::CommandPoolCreateInfo poolInfo {};
  poolInfo.flags = vk::CommandPoolCreateFlags()
    | vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
  poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

  vk::CommandPool cp {};
  try {
    cp = device.createCommandPool(poolInfo);
  } catch (vk::SystemError err) {
    if (debug) {
      printf("Error while creating command pool. Error: %s\n",
             err.what());
    }
  }

  return cp;
}

inline void make_frame_command_buffers(
  CommandBufferInputChunk* input,
  bool debug) {
  vk::CommandBufferAllocateInfo allocInfo {};
  allocInfo.commandPool        = input->commandPool;
  allocInfo.level              = vk::CommandBufferLevel::ePrimary;
  allocInfo.commandBufferCount = 1;

  for (size_t i = 0; i < input->frames->size(); ++i) {
    try {
      input->frames->at(i).mCommandBuffer =
        input->device.allocateCommandBuffers(allocInfo)[0];
    } catch (vk::SystemError err) {
      printf("Error while creating command buffer for frame %lu. Error%s\n",
             i, err.what());
    }
  }
}

inline vk::CommandBuffer make_command_buffer(
  CommandBufferInputChunk* input,
  bool debug) {
  vk::CommandBufferAllocateInfo allocInfo {};
  allocInfo.commandPool        = input->commandPool;
  allocInfo.level              = vk::CommandBufferLevel::ePrimary;
  allocInfo.commandBufferCount = 1;

  vk::CommandBuffer mcb {};
  try {
     mcb =
      input->device.allocateCommandBuffers(allocInfo)[0];
  } catch (vk::SystemError err) {
    printf("Error while creating main command buffer. Error %s\n",
           err.what());
  }

  return mcb;
}

}  // namespace vkInit

#endif  // INC_COMMANDS_H_
