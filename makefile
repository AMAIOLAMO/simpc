CC=cc
CFLAGS=-Wall -Wextra
LINKERFLAGS=-lm

all: example_imgs renderer

example_imgs: example_imgs.c
	$(CC) $(CFLAGS) -o example_imgs example_imgs.c $(LINKERFLAGS)

renderer: ./renderer/vrenderer.c
	$(CC) $(CFLAGS) -o vrenderer ./renderer/vrenderer.c $(LINKFLAGS)

clean:
	rm ./example_imgs
	rm ./vrenderer
	rm ./images/*
