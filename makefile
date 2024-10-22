CC=cc
CFLAGS=-Wall -Wextra

all: example_imgs renderer

example_imgs: example_imgs.c
	$(CC) $(CFLAGS) -O3 -o example_imgs example_imgs.c -lm

renderer/vrenderer: renderer/vrenderer.c
	$(CC) $(CFLAGS) -O3 -o renderer/vrenderer renderer/vrenderer.c -lm

clean:
	rm ./example_imgs
	rm ./images/*
