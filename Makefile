CFLAGS=-g -Wall -std=c99 -I include
LDFLAGS=-lglfw -lGL -lX11 -lpthread -lXi -ldl

all: helloworld

helloworld: obj/glad.o obj/main.o
	gcc $(LDFLAGS) $^ -o $@

obj/%.o:src/%.c
	gcc $(CFLAGS) -c $^ -o $@

%.o: Makefile
