#include "simp.c"
#include <time.h>
#include <dirent.h>

#define WIDTH 1000
#define HEIGHT 800

uint32_t pixels[WIDTH*HEIGHT];


bool trySaveToPPM(const char* filePath) {
  Errno err = simpcSavePPM(pixels, WIDTH, HEIGHT, filePath);

  if(err) {
    fprintf(stderr, "ERROR: %s", strerror(err));
    return false;
  }

  return true;
}


void bezier(void) {
  srand(time(NULL));

  uint32_t color = 0xFF121212;
  simpcFill(pixels, WIDTH, HEIGHT, color);

  for (size_t i = 0; i < 500; i++) {
    int x1 = rand() % WIDTH;
    int x2 = rand() % WIDTH;
    int x3 = rand() % WIDTH;
    int x4 = rand() % WIDTH;

    int y1 = rand() % WIDTH;
    int y2 = rand() % WIDTH;
    int y3 = rand() % WIDTH;
    int y4 = rand() % WIDTH;

    // simpcFillCircle(pixels, WIDTH, HEIGHT, x1, y1, 5, 0xFF0000FF);
    // simpcFillCircle(pixels, WIDTH, HEIGHT, x2, y2, 5, 0xFF0000FF);
    // simpcFillCircle(pixels, WIDTH, HEIGHT, x3, y3, 5, 0xFF0000FF);
    // simpcFillCircle(pixels, WIDTH, HEIGHT, x4, y4, 5, 0xFF0000FF);

    simpcDrawBezier(pixels, WIDTH, HEIGHT, x1, y1, x2, y2, x3, y3, x4, y4, 0xFF00FF00, 20);

  }
}


void lines(void) {
  srand(time(NULL));
  
  uint32_t color = 0xFF121212;
  simpcFill(pixels, WIDTH, HEIGHT, color);

  const size_t COUNT = 200;

  for (size_t i = 0; i < COUNT; i++) {
    float angle = PI / COUNT * i;

    float radius = 100.0f + (double)rand() / RAND_MAX * 300.0f;

    float x = cosf(angle) * radius;
    float y = sinf(angle) * radius;

    uint8_t r = (double)rand() / RAND_MAX * 255;
    uint8_t g = (double)rand() / RAND_MAX * 255;
    uint8_t b = (double)rand() / RAND_MAX * 255;

    simpcDrawLine(pixels, WIDTH, HEIGHT, WIDTH/2 + (int)x, HEIGHT/2 + (int)y, WIDTH/2 - (int)x, HEIGHT/2 - (int)y, simpcColorFromRGB(r, g, b));
  }
}


void allShapes(void) {
  uint32_t color = 0xFF121212;
  simpcFill(pixels, WIDTH, HEIGHT, color);

  simpcFillRect(pixels, WIDTH, HEIGHT, 100, 100, 70, 50, 0xFF00FF00);

  simpcFillTriangle(pixels, WIDTH, HEIGHT, 200, 200, 415, 12, 94, 425, 0xFFFF0000);
  simpcFillTriangle(pixels, WIDTH, HEIGHT, 300, 300, 415, 12, 94, 425, 0xFF00FF00);

  simpcFillCircle(pixels, WIDTH, HEIGHT, WIDTH/2-100, HEIGHT/2, 50, 0xFFFF00FF);

  simpcFillCircleSSAA(pixels, WIDTH, HEIGHT, WIDTH/2-100, HEIGHT/2+100, 50, 0xFFFF00FF);

  simpcFillCircleSSAA(pixels, WIDTH, HEIGHT, WIDTH/2-100, HEIGHT/2+150, 50, 0x55FF8800);

  simpcFillSector(pixels, WIDTH, HEIGHT, WIDTH/2, HEIGHT/2, 50, TAU/4*3, TAU, simpcColorFromRGB(0, 100, 200));
  
  simpcFillSector(pixels, WIDTH, HEIGHT, WIDTH/2, HEIGHT/2, 50, 0, TAU/4*3, simpcColorFromRGB(0, 100, 0));

  simpcDrawArc(pixels, WIDTH, HEIGHT, WIDTH/2+100, HEIGHT/2, 45, 5, TAU/12, TAU/4*3, 0xFFAA00FF);

  simpcDrawLine(pixels, WIDTH, HEIGHT, 600, 700, 700, 1000, 0xFFFFFFFF);
}

void checker(void) {
  uint32_t color = 0xFF121212;
  simpcFill(pixels, WIDTH, HEIGHT, color);

  const size_t ROWS = 8;
  const size_t COLS = 10;

  const size_t CELL_WIDTH = WIDTH / COLS;
  const size_t CELL_HEIGHT = HEIGHT / ROWS;

  const size_t PADDING = 5;

  for (size_t row = 0; row < ROWS; row++) {
    for (size_t col = 0; col < COLS; col++) {
      if((row + col) % 2 == 0) {
        uint32_t cellColor = simpcColorFromRGB(255, 0, 0);

        simpcFillRect(pixels, WIDTH, HEIGHT, col*CELL_WIDTH + PADDING, row*CELL_HEIGHT + PADDING, CELL_WIDTH - PADDING*2, CELL_HEIGHT - PADDING*2, cellColor);
      }

    }
  }
}


void circlingCircles(void) {
  uint32_t color = 0xFF121212;
  simpcFill(pixels, WIDTH, HEIGHT, color);

  const size_t COUNT = 150;
  const size_t BIG_RADIUS = 350;

  const float ROTATIONS = 5;
  
  for (size_t i = 0; i < COUNT; i++) {
    float t = (float)i / COUNT;

    float angle = ROTATIONS * TAU / COUNT * i;
    float currentBigRadius = BIG_RADIUS * t;

    size_t x = (float)WIDTH/2 + cosf(angle) * currentBigRadius;
    size_t y = (float)HEIGHT/2 + sinf(angle) * currentBigRadius;

    float radius = simpcLerp(5, 50, t);

    simpcFillCircleSSAA(pixels, WIDTH, HEIGHT, x, y, radius, simpcLerpColor(0xFF990000, 0xFFFFFF00, t));
  }
}

void circleGrid(void) {
  srand(time(NULL));

  uint32_t color = 0xFF121212;
  simpcFill(pixels, WIDTH, HEIGHT, color);

  const size_t ROWS = 8*3;
  const size_t COLS = 10*3;

  const size_t CELL_WIDTH = WIDTH / COLS;
  const size_t CELL_HEIGHT = HEIGHT / ROWS;

  for (size_t row = 0; row < ROWS; row++) {
    for (size_t col = 0; col < COLS; col++) {
      size_t radius = simpcLerp(5, 15, (double)rand() / RAND_MAX);

      size_t thickness = simpcLerp(2, 5, (radius - 5) / 10.0f);

      simpcDrawArc(pixels, WIDTH, HEIGHT, col*CELL_WIDTH + CELL_WIDTH/2, row*CELL_HEIGHT + CELL_HEIGHT/2, radius, thickness, 0, TAU, simpcLerpColor(0xFFFF0000, 0xFF0000FF, (float)(row+col) / (ROWS+COLS)));
    }
  }
}

void randomCircles(void) {
  srand(time(NULL));

  uint32_t color = 0xFF121212;
  simpcFill(pixels, WIDTH, HEIGHT, color);

  for (size_t i = 0; i < 500; i++) {
    int x = rand() % WIDTH;
    int y = rand() % HEIGHT;
    int radius = 5 + rand() % 50;
    uint8_t r = rand() % 255;
    uint8_t g = rand() % 255;
    uint8_t b = rand() % 255;
    uint8_t a = 55 + rand() % 200;


    simpcFillCircleSSAA(pixels, WIDTH, HEIGHT, x, y, radius, simpcColorFromRGBA(r, g, b, a));
  }
}

void randomRectangles(void) {
  srand(time(NULL));

  uint32_t color = 0xFF121212;
  simpcFill(pixels, WIDTH, HEIGHT, color);

  for (size_t i = 0; i < 5000; i++) {
    int x = rand() % WIDTH;
    int y = rand() % HEIGHT;

    int rw = 30 + rand() % 50;
    int rh = 30 + rand() % 50;
    
    uint8_t r = rand() % 255;
    uint8_t g = rand() % 255;
    uint8_t b = rand() % 255;
    uint8_t a = 55 + rand() % 200;


    simpcFillRect(pixels, WIDTH, HEIGHT, x, y, rw, rh, simpcColorFromRGBA(r, g, b, a));
  }
}

#define IMAGES_DIR "example_images"


#define DEF_EXAMPLE(exampleFunc) do { \
      clock_t time = clock(); \
      exampleFunc(); \
      if(trySaveToPPM(IMAGES_DIR"/"#exampleFunc".ppm") == false) printf(#exampleFunc "-> ERR: %s\n", strerror(errno)); \
      else { \
      printf(#exampleFunc " OK -> took: %fs\n", endClock(time)); \
    } \
  } while (0)

float endClock(clock_t time) {
  return (float)(clock() - time) / CLOCKS_PER_SEC;
}

int main(int argc, char *argv[])
{
  (void) argc;
  (void) argv;

  DIR *imageDir = opendir(IMAGES_DIR);

  if(imageDir == NULL) {
    fprintf(stderr, "Error: cannot open directory "IMAGES_DIR" for writing\n");
    return -1;
  }

  printf("rendering..\n");

  clock_t totalTime = clock();

  DEF_EXAMPLE(allShapes);
  DEF_EXAMPLE(checker);
  DEF_EXAMPLE(randomCircles);
  DEF_EXAMPLE(circlingCircles);
  DEF_EXAMPLE(circleGrid);
  DEF_EXAMPLE(randomRectangles);
  DEF_EXAMPLE(lines);
  DEF_EXAMPLE(bezier);

  printf("total execution time: %fs\n", endClock(totalTime));

  return 0;
}
