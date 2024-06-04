# Simpc
> A simple C only graphics library to have fun with

## Building
> simply just cd into the `simpc` directory, and run `make all -B`
> this compiles both `example_imgs` and `vrenderer`

## Components
> there are mainly two major components to this library: `simp.c` and `vrenderer.c`

`simp.c` is the core graphics library for drawing all kinds of shapes on a canvas

`vrenderer.c` is the rendering application, it is used as such:

```bash
./vrenderer yourAnimation.so yourAnimation.mp4
```

which loads the animation from the dynamic library with the name "yourAnimation.so",
and renders it into a video file of name "yourAnimation.mp4"


