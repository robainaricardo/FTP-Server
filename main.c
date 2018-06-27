/*
Unipampa Engenharia de computação
Redes de computadores 2018/1
Trabalho prático de programação em Redes
Servidor FTP - RFC 959

Grupo: Eduardo Carvalho Teixeira 131151688
       Ricardo Peixoto Robaina 131151690

“Este projeto foi desenvolvido integralmente pela equipe, sem ajuda não autorizada de
alunos não membros do projeto no processo de codificação”.

Código base utilizado neste projeto: https://github.com/mrleiju/FTPd
*/

#include"lib.h"




// Main function controling the entire logic of the ftp server
int main(){
    // initialize socket descriptor and sockaddr_in variable
    unsigned int port = PORTA;
    char buf[BUF_SIZE + 1] = "";
    // sa   -   socket address, used to setup the listening port and connection.
    // ca   -   client address, used to actively connected data session.
    //          ca will be initialized by PORT command
    struct sockaddr_in sa, ca;
    iniciaSocket(&sa, NULL, port);
    int sd = criarSocket(&sa);

    int recurso = 10000;

    if (sd < 0){
        fprintf(stderr, "Erro ao crirar o socket!");
        return -2;
    }
    // listen
    if(listen(sd, MAX_CONNECTIONS) == -1){
        fprintf(stderr, "Erro ao escutar\n");
        close(sd);
        return -3;
    }


    //Criar as threads===
    struct arg_instancia arg1, arg2;
    arg1.sd = sd;
    arg2.sd = sd;
    arg1.ca = ca;
    arg2.ca = ca;
    //arg1.recurso = recurso;
    //arg2.recurso = recurso;
    printf("%d\n",  recurso);
    //arg1.cd = cd;
    //arg2.cd = cd;

    pthread_t t1;
    pthread_create(&t1, NULL, instancia, (void*)&arg1);

    pthread_t t2;
    pthread_create(&t2, NULL, instancia, (void*)&arg2);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);


    close(sd);
    return 0;
}
