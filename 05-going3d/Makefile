CC=g++
CFLAGS=-g -Wall -I ../inc
LDFLAGS=-lglfw -lGL -lX11 -lpthread -lXi -ldl

PROG=3d

LIB=glad.o shader.o texture.o stb_image.o
_LIB=$(addprefix obj/, $(LIB))

all: $(PROG)

clean:
	rm -f obj/* $(PROG)

$(PROG): $(_LIB) obj/main.o
	$(CC) $^ -o $@ $(LDFLAGS)

obj/%.o:src/%.cc Makefile
	$(CC) $(CFLAGS) -c $< -o $@

obj/%.o:../src/%.c Makefile
	gcc -g -Wall -I ../inc -c $< -o $@

obj/%.o:../src/%.cc Makefile
	$(CC) $(CFLAGS) -c $< -o $@

