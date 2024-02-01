// Copyright (c) 2024 Meerkat
#ifndef INC_APP_H_
#define INC_APP_H_

#include "Common.h"
#include "Engine.h"

class App {
 public:
  App(uint32_t width, uint32_t height, bool debugMode);
  ~App();
  void run();

 private:
  void build_glfw_window(uint32_t width, uint32_t height, bool debugMode);
  void calculate_frame_rate();

  Engine*     mGraphicsEngine = nullptr;
  GLFWwindow* mWindow         = nullptr;

  double      mLastTime    = 0.0;
  double      mCurrentTime = 0.0;
  int32_t     mNumFrames   = 0;
  float       mFrameTime   = 0.0f;
};

#endif  // INC_APP_H_
