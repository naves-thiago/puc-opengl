UNAME := $(shell uname)
PROG=model
BASEDIR=..

all:

ifeq ($(UNAME), Darwin)
	@make -f makefile.mac PROG=$(PROG) BASEDIR=$(BASEDIR)
else
	@make -f makefile.linux PROG=$(PROG) BASEDIR=$(BASEDIR)
endif

clean:
	rm -f obj/* $(PROG)
