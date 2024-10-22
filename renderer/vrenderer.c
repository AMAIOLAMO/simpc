#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <wait.h>
#include <stdbool.h>
#include <errno.h>
#include <assert.h>
#include <dlfcn.h>

#define DA_APPEND(da, value) \
  do {                       \
    while((da)->count >= (da)->capacity) { \
      (da)->capacity = (da)->capacity == 0 ? 1 : (da)->capacity * 2; \
      (da)->items = realloc((da)->items, (da)->capacity * sizeof( (da)->items[0] )); \
    } \
    (da)->items[(da)->count++] = value; \
  } while(0)

#define DA_APPEND_MANY(dynArray, appendItems, itemsCount) \
  do { \
    while (((dynArray)->count + (itemsCount)) > (dynArray)->capacity) { \
      (dynArray)->capacity = (dynArray)->capacity == 0 ? 1 : (dynArray)->capacity * 2; \
      (dynArray)->items = realloc((dynArray)->items, (dynArray)->capacity * sizeof((dynArray)->items[0])); \
    } \
    memcpy(&(dynArray)->items[(dynArray)->count], appendItems, (itemsCount) * sizeof((dynArray)->items[0])); \
    (dynArray)->count += (itemsCount); \
  } while (0)


typedef struct {
  char **items;
  size_t count, capacity;
} Cmd;

#define SINGLE_ARG_MAX_LEN 255

#define CMD_APPEND(cmd, fmt, ...) DA_APPEND(cmd, arg)

#define CMD_APPEND_VA(cmd, ...) \
  do { \
    const char* args[] = { __VA_ARGS__ }; \
    DA_APPEND_MANY(cmd, args, sizeof( args ) / sizeof( args[0] )); \
  } while (0)


// must be null terminated, andd first arg  
// must contain the program name 
#define CMD_RUN_CURRENT(cmdPtr) \
  do { \
    Cmd cmdNullTerminated = {0}; \
    DA_APPEND_MANY(&cmdNullTerminated, (cmdPtr)->items, (cmdPtr)->count); \
    DA_APPEND(&cmdNullTerminated, NULL); \
    execvp(cmdNullTerminated.items[0], cmdNullTerminated.items); \
  } while(0)

#define CMD_RUN_ASYNC(cmdPtr) \
  do { \
    pid_t pid = fork(); \
    if(pid == 0) CMD_RUN_CURRENT(cmdPtr); \
    if(pid < 0) perror("In fork: "); \
  } while(0)

#define CMD_RUN_SYNC(cmdPtr) \
  do { \
    CMD_RUN_ASYNC(cmdPtr); \
    pid_t pid = getpid(); \
    if(pid > 0) wait(NULL); \
  } while(0)


typedef struct {
  const char *data;
  size_t length;
} StrView;

typedef struct {
  pid_t pid;
  int ioPipes[2]; // file descriptors representing the I/O pipes
} FFMPEG;

void ffmpegStartIO(FFMPEG *ffmpeg) {
  pipe(ffmpeg->ioPipes);
}

int* ffmpegGetReadPipe(FFMPEG *ffmpeg) {
  return &ffmpeg->ioPipes[0];
}

int* ffmpegGetWritePipe(FFMPEG *ffmpeg) {
  return &ffmpeg->ioPipes[1];
}


#define FFMPEG_FMT_SIZE 256
int ffmpegBeginRendering(FFMPEG *ffmpeg, const char* outputPath, size_t width, size_t height, size_t fps) {
  ffmpeg->pid = fork();

  if(ffmpeg->pid < 0) return -1;

  // is parent process
  if(ffmpeg->pid > 0) {
    // close useless pipe 
    close(*ffmpegGetReadPipe(ffmpeg));
    return 0;
  }
  // else Is child process
  
  // close useless pipe 
  close(*ffmpegGetWritePipe(ffmpeg));

  close(STDIN_FILENO);
  dup(*ffmpegGetReadPipe(ffmpeg)); // duplicates the read pipe to our stdout
  // dup2(0, *readPipe);

  Cmd cmd = {0};

  char sizeBuf[FFMPEG_FMT_SIZE];
  snprintf(sizeBuf, FFMPEG_FMT_SIZE, "%zux%zu", width, height);
  
  char fpsBuf[FFMPEG_FMT_SIZE];
  snprintf(fpsBuf, FFMPEG_FMT_SIZE, "%zu", fps);

  // TODO: do character stuff
  CMD_APPEND_VA(&cmd, "ffmpeg",
    "-y",
    "-f", "rawvideo",
    "-vcodec", "rawvideo",
    "-s", sizeBuf,
    "-pix_fmt", "rgb24",
    "-r", fpsBuf,
    "-i", "-", // input block (from pipe)

    "-an",
    "-vcodec", "mpeg4",
    outputPath
  );

  CMD_RUN_CURRENT(&cmd);
  return 0;
}

void ffmpegEndRendering(FFMPEG *ffmpeg) {
  close(*ffmpegGetWritePipe(ffmpeg));
  wait(NULL);
}


void ffmpegSendRawFrame(FFMPEG *ffmpeg, uint32_t *pixels, size_t width, size_t height) {
  for (size_t pixelIndex = 0; pixelIndex < width*height; pixelIndex++) {
    write(
      *ffmpegGetWritePipe(ffmpeg),
      &pixels[pixelIndex],
      sizeof(uint8_t)*3
    );
  }
}

typedef char* mut_str;

mut_str shiftArgs(int *argcRef, mut_str **argvRef) {
  mut_str popStr = (*argvRef)[0];

  *argcRef -= 1;
  *argvRef = &(*argvRef)[1];

  return popStr;
}

typedef void (*UpdateFrame_t)(uint32_t *pixels, size_t width, size_t height, size_t frameIndex, size_t totalFrames, size_t fps);

typedef size_t (*GetFps_t)(void);
typedef size_t (*GetTotalFrames_t)(void);
typedef size_t (*GetWidth_t)(void);
typedef size_t (*GetHeight_t)(void);

#define ANIM_FUNCS \
  REPL(updateFrame) \
  REPL(getFps) \
  REPL(getTotalFrames) \
  REPL(getWidth) \
  REPL(getHeight)

typedef struct {
  void *dlPlugin;

  UpdateFrame_t updateFrame;
  GetFps_t getFps;
  GetTotalFrames_t getTotalFrames;
  GetWidth_t getWidth;
  GetHeight_t getHeight;
} Animation;


bool animationLoad(Animation *animation, const char* animationPath) {
  animation->dlPlugin = dlopen(animationPath, RTLD_NOW);

  if(animation->dlPlugin == NULL)
    return false;


  #define REPL(item) \
    animation->item = dlsym(animation->dlPlugin, #item); \
    if(animation->item == NULL) return false;

  ANIM_FUNCS

  #undef REPL

  return true;
}

void animationUnload(Animation *animation) {
  assert(animation != NULL);
  dlclose(animation->dlPlugin);
}

#define LOG_PREFIX "[RENDERER] "

bool ffmpegRenderAnimation(FFMPEG *ffmpeg, Animation *animation, const char *outputPath) {
  ffmpegStartIO(ffmpeg);

  int renderErr = ffmpegBeginRendering(ffmpeg, outputPath,
          animation->getWidth(), animation->getHeight(),
          animation->getFps());

  if(renderErr < 0) {
    fprintf(stderr, LOG_PREFIX"Err: %s\n", strerror(errno));
    return false;
  }

  uint32_t *pixels = malloc(sizeof(uint32_t) * animation->getWidth() * animation->getHeight());


  for (size_t frameIndex = 0; frameIndex < animation->getTotalFrames(); frameIndex++) {
    animation->updateFrame(pixels, animation->getWidth(),
            animation->getHeight(), frameIndex, animation->getTotalFrames(),
            animation->getFps());

    ffmpegSendRawFrame(ffmpeg, pixels, animation->getWidth(), animation->getHeight());
  }

  free(pixels);

  ffmpegEndRendering(ffmpeg);
  return true;
}


int main(int argc, char *argv[])
{
  if(argc != 2+1) {
    fprintf(stderr, "Err: did not specify animation library path\nUsage: vrenderer <animationPlugPath> <outputPath>\n");
    return 1;
  }

  char *program = shiftArgs(&argc, &argv); (void) program;
  char *animationPath = shiftArgs(&argc, &argv);
  char *outputPath = shiftArgs(&argc, &argv);

  Animation animation = {0};

  printf(LOG_PREFIX"loading plugin from path: %s\n", animationPath);

  if(animationLoad(&animation, animationPath) == false) {
    fprintf(stderr, "Err: %s\n", dlerror());
    return 1;
  }

  printf(LOG_PREFIX"%s loaded\n\tfps: %zu\n", animationPath, animation.getFps());


  FFMPEG ffmpeg = {0};

  printf(LOG_PREFIX"Begin Rendering...\n");

  clock_t renderTime = clock();
  ffmpegRenderAnimation(&ffmpeg, &animation, outputPath);

  printf(LOG_PREFIX"Rendering Complete!, took: %fs\n", (float)(clock() - renderTime) / CLOCKS_PER_SEC);

  animationUnload(&animation);

  return 0;
}
