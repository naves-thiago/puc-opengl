CC=g++
CFLAGS=-g -Wall -I ../inc
LDFLAGS=-lglfw -lGL -lX11 -lpthread -lXi -ldl

PROG=shader
all: $(PROG)

$(PROG): obj/glad.o obj/shader.o obj/main.o
	$(CC) $^ -o $@ $(LDFLAGS)

obj/%.o:src/%.cc
	$(CC) $(CFLAGS) -c $^ -o $@

obj/glad.o:../src/glad.c
	gcc -g -Wall -I ../inc -c $^ -o $@

obj/shader.o:../src/shader.cc
	$(CC) $(CFLAGS) -c $^ -o $@

%.o: Makefile
