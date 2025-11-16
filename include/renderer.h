#pragma once
#include <SDL3/SDL.h>
#include "decoder.h"

class Renderer {
private:
  SDL_Window* window;
  SDL_Renderer* renderer;
  SDL_Texture* texture;
  int width, height;

public:
  Renderer();
  ~Renderer();
  bool init(int w, int h);
  void render(const AVFrame* frame);
  bool poll_event();
};