#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <math.h>
#include <sys/types.h>
#include "../../simp.c"

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
  (void) totalFrames;
  float t = simpcGetAnimationTime(frameIndex, fps);
  float percent = (float)frameIndex / totalFrames;

  float radius = 50 * (1.0f - percent);
  float rotationRadius = 90 * (1.0f - percent);

  simpcFill(pixels, width, height, 0xFF121212);
  simpcFillCircle(pixels, width, height, (float)width/2 + sinf(t*5) * rotationRadius, (float)height/2 + cosf(t*5) * rotationRadius, radius, 0xFFFF0000);
}
