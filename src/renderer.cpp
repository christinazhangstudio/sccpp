#include "renderer.h"
#include "logger.h"

// with extern "C", C++ knows these functions come from
// C code. this way, the compiler knows to reference
// and unmangled C symbol and not a mangled C++ symbol.
extern "C" {
  #include <libswscale/swscale.h>
}

extern Logger* g_log; // init in main

Renderer::Renderer() : window(nullptr), renderer(nullptr), texture(nullptr), width(0), height(0) {}

bool Renderer::init(int w, int h) {
  if(!SDL_Init(SDL_INIT_VIDEO)) {
    return false;
  }

  // SCALE TO FIT MONITOR
  // 544 * 1104 is the minimum!!
  const int MAX_WIDTH = 544;
  const int MAX_HEIGHT = 1104;
  float scale = fminf((float)MAX_WIDTH / w, (float)MAX_HEIGHT / h);
  scale = fminf(scale, 1.0f);

  int display_w = (int)(w * scale);
  int display_h = (int)(h * scale);

  width = w; 
  height = h;

  window = SDL_CreateWindow("sccpp", display_w, display_h, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
  renderer = SDL_CreateRenderer(window, nullptr);
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV,
                              SDL_TEXTUREACCESS_STREAMING, w, h);

  if(g_log) g_log->info("attempted to create window and texture");
  return window && renderer && texture;
}

void Renderer::render(const AVFrame* frame) {
  if(!frame) {
    return;
  }
  
  if(!texture || frame->width != width || frame->height != height) {
    // if(g_log) g_log->error("recreating window and texture");
    // if (texture) { 
    //   SDL_DestroyTexture(texture); 
    //   texture = nullptr; 
    // }
    if(g_log) g_log->error("failed to init window or texture");
  }

  // uint8_t* dst_pixels = nullptr;
  int dst_pitch = 0;
  // if(SDL_LockTexture(texture, nullptr, (void**)&dst_pixels, &dst_pitch) != 0) {
  //   if(g_log) g_log->error("failed to lock texture: %s", SDL_GetError());
  //   return;
  // }
  SDL_UpdateYUVTexture(texture, NULL,
    frame->data[0], frame->linesize[0],
    frame->data[1], frame->linesize[1],
    frame->data[2], frame->linesize[2]);

  if(g_log) g_log->info("Frame: %dx%d, pixfmt=%s (%d)", frame->width, frame->height,
           av_get_pix_fmt_name((AVPixelFormat)frame->format), frame->format);
  if(g_log) g_log->info("SDL pitch = %d, expected width*4 = %d", dst_pitch, frame->width * 4);

  // sws_scale is not needed with SDL_UpdateYUVTexture()...
  // SwsContext* sws = sws_getContext(
  //   frame->width,
  //   frame->height,
  //   (AVPixelFormat)frame->format,
  //   width, height, AV_PIX_FMT_BGRA,
  //   SWS_BILINEAR, nullptr, nullptr, nullptr);

  // // source plane pointers/linesizes from the decoded frame
  // uint8_t* src_data[4] = {
  //   frame->data[0], 
  //   frame->data[1], 
  //   frame->data[2],
  //   frame->data[3]
  // };

  // int src_linesize[4] = {
  //   frame->linesize[0],
  //   frame->linesize[1],
  //   frame->linesize[2],
  //   frame->linesize[3]
  // };

  // // dst arrays for sws scale (one plane)
  // uint8_t* dst_data[1]     = { dst_pixels };
  // int      dst_linesize[1] = { dst_pitch };

  // // scale/convert
  // sws_scale(sws, src_data, src_linesize, 0, frame->height,
  //           dst_data, dst_linesize);

  // sws_freeContext(sws);
  // SDL_UnlockTexture(texture);

  // render texture
  SDL_RenderClear(renderer);

  int ww, wh;
  SDL_GetWindowSize(window, &ww, &wh);
  SDL_FRect dst = {0, 0, (float)ww, (float)wh};
  SDL_RenderTexture(renderer, texture, nullptr, &dst);

  SDL_RenderPresent(renderer);

}

bool Renderer::poll_event() {
  SDL_Event e;
  while(SDL_PollEvent(&e)) {
    if(e.type == SDL_EVENT_QUIT) {
      return false;
    }
  }
  return true;
}

Renderer::~Renderer() {
  if(texture) SDL_DestroyTexture(texture);
  if(renderer) SDL_DestroyRenderer(renderer);
  if(window) SDL_DestroyWindow(window);
  SDL_Quit();
}