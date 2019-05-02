CFLAGS=-g -Wall -std=c99 -I ../inc
LDFLAGS=-lglfw -lGL -lX11 -lpthread -lXi -ldl

PROG=interpolate
all: $(PROG)

$(PROG): obj/glad.o obj/main.o
	gcc $^ -o $@ $(LDFLAGS)

obj/%.o:src/%.c
	gcc $(CFLAGS) -c $^ -o $@

%.o: Makefile
