FLAGS	=	-Wall -Wextra -g

setpi: setpi.o manager.o
	gcc -o setpi src/setpi.c src/manager.c ${FLAGS}

setpi.o: src/setpi.c incl/manager.h
	gcc -c src/setpi.c

manager.o: src/manager.c incl/manager.h
	gcc -c src/manager.c

.PHONY: clean install

clean: 
		rm setpi setpi.o manager.o

install: setpi
		install -Dm755 setpi /usr/local/bin
		install -Dm644 bash-completion/setpi /etc/bash_completion.d/setpi
		mkdir -p /etc/setpi/profiles

uninstall: 
		rm /usr/local/bin/setpi
		rm -rf /etc/setpi
		rm /etc/bash_completion.d/setpi
