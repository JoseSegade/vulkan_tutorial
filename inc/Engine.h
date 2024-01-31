// Copyright (c) 2024 Meerkat

#ifndef INC_ENGINE_H_
#define INC_ENGINE_H_

#include "Common.h"
#include <vector>

class Engine {
 public:
  void init();
  void destroy();

 private:
  void build_glfw_window();
  void make_instance();
  void make_device();

  bool                       mHasDebug       = true;

  uint32_t                   mWidth          = 800;
  uint32_t                   mHeight         = 600;

  GLFWwindow*                mWindow         = nullptr;

  vk::Instance               mInstance       = nullptr;
  vk::DebugUtilsMessengerEXT mDebugMessenger = nullptr;
  vk::DispatchLoaderDynamic  mDldi;
  vk::PhysicalDevice         mPhysicalDevice = nullptr;
  vk::Device                 mDevice         = nullptr;
  vk::Queue                  mGraphicsQueue  = nullptr;
  vk::Queue                  mPresentQueue   = nullptr;
  vk::SurfaceKHR             mSurface        = nullptr;
  vk::SwapchainKHR           mSwapchain;
  std::vector<vk::Image>     mSwapchainImages;
  vk::Format                 mSwapchainFormat;
  vk::Extent2D               mSwapchainExtent;
};

#endif  // INC_ENGINE_H_
