#pragma once
#include <cstdint>
#include <vector>

extern "C" {
  #include <libavcodec/avcodec.h>
  #include <libavutil/imgutils.h>
}

class VideoDecoder {
private:
  AVCodecContext* codec_ctx;
  AVFrame* frame;
  AVPacket* packet;

public:
  VideoDecoder();
  ~VideoDecoder();

  bool init();
  bool decode(const uint8_t* data, size_t size);
  void log_frame_info();

  int get_width() const { 
    return frame ? frame->width : 0; 
  }
  int get_height() const { 
    return frame ? frame->height : 0; 
  }
  const AVFrame* get_frame() const { 
    return frame; 
  }
};