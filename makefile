CC = gcc
CFLAGS = $(shell sdl2-config --cflags)
LIBS   = $(shell sdl2-config --libs)

SERVER_CFLAGS = -Wall -Wextra -g

client:
	$(CC) client.c -o client $(CFLAGS) $(LIBS)

server:
	$(CC) server.c net.c -o server $(SERVER_CFLAGS)

clean:
	rm -f server client

all: server client
