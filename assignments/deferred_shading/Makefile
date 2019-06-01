UNAME := $(shell uname)

all:

ifeq ($(UNAME), Darwin)
	@make -f makefile.mac
else
	@make -f makefile.linux
endif

clean:
	rm obj/*
	rm normal
