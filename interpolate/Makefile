CFLAGS=-g -Wall -std=c99 -I ../include
LDFLAGS=-lglfw -lGL -lX11 -lpthread -lXi -ldl

PROG=interpolate
all: $(PROG)

$(PROG): obj/glad.o obj/main.o
	gcc $(LDFLAGS) $^ -o $@

obj/%.o:src/%.c
	gcc $(CFLAGS) -c $^ -o $@

%.o: Makefile
