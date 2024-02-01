// Copyright (c) 2024 Meerkat

#include "../inc/App.h"

int main() {
  App* app = new App(800, 800, true);
  app->run();
  delete app;

  return 0;
}
