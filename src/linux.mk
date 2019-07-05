INC=$(BASEDIR)/inc
SRC=$(BASEDIR)/src

obj/%.o:$(SRC)/%.c $(SRC)/mac.mk makefile.mac
	gcc -g -Wall -I $(BASEDIR)/inc -c $< -o $@

obj/%.o:$(SRC)/%.cc $(INC)/%.hh $(SRC)/mac.mk makefile.mac
	$(CC) $(CFLAGS) -c $< -o $@
