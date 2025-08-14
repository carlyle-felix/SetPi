FLAGS	=	-Wall -g

set_pi: set_pi.o manager.o
	gcc -o set_pi src/set_pi.c src/manager.c ${FLAGS}

set_pi.o: src/set_pi.c incl/manager.h
	gcc -c src/set_pi.c

manager.o: src/manager.c incl/manager.h
	gcc -c src/manager.c
