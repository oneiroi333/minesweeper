OBJ = main.o game.o matrix.o utils.o gui.o utf8_lib.o
NAME = the_lurking_death

$(NAME): $(OBJ)
	gcc -o $@ $(OBJ) -lncursesw -ltinfo -lmenu
	rm -f *.o

main.o: game.o gui.o
	gcc -g -c src/main.c src/core/game.c src/gui/gui.c

game.o: matrix.o utils.o
	gcc -g -c src/core/game.c src/core/matrix.c src/gui/gui.c

gui.o: utils.o utf8_lib.o
	gcc -g -c src/gui/gui.c src/core/utils.c src/gui/utf8_lib.c

matrix.o:
	gcc -g -c src/core/matrix.c

utils.o:
	gcc -g -c src/core/utils.c

utf8_lib.o:
	gcc -g -c src/gui/utf8_lib.c

clean:
	rm -f $(NAME)
	rm -f *.o
