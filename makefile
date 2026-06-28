CC = gcc
CFLAGS = $(shell sdl2-config --cflags)
LIBS   = $(shell sdl2-config --libs)

all:
	$(CC) client.c -o client $(CFLAGS) $(LIBS)
