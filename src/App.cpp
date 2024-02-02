// Copyright (c) 2024 Meerkat

#include "../inc/App.h"
#include <algorithm>
#include <sstream>

App::App(uint32_t width, uint32_t height, bool debug) {
  build_glfw_window(width, height, debug);
  mGraphicsEngine = new Engine();
  mGraphicsEngine->init(width, height, mWindow, debug);
  mScene = new Scene();
  mScene->Init();
}

App::~App() {
  delete mScene;
  mGraphicsEngine->destroy();
  delete mGraphicsEngine;
  glfwTerminate();
}

void App::build_glfw_window(uint32_t width, uint32_t height, bool debug) {
  glfwInit();

  // No api, vulkan will be linked later
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  mWindow = glfwCreateWindow(width, height, "Engine", nullptr, nullptr);

  if (mWindow) {
    if (debug) {
      printf("Succesfully created glfw window\n");
    }
  } else {
    if (debug) {
      printf("Glfw window creation failed\n");
    }
  }
}

void App::run() {
  while (!glfwWindowShouldClose(mWindow)) {
    glfwPollEvents();
    mGraphicsEngine->render(mScene);
    calculate_frame_rate();
  }
}

void App::calculate_frame_rate() {
  mCurrentTime = glfwGetTime();
  double delta = mCurrentTime - mLastTime;

  if (delta >= 1.0) {
    int32_t framerate = std::max(1, static_cast<int32_t>(mNumFrames / delta));
    std::stringstream title;
    title << "Running at " << framerate << " fps.";
    glfwSetWindowTitle(mWindow, title.str().c_str());
    mLastTime = mCurrentTime;
    mNumFrames = -1;
    mFrameTime = static_cast<float>(1000.0 / framerate);
  }

  ++mNumFrames;
}
