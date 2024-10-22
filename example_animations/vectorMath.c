#include <stdio.h>

#include "../simp.c"

size_t getFps() {
  return 24;
}

size_t getTotalFrames() {
  return getFps() * 10;
}

size_t getWidth() {
  return 420;
}

size_t getHeight() {
  return 420;
}
void updateFrame(uint32_t *pixels, size_t width, size_t height, size_t frameIndex, size_t totalFrames, size_t fps) {
  float t = simpcGetAnimationTime(frameIndex, fps);

  simpcFill(pixels, width, height, 0xFF121212);

  size_t cx = width/2;
  size_t cy = height/2;

  size_t radius = 50;
  float lx = cosf(t*3) * radius;
  float ly = sinf(t*3) * radius;

  simpcDrawArc(pixels, width, height, cx, cy, radius, 4, 0, TAU, 0xFFAA0000);

  simpcDrawLine(pixels, width, height, cx, cy, cx + lx, cy + ly, 0xFF00FF00);


  simpcDrawLine(pixels, width, height, cx, cy, cx + 2 * cosf(t*3) * radius, cy, 0x3300FF00);

  simpcDrawLine(pixels, width, height, cx, cy, cx, cy + 2 * sinf(t*3) * radius, 0x3300FF00);


  simpcDrawLine(pixels, width, height, cx + lx, cy + ly, cx, cy + 2 * sinf(t*3) * radius, 0xFF00FF00);

  simpcDrawLine(pixels, width, height, cx + lx, cy + ly, cx + 2 * cosf(t*3) * radius, cy, 0xFF00FF00);
}

