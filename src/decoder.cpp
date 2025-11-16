#include "decoder.h"
#include "logger.h"
#include <cstdio>

extern Logger* g_log; // init in main

VideoDecoder::VideoDecoder() : codec_ctx(nullptr), frame(nullptr), packet(nullptr) {}

bool VideoDecoder::init() {
  const AVCodec* codec = avcodec_find_decoder(AV_CODEC_ID_H264);
  if(!codec) {
    if(g_log) g_log->error("H.264 not found");
    return false;
  }

  codec_ctx = avcodec_alloc_context3(codec);
  if(!codec_ctx) {
    if(g_log) g_log->error("failed to alloc decoder context");
    return false;
  }

  codec_ctx->flags2 |= AV_CODEC_FLAG2_CHUNKS;
  if(avcodec_open2(codec_ctx, codec, nullptr) < 0) {
    if(g_log) g_log->error("failed to open decoder");
  }

  frame = av_frame_alloc();
  packet = av_packet_alloc();
  if(!frame || !packet) {
    if(g_log) g_log->error("failed to alloc frame or packet");
    return false;
  }

  if(g_log) g_log->info("initialized ffmpeg decoder");
  return true;
}

bool VideoDecoder::decode(const uint8_t* data, size_t size) {
  packet->data = (uint8_t*)data;
  packet->size = size;

  int ret = avcodec_send_packet(codec_ctx, packet);
  if(ret < 0) {
    if(g_log) g_log->error("failed to send packet");
  }

  while(ret >= 0) {
    ret = avcodec_receive_frame(codec_ctx, frame);
    if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
      break;
    } else if (ret < 0) {
      if(g_log) g_log->error("failed to receive frame");
    }

    log_frame_info();
    return true;
  }

  return false;
}

void VideoDecoder::log_frame_info() {
  if(!g_log || !frame) {
    return;
  }
  g_log->info("DECODED FRAME: %dx%d, format=%d",
    frame->width,
    frame->height,
    frame->format
  );
}

VideoDecoder::~VideoDecoder() {
  if(packet) av_packet_free(&packet);
  if(frame) av_frame_free(&frame);
  if(codec_ctx) avcodec_free_context(&codec_ctx);
}
