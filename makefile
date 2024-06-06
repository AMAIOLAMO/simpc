CC=cc
CFLAGS=-Wall -Wextra

example_imgs: example_imgs.c
	$(CC) $(CFLAGS) -O3 -o example_imgs example_imgs.c -lm

clean:
	rm ./example_imgs
	rm ./images/*
