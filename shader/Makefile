CC=g++
CFLAGS=-g -Wall -I ../include -I inc
LDFLAGS=-lglfw -lGL -lX11 -lpthread -lXi -ldl

PROG=shader
all: $(PROG)

$(PROG): obj/glad.o obj/shader.o obj/main.o
	$(CC) $(LDFLAGS) $^ -o $@

obj/%.o:src/%.cc
	$(CC) $(CFLAGS) -c $^ -o $@

obj/glad.o:src/glad.c
	gcc -g -Wall -I ../include -I inc -c $^ -o $@

%.o: Makefile
