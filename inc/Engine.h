// Copyright (c) 2024 Meerkat

#ifndef INC_ENGINE_H_
#define INC_ENGINE_H_

#include "Common.h"
#include "Frame.h"
#include "Scene.h"
#include <vector>

class Engine {
 public:
  void init(
    uint32_t width, uint32_t height, GLFWwindow* window, bool debugMode);
  void destroy();
  void render(Scene* scene);

 private:
  void make_instance();
  void make_device();
  void make_swapchain();
  void recreate_swapchain();
  void cleanup_swapchain();
  void make_pipeline();
  void finalize_setup();
  void make_framebuffers();
  void make_frame_sync_objects();
  void record_draw_commands(vk::CommandBuffer commandBuffer,
                            uint32_t imageIndex, Scene* scene);

  bool                       mHasDebug       = false;

  uint32_t                   mWidth          = 1;
  uint32_t                   mHeight         = 1;

  GLFWwindow*                mWindow         = nullptr;

  vk::Instance               mInstance       = nullptr;
  vk::DebugUtilsMessengerEXT mDebugMessenger = nullptr;
  vk::DispatchLoaderDynamic  mDldi;
  vk::PhysicalDevice         mPhysicalDevice = nullptr;
  vk::Device                 mDevice         = nullptr;
  vk::Queue                  mGraphicsQueue  = nullptr;
  vk::Queue                  mPresentQueue   = nullptr;
  vk::SurfaceKHR             mSurface        = nullptr;

  vk::SwapchainKHR                    mSwapchain;
  std::vector<vkUtil::SwapChainFrame> mSwapchainFrames;
  vk::Format                          mSwapchainFormat;
  vk::Extent2D                        mSwapchainExtent;

  vk::PipelineLayout                  mPipelineLayout;
  vk::RenderPass                      mRenderPass;
  vk::Pipeline                        mGraphicsPipeline;

  vk::CommandPool                     mCommandPool;
  vk::CommandBuffer                   mMainCommandBuffer;

  uint32_t                            mMaxFramesInFlight;
  uint32_t                            mFrameNumber;
};

#endif  // INC_ENGINE_H_
