OBJ = main.o game.o matrix.o utils.o gui.o utf8_lib.o
VPATH = src src/core src/gui
NAME = the_lurking_death
BASEDIR = $(PWD)

$(NAME): $(OBJ)
	gcc -o $@ $(OBJ) -lncursesw -ltinfo -lmenu
	rm -f *.o

%.o: %.c
	gcc -g -c $<

tests:
	cd test && mkdir tests && cd tests
	gcc $(BASEDIR)/test/test_matrix.c $(BASEDIR)/src/core/matrix.c -o $(BASEDIR)/test/tests/matrix

run_tests:
	$(BASEDIR)/test/tests/matrix

clean:
	rm -f $(NAME)
	rm -f *.o
	rm -rf test/tests
