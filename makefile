# makefile para compilar o Servidor FTP
# makefile criado por Ricardo Robaina em 01/05

all: ServerFTP.c	
	gcc ServerFTP.c -o ServerFTP
	clear
	./ServerFTP


