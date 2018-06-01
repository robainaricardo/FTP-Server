# makefile para compilar o Servidor FTP
# makefile criado por Ricardo Robaina em 01/05

all:
	gcc -c *.c
	gcc -o FTP-Server *.o -lpthread
	clear
	./FTP-Server
