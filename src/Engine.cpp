// Copyright (c) 2024 Meerkat
#include "../inc/Engine.h"
#include "../inc/Instance.h"
#include "../inc/Logging.h"
#include "../inc/Device.h"

void Engine::init() {
  if (mHasDebug) {
    printf("Initializing app...\n");
  }

  build_glfw_window();
  make_instance();
  make_device();

  if (mHasDebug) {
    printf("Initializing app done.\n");
  }
}

void Engine::destroy() {
  if (mHasDebug) {
    printf("Destroying app...\n");
  }

  mDevice.destroySwapchainKHR(mSwapchain);
  mDevice.destroy();
  mInstance.destroySurfaceKHR(mSurface);

  if (mHasDebug) {
    mInstance.destroyDebugUtilsMessengerEXT(mDebugMessenger, nullptr,
                                            mDldi);
  }

  mInstance.destroy();
  glfwTerminate();

  if (mHasDebug) {
    printf("Destroying done.\n");
  }
}

void Engine::build_glfw_window() {
  glfwInit();

  // No api, vulkan will be linked later
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  mWindow = glfwCreateWindow(mWidth, mHeight, "Engine", nullptr, nullptr);

  if (mWindow) {
    if (mHasDebug) {
      printf("Succesfully created glfw window\n");
    }
  } else {
    if (mHasDebug) {
      printf("Glfw window creation failed\n");
    }
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
  mSwapchainImages = bundle.images;
  mSwapchainFormat = bundle.format;
  mSwapchainExtent = bundle.extent;
}
