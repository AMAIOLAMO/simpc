#include <stdint.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include "../simp.c"

float timeSince(clock_t time) {
  return (float)(clock() - time) / CLOCKS_PER_SEC;
}

#define WIDTH 400
#define HEIGHT 400
uint32_t pixels[WIDTH*HEIGHT];

void rectangle_test(void) {
  simpcFillRect(pixels, WIDTH, HEIGHT, 0, 0, WIDTH, HEIGHT, 0xFFFFFFFF);
}

void fill_circle_test(void) {
  simpcFillCircle(pixels, WIDTH, HEIGHT, WIDTH/2, HEIGHT/2, WIDTH/2, 0xFFFFFFFF);
}

void fill_circle_ssaa_test(void) {
  simpcFillCircleSSAA(pixels, WIDTH, HEIGHT, WIDTH/2, HEIGHT/2, WIDTH/2, 0xFFFFFFFF);
}

void draw_arc_test(void) {
  simpcDrawArc(pixels, WIDTH, HEIGHT, WIDTH/2, HEIGHT/2, WIDTH/2, 10, 0, TAU, 0xFFFFFFFF);
}

void alpha_mix_test(void) {
  size_t index = rand() % WIDTH*HEIGHT;
  simpcAlphaMix(&pixels[index], 0xFFFFFFFF);
}

#define DEF_TEST(testFunc, tries) do { \
  printf(#testFunc" with %i tries", tries); \
  fflush(stdout); \
  clock_t time = clock(); \
  for (size_t try = 0; try < tries; try++) \
    testFunc(); \
  printf("\n\t-> total: %fs\n\t-> avrg: %fs\n\n", timeSince(time), timeSince(time) / tries); \
  } while(0)

int main(void)
{
  DEF_TEST(rectangle_test, 5000);
  DEF_TEST(fill_circle_test, 5000);
  DEF_TEST(fill_circle_ssaa_test, 5000);
  DEF_TEST(draw_arc_test, 5000);
  DEF_TEST(alpha_mix_test, 5000);
  return 0;
}
