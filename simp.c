#ifndef SIMP_C_
#define SIMP_C_

#define PI 3.14159265358979f
#define TAU (PI*2.0f)

#define returnDefer(value) do { result = (value); goto defer; } while(0)

typedef int Errno;

#define getColorPart(color, index) (((color)>>(8*(index)))&0xFF)
#define getRed(color) getColorPart(color, 0)
#define getGreen(color) getColorPart(color, 1)
#define getBlue(color) getColorPart(color, 2)
#define getAlpha(color) getColorPart(color, 3)

float simpcGetAnimationTime(size_t frameIndex, size_t fps) {
  float dt = 1.0f / fps;
  return dt * frameIndex;
}

int max(int a, int b) { return a > b ? a : b; }
int min(int a, int b) { return a < b ? a : b; }

float simpcLerp(float a, float b, float t) {
  return a + (b - a) * t;
}

uint32_t simpcLerpColor(uint32_t a, uint32_t b, float t) {
  uint32_t result = 0;

  uint8_t ar = (a>>(8*0))&0xFF;
  uint8_t ag = (a>>(8*1))&0xFF;
  uint8_t ab = (a>>(8*2))&0xFF;

  uint8_t br = (b>>(8*0))&0xFF;
  uint8_t bg = (b>>(8*1))&0xFF;
  uint8_t bb = (b>>(8*2))&0xFF;

  result |= (uint32_t)simpcLerp(ab, bb, t);
  result <<= 8;

  result |= (uint32_t)simpcLerp(ag, bg, t);
  result <<= 8;

  result |= (uint32_t)simpcLerp(ar, br, t);

  return result;
}

uint32_t simpcColorFromRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  uint32_t color = 0;

  // 0xAABBGGRR
  color |= a;
  color <<= 8;

  color |= b;
  color <<= 8;

  color |= g;
  color <<= 8;
  
  color |= r;

  return color;
}

uint32_t simpcColorFromRGB(uint8_t r, uint8_t g, uint8_t b) {
  return simpcColorFromRGBA(r, g, b, 255);
}


void simpcAlphaMix(uint32_t *originalColor, uint32_t mixColor) {
  uint8_t alpha = getAlpha(mixColor);

  *originalColor = simpcLerpColor(*originalColor, mixColor, alpha / 255.0f);
}

Errno simpcSavePPM(uint32_t *pixels, size_t width, size_t height, const char *filePath) {
  int result = 0;

  // open for write and binary mode
  FILE *file = fopen(filePath, "wb");
  if(file == NULL) returnDefer(errno);

  fprintf(file, "P6\n%zu %zu 255\n", width, height);
  if(ferror(file)) returnDefer(errno);

  for (size_t i = 0; i < width*height; i++) {
    uint32_t pixel = pixels[i];
    uint8_t bytes[3] = {
      getRed(pixel),
      getGreen(pixel),
      getBlue(pixel)
    };
    fwrite(bytes, sizeof(bytes), 1, file);

    if(ferror(file)) returnDefer(errno);
  }

defer:
  if(file) fclose(file);
  return result;
}

void simpcFill(uint32_t *pixels, size_t width, size_t height, uint32_t color) {
  for (size_t i = 0; i < width*height; i++)
    simpcAlphaMix(&pixels[i], color);
}


void simpcFillRect(uint32_t *pixels, size_t pixelsWidth, size_t pixelsHeight, int x, int y, size_t width, size_t height, uint32_t color) {
  size_t ix = (size_t)max(x, 0);
  size_t iy = (size_t)max(y, 0);

  size_t ex = (size_t)min(x + width, pixelsWidth - 1);
  size_t ey = (size_t)min(y + height, pixelsHeight - 1);

  for (size_t cy = iy; cy <= ey; cy++)
    for (size_t cx = ix; cx <= ex; cx++)
      simpcAlphaMix(&pixels[cy*pixelsWidth + cx], color);
}

bool simpcIsWithinRadius(int dx, int dy, size_t radius) {
  return (size_t)(dx*dx + dy*dy) <= (radius*radius);
}

void simpcFillCircle(uint32_t *pixels, size_t width, size_t height, int x, int y, size_t radius, uint32_t color) {
  int tlx = x - radius;
  int tly = y - radius;
  int brx = x + radius;
  int bry = y + radius;

  size_t ix = max(tlx, 0);
  size_t iy = max(tly, 0);
  size_t ex = min(brx, width - 1);
  size_t ey = min(bry, height - 1);

  for (size_t cy = iy; cy <= ey; cy++) {
    for (size_t cx = ix; cx <= ex; cx++) {
      if(simpcIsWithinRadius(cx - x, cy - y, radius) == false) continue;

      simpcAlphaMix(&pixels[cy*width + cx], color);
    }
  }
}

void simpcColorAdd(uint32_t *r, uint32_t *g, uint32_t *b, uint32_t color) {
  *r += getRed(color);
  *g += getGreen(color);
  *b += getBlue(color);
}

void simpcFillCircleSSAA(uint32_t *pixels, size_t width, size_t height, int x, int y, size_t radius, uint32_t color) {
  int tlx = x - radius;
  int tly = y - radius;
  int brx = x + radius;
  int bry = y + radius;

  size_t ix = max(tlx, 0);
  size_t iy = max(tly, 0);
  size_t ex = min(brx, width - 1);
  size_t ey = min(bry, height - 1);

  const size_t RES_SCALAR = 2;
  const size_t RES_SCALAR_SQR = RES_SCALAR*RES_SCALAR;

  for (size_t cy = iy; cy <= ey; cy++) {
    for (size_t cx = ix; cx <= ex; cx++) {

      int x2 = x * RES_SCALAR;
      int y2 = y * RES_SCALAR;

      size_t r2 = radius * RES_SCALAR;

      uint32_t r = 0;
      uint32_t g = 0;
      uint32_t b = 0;

      for (size_t cx2 = cx * RES_SCALAR; cx2 <= (cx * RES_SCALAR + 1); cx2++) {
        for (size_t cy2 = cy * RES_SCALAR; cy2 <= (cy * RES_SCALAR + 1); cy2++) {
          uint32_t bgColor = pixels[cy*width + cx];
          
          if(simpcIsWithinRadius(cx2 - x2, cy2 - y2, r2) == false) {
            simpcColorAdd(&r, &g, &b, bgColor);
            continue;
          }
          // else

          // add normal circle color;
          simpcColorAdd(&r, &g, &b, color);
        }
      }

      r /= RES_SCALAR_SQR;
      g /= RES_SCALAR_SQR;
      b /= RES_SCALAR_SQR;
      // calculate averaged color 

      // set current pixel color with averaged color
      uint32_t resultColor = simpcColorFromRGBA(r, g, b, getAlpha(color));
      simpcAlphaMix(&pixels[cy*width + cx], resultColor);
    }
  }
}

void simpcDrawArc(uint32_t *pixels, size_t width, size_t height, int x, int y, size_t radius, size_t thickness, float startAngle, float endAngle, uint32_t color) {
  size_t outerRadius = radius + thickness/2;
  size_t innerRadius = radius - thickness/2;

  int tlx = x - outerRadius;
  int tly = y - outerRadius;
  int brx = x + outerRadius;
  int bry = y + outerRadius;

  size_t ix = max(tlx, 0);
  size_t iy = max(tly, 0);
  size_t ex = min(brx, width - 1);
  size_t ey = min(bry, height - 1);

  for (size_t cy = iy; cy <= ey; cy++) {
    for (size_t cx = ix; cx <= ex; cx++) {
      float dx = (float)cx - (float)x;
      float dy = (float)cy - (float)y;

      size_t distSqr = (cx - x)*(cx - x) + (cy - y)*(cy - y);

      if(distSqr > (outerRadius*outerRadius)) continue;
      if(distSqr < (innerRadius*innerRadius)) continue;

      float ca = atan2f(dy, dx);

      if(ca < 0) ca = TAU + ca;
      if(ca > endAngle || ca < startAngle) continue;

      simpcAlphaMix(&pixels[cy*width + cx], color);
    }
  }
}

void simpcFillSector(uint32_t *pixels, size_t width, size_t height, int x, int y, size_t radius, float startAngle, float endAngle, uint32_t color) {
  int tlx = x - radius;
  int tly = y - radius;
  int brx = x + radius;
  int bry = y + radius;

  size_t ix = max(tlx, 0);
  size_t iy = max(tly, 0);
  size_t ex = min(brx, width - 1);
  size_t ey = min(bry, height - 1);

  for (size_t cy = iy; cy <= ey; cy++) {
    for (size_t cx = ix; cx <= ex; cx++) {
      float dx = (float)cx - (float)x;
      float dy = (float)cy - (float)y;

      size_t distSqr = ((cx - x)*(cx - x) + (cy - y)*(cy - y));

      if(distSqr > (radius*radius)) continue;

      float ca = atan2f(dy, dx);

      if(ca < 0) ca = TAU + ca;
      if(ca > endAngle || ca < startAngle) continue;

      simpcAlphaMix(&pixels[cy*width + cx], color);
    }
  }
}

float simpcAbsf(float a) { return a < 0.0f ? -a : a; }

float triangleArea(int x1, int y1, int x2, int y2, int x3, int y3)
{
     return simpcAbsf((x1*(y2-y3) + x2*(y3-y1) + x3*(y1-y2))/2.0f);
}

bool isInsideTriangle(int x1, int y1, int x2, int y2, int x3, int y3, int x, int y)
{   
   /* Calculate triangleArea of triangle ABC */
   float A = triangleArea(x1, y1, x2, y2, x3, y3);
  
   /* Calculate triangleAreaof triangle PBC */ 
   float A1 = triangleArea(x, y, x2, y2, x3, y3);
  
   /* Calculate triangleAreaof triangle PAC */ 
   float A2 = triangleArea(x1, y1, x, y, x3, y3);
  
   /* Calculate triangleAreaof triangle PAB */  
   float A3 = triangleArea(x1, y1, x2, y2, x, y);
    
   /* Check if sum of A1, A2 and A3 is same as A */
   return (A == A1 + A2 + A3);
}

void simpcFillTriangle(uint32_t *pixels, size_t width, size_t height, int x1, int y1, int x2, int y2, int x3, int y3, uint32_t color) {
  int lx = min(x1, min(x2, x3));
  int rx = max(x1, max(x2, x3));


  int ly = min(y1, min(y2, y3));
  int ry = max(y1, max(y2, y3));

  size_t ix = (size_t)max(lx, 0);
  size_t iy = (size_t)max(ly, 0);

  size_t ex = (size_t)min(rx, width - 1);
  size_t ey = (size_t)min(ry, height - 1);

  for (size_t cy = iy; cy <= ey; cy++)
    for (size_t cx = ix; cx <= ex; cx++)
      if(isInsideTriangle(x1, y1, x2, y2, x3, y3, cx, cy))
        simpcAlphaMix(&pixels[cy*width + cx], color);
}

int simpcClampInt(int value, int minValue, int maxValue) {
  return min(max(value, minValue), maxValue);
}

void simpcDrawLine(uint32_t *pixels, size_t width, size_t height, int x1, int y1, int x2, int y2, uint32_t color) {
  // y = mx + c
  // m = (y - c) / m 
  // m = (y2 - y1) / (x2 - x1) 
  // c = y - mx
  
  size_t iy = max(min(y1, y2), 0);
  size_t ey = min(max(y1, y2), height - 1);

  if(x2 == x1) {
    for (size_t cy = iy; cy <= ey; cy++)
      simpcAlphaMix(&pixels[cy*width + x1], color);
    return;
  }

  size_t ix = max(min(x1, x2), 0);
  size_t ex = min(max(x1, x2), width - 1);

  float m = (float)(y2 - y1) / (x2 - x1);
  float c = y1 - m * x1;

  int lastY = m * ix + c;
  for (size_t cx = ix; cx <= ex; cx++) {
    int targetY = m * cx + c;


    // filling inbetween the gaps of the line  
    int cy = lastY;
    while (true) {
      if(m >= 0 ? cy > targetY : cy < targetY)
        break;
      // else
      if(cy < 0 || (size_t)cy >= height) {
        cy += m >= 0 ? 1 : -1;
        continue;
      }

      simpcAlphaMix(&pixels[cy*width + cx], color);
      cy += m >= 0 ? 1 : -1;
    }
    lastY = targetY;
  }
}

void simpcSampleBezier(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, float *resultX, float *resultY, float t) {
  float lx1 = simpcLerp(x1, x2, t);
  float ly1 = simpcLerp(y1, y2, t);
  float lx2 = simpcLerp(x2, x3, t);
  float ly2 = simpcLerp(y2, y3, t);
  float lx3 = simpcLerp(x3, x4, t);
  float ly3 = simpcLerp(y3, y4, t);
  float lx11 = simpcLerp(lx1, lx2, t);
  float ly11 = simpcLerp(ly1, ly2, t);
  float lx12 = simpcLerp(lx2, lx3, t);
  float ly12 = simpcLerp(ly2, ly3, t);

  *resultX = simpcLerp(lx11, lx12, t);
  *resultY = simpcLerp(ly11, ly12, t);
}

void simpcDrawBezier(uint32_t *pixels, size_t width, size_t height, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, uint32_t color, size_t resolution) {
  for (size_t currentSection = 0; currentSection < resolution; currentSection++) {
    float t1 = 1.0f / resolution * currentSection;
    float t2 = 1.0f / resolution * (currentSection + 1);

    float lineX1, lineY1;
    float lineX2, lineY2;

    simpcSampleBezier(x1, y1, x2, y2, x3, y3, x4, y4, &lineX1, &lineY1, t1);

    simpcSampleBezier(x1, y1, x2, y2, x3, y3, x4, y4, &lineX2, &lineY2, t2);

    simpcDrawLine(pixels, width, height, lineX1, lineY1, lineX2, lineY2, color);
  }
}

// TODO: implement line culling in line algorithm  
// TODO: re implement fill triangle to use horizontal fill algorithm  
// TODO: implement basic 3D view of a Cube  
// TODO: allow rendering of a video  



#endif /* ifndef SIMP_C_ */
