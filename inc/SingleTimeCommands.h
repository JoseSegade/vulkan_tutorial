// Copyright (c) 2024 Meerkat
#ifndef INC_SINGLETIMECOMMANDS_H_
#define INC_SINGLETIMECOMMANDS_H_

#include "Common.h"

namespace vkUtil {

void start_job(vk::CommandBuffer commandBuffer);
void end_job(vk::CommandBuffer commandBuffer, vk::Queue queue);

}  // namespace vkUtil

#endif  // INC_SINGLETIMECOMMANDS_H_
