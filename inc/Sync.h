// Copyright (c) 2024 Meerkat
#ifndef INC_SYNC_H_
#define INC_SYNC_H_

#include "Common.h"

namespace vkInit {

inline vk::Semaphore make_semaphore(vk::Device device, bool debug) {
  vk::SemaphoreCreateInfo semaphoreInfo {};
  semaphoreInfo.flags = vk::SemaphoreCreateFlags();

  vk::Semaphore s {};
  try {
    s = device.createSemaphore(semaphoreInfo);
  } catch (vk::SystemError err) {
    if (debug) {
      printf("Failed to create semaphore. Error %s\n",
             err.what());
    }
  }

  return s;
}

inline vk::Fence make_fence(vk::Device device, bool debug) {
  vk::FenceCreateInfo fenceInfo {};
  fenceInfo.flags = vk::FenceCreateFlags() | vk::FenceCreateFlagBits::eSignaled;

  vk::Fence f{};
  try {
    f = device.createFence(fenceInfo);
  } catch (vk::SystemError err) {
    if (debug) {
      printf("Failed to create fence. Error %s\n",
             err.what());
    }
  }

  return f;
}

}  // namespace vkInit

#endif  // INC_SYNC_H_
