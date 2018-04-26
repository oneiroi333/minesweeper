OBJ = main.o game.o matrix.o utils.o gui.o utf8_lib.o
VPATH = src
NAME = the_lurking_death

$(NAME): $(OBJ)
	gcc -o $@ $(OBJ) -lncursesw -ltinfo -lmenu
	rm -f *.o

%.o: %.c
	gcc -g -c $<

clean:
	rm -f $(NAME)
	rm -f *.o
