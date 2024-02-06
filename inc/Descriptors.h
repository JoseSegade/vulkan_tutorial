// Copyright (c) 2024 Meerkat
#ifndef INC_DESCRIPTORS_H_
#define INC_DESCRIPTORS_H_

#include "Common.h"
#include <vector>

namespace vkInit {

struct DescriptorSetLayoutData {
  uint32_t                          count;
  std::vector<uint32_t>             indices;
  std::vector<vk::DescriptorType>   types;
  std::vector<uint32_t>             counts;
  std::vector<vk::ShaderStageFlags> stages;
};

inline vk::DescriptorSetLayout make_descriptor_set_layout(
  vk::Device device, const DescriptorSetLayoutData& bindings,
  bool debug) {
  std::vector<vk::DescriptorSetLayoutBinding> layoutBindings{};
  layoutBindings.reserve(bindings.count);

  for (uint32_t i = 0; i < bindings.count; ++i) {
    vk::DescriptorSetLayoutBinding layoutBinding {};
    layoutBinding.binding         = bindings.indices[i];
    layoutBinding.descriptorType  = bindings.types[i];
    layoutBinding.descriptorCount = bindings.counts[i];
    layoutBinding.stageFlags      = bindings.stages[i];

    layoutBindings.push_back(layoutBinding);
  }

  vk::DescriptorSetLayoutCreateInfo layoutInfo {};
  layoutInfo.flags        = vk::DescriptorSetLayoutCreateFlagBits();
  layoutInfo.bindingCount = bindings.count;
  layoutInfo.pBindings    = layoutBindings.data();

  vk::DescriptorSetLayout res {};
  try {
    res = device.createDescriptorSetLayout(layoutInfo);
  } catch (vk::SystemError err) {
    if (debug) {
      printf("Error while creating descriptor set layout. Error %s\n",
             err.what());
    }
  }

  return res;
}

inline vk::DescriptorPool make_descriptor_pool(
  vk::Device device,
  uint32_t size,
  const DescriptorSetLayoutData& bindings,
  bool debug) {
  std::vector<vk::DescriptorPoolSize> poolSizes {};

  for (uint32_t i = 0; i < bindings.count; ++i) {
    vk::DescriptorPoolSize poolSize {};
    poolSize.type            = bindings.types[i];
    poolSize.descriptorCount = size;

    poolSizes.push_back(poolSize);
  }

  vk::DescriptorPoolCreateInfo poolInfo {};
  poolInfo.flags         = vk::DescriptorPoolCreateFlags();
  poolInfo.maxSets       = size;
  poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  poolInfo.pPoolSizes    = poolSizes.data();

  vk::DescriptorPool res {};
  try {
    res = device.createDescriptorPool(poolInfo);
  } catch (vk::SystemError err) {
    if (debug) {
      printf("Error while creating descriptor pool. Error: %s\n",
             err.what());
    }
  }

  return res;
}

inline vk::DescriptorSet allocate_descriptor_set(
  vk::Device device,
  vk::DescriptorPool descriptorPool,
  vk::DescriptorSetLayout layout,
  bool debug) {
  vk::DescriptorSetAllocateInfo allocationInfo {};
  allocationInfo.descriptorPool = descriptorPool;
  allocationInfo.descriptorSetCount = 1;
  allocationInfo.pSetLayouts = &layout;

  vk::DescriptorSet res {};
  try {
    res = device.allocateDescriptorSets(allocationInfo)[0];
  } catch (vk::SystemError err) {
    if (debug) {
      printf("Error while creating descriptor set. Error: %s\n",
             err.what());
    }
  }

  return res;
}

}  // namespace vkInit

#endif  // INC_DESCRIPTORS_H_
