// Copyright (c) 2024 Meerkat
#include "../inc/Engine.h"
#include "../inc/Instance.h"
#include "../inc/Logging.h"
#include "../inc/Device.h"
#include "../inc/Swapchain.h"
#include "../inc/Pipeline.h"
#include "../inc/Framebuffer.h"
#include "../inc/Commands.h"
#include "../inc/Sync.h"

void Engine::init(
  uint32_t width, uint32_t height, GLFWwindow* window, bool debugMode) {
  mWidth    = width;
  mHeight   = height;
  mWindow   = window;
  mHasDebug = debugMode;

  if (mHasDebug) {
    printf("Initializing app...\n");
  }

  make_instance();
  make_device();
  make_pipeline();
  finalize_setup();

  if (mHasDebug) {
    printf("Initializing app done.\n");
  }
}

void Engine::destroy() {
  mDevice.waitIdle();

  if (mHasDebug) {
    printf("Destroying app...\n");
  }

  mDevice.destroyFence(mInFlightFence);
  mDevice.destroySemaphore(mImageAvailable);
  mDevice.destroySemaphore(mRenderFinished);

  mDevice.destroyCommandPool(mCommandPool);

  mDevice.destroyPipeline(mGraphicsPipeline);
  mDevice.destroyPipelineLayout(mPipelineLayout);
  mDevice.destroyRenderPass(mRenderPass);

  for (const vkUtil::SwapChainFrame& f : mSwapchainFrames) {
    mDevice.destroyImageView(f.imageView);
    mDevice.destroyFramebuffer(f.framebuffer);
  }
  mDevice.destroySwapchainKHR(mSwapchain);
  mDevice.destroy();
  mInstance.destroySurfaceKHR(mSurface);

  if (mHasDebug) {
    mInstance.destroyDebugUtilsMessengerEXT(mDebugMessenger, nullptr,
                                            mDldi);
  }

  mInstance.destroy();

  if (mHasDebug) {
    printf("Destroying done.\n");
  }
}

void Engine::make_instance() {
  mInstance = vkInit::make_instance(mHasDebug, "Engine");
  mDldi     = vk::DispatchLoaderDynamic(mInstance, vkGetInstanceProcAddr);
  if (mHasDebug) {
    mDebugMessenger = vkInit::make_debug_messenger(mInstance, mDldi);
  }

  VkSurfaceKHR surface;
  VkResult createSurfaceResult =
    glfwCreateWindowSurface(mInstance, mWindow, nullptr, &surface);
  if (createSurfaceResult != VK_SUCCESS) {
    if (mHasDebug) {
      printf("Failed to create the glfw surface for Vulkan.\n");
    }
  } else {
    if (mHasDebug) {
      printf("Successfully creasted glfw surface for Vulkan.\n");
    }
  }
  mSurface = surface;
}

void Engine::make_device() {
  mPhysicalDevice = vkInit::choose_physical_device(mInstance, mHasDebug);
  mDevice         =
    vkInit::create_logical_device(mPhysicalDevice, mSurface, mHasDebug);
  std::array<vk::Queue, 2> queues =
    vkInit::get_queue(mPhysicalDevice, mDevice, mSurface, mHasDebug);
  mGraphicsQueue = queues[0];
  mPresentQueue  = queues[1];
  vkInit::SwapChainBundle bundle = vkInit::create_swapchain(
    mPhysicalDevice, mDevice, mSurface, mWidth, mHeight, mHasDebug);

  mSwapchain       = bundle.swapchain;
  mSwapchainFrames = bundle.frames;
  mSwapchainFormat = bundle.format;
  mSwapchainExtent = bundle.extent;
}

void Engine::make_pipeline() {
  vkInit::GraphicsPipelineInBundle specification {};
  specification.device           = mDevice;
  specification.vertexFilepath   = "./bin/shaders/default.vert.spv";
  specification.fragmenFilepath  = "./bin/shaders/default.frag.spv";
  specification.extent           = mSwapchainExtent;
  specification.swapchainFormat  = mSwapchainFormat;

  vkInit::GraphicsPipelineOutBundle output =
    vkInit::make_graphics_pipeline(specification, mHasDebug);

  mPipelineLayout   = output.pipelineLayout;
  mRenderPass       = output.renderPass;
  mGraphicsPipeline = output.graphicsPipeline;
}

void Engine::finalize_setup() {
  vkInit::FramebufferInput framebufferInput{};
  framebufferInput.device          = mDevice;
  framebufferInput.renderPass      = mRenderPass;
  framebufferInput.swapchainExtent = mSwapchainExtent;

  vkInit::make_framebuffers(framebufferInput, &mSwapchainFrames, mHasDebug);

  mCommandPool =
    vkInit::make_command_pool(mDevice, mPhysicalDevice, mSurface, mHasDebug);

  vkInit::CommandBufferInputChunk commandBufferInput{};
  commandBufferInput.device      = mDevice;
  commandBufferInput.frames      = &mSwapchainFrames;
  commandBufferInput.commandPool = mCommandPool;

  mMainCommandBuffer =
    vkInit::make_command_buffers(&commandBufferInput, mHasDebug);

  mInFlightFence = vkInit::make_fence(mDevice, mHasDebug);
  mImageAvailable = vkInit::make_semaphore(mDevice, mHasDebug);
  mRenderFinished = vkInit::make_semaphore(mDevice, mHasDebug);
}

void Engine::record_draw_commands(vk::CommandBuffer commandBuffer,
                                  uint32_t imageIndex) {
  vk::CommandBufferBeginInfo beginInfo {};
  try {
    commandBuffer.begin(beginInfo);
  } catch (vk::SystemError err) {
    if (mHasDebug) {
      printf("Failed to begin recording command buffer. Error: %s\n",
             err.what());
    }
  }

  vk::ClearValue clearColor  =
    { { 1.0f, 0.5f, 0.25f, 1.0f } };
  vk::RenderPassBeginInfo renderPassInfo {};
  renderPassInfo.renderPass = mRenderPass;
  renderPassInfo.framebuffer = mSwapchainFrames[imageIndex].framebuffer;
  renderPassInfo.renderArea.offset.x = 0;
  renderPassInfo.renderArea.offset.y = 0;
  renderPassInfo.renderArea.extent = mSwapchainExtent;
  renderPassInfo.clearValueCount = 1;
  renderPassInfo.pClearValues = &clearColor;

  commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

  commandBuffer.bindPipeline(
    vk::PipelineBindPoint::eGraphics, mGraphicsPipeline);

  commandBuffer.draw(3, 1, 0, 0);

  commandBuffer.endRenderPass();

  try {
    commandBuffer.end();
  } catch (vk::SystemError err) {
    if (mHasDebug) {
      printf("Failed to finish recording command buffer. Error: %s\n",
             err.what());
    }
  }
}

void Engine::render() {
  mDevice.waitForFences(1, &mInFlightFence, VK_TRUE, UINT64_MAX);
  mDevice.resetFences(1, &mInFlightFence);

  uint32_t imageIndex =
    mDevice.acquireNextImageKHR(
      mSwapchain, UINT64_MAX, mImageAvailable, nullptr).value;

  vk::CommandBuffer commandBuffer = mSwapchainFrames[imageIndex].commandBuffer;
  commandBuffer.reset();

  record_draw_commands(commandBuffer, imageIndex);

  vk::Semaphore waitSemaphores[] = { mImageAvailable };
  vk::Semaphore signalSemaphores[] = { mRenderFinished };
  vk::PipelineStageFlags waitStages[] =
    { vk::PipelineStageFlagBits::eColorAttachmentOutput };
  vk::SubmitInfo submitInfo {};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  try {
    mGraphicsQueue.submit(submitInfo, mInFlightFence);
  } catch (vk::SystemError err) {
    if (mHasDebug) {
      printf("Failed to submit draw command buffer. Error: %s\n",
             err.what());
    }
  }

  vk::SwapchainKHR swapchains[] = { mSwapchain };
  vk::PresentInfoKHR presentInfo {};
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapchains;
  presentInfo.pImageIndices = &imageIndex;

  mPresentQueue.presentKHR(presentInfo);
}
