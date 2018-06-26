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

volatile int recurso = 30;

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
    int length = 0;
    bool image_mode;
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

    //Criando instancias do servidor
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
      fork();
    }

    while(true){ // keep listening on the port
        printf("+++++++++++++++++    Servidor FTP ON     ++++++++++++++++++\n");
        int velocidade = 0;

        // cd is the connection descriptor
        image_mode = false;
        int cd = accept(sd, NULL, NULL);
        if (cd < 0){
            fprintf(stderr, "Erro ao aceitar");
            close(sd);
            return -3;
        }
        // send 220 to the client
        mensagem(220, cd);

        if(autencicacao(cd)){
            fprintf(stderr, "Erro ao fazer o login");
            encerrarConexao(cd, sd);
            continue;
        }



        length = read(cd, buf, BUF_SIZE - 1);



        while(length > 0){ // start to parse command

            printf("%d\n", velocidade);

            buf[length] = '\0';
            if (strncmp(buf, "quit", 4) == 0 || strncmp(buf, "QUIT", 4) == 0){
                mensagem(221, cd);
                break;
            }
            else if (strncmp(buf, "SYST\r\n", 6) == 0){
                mensagem(215, cd);
            }
            else if (strncmp(buf, "STRU ", 5) == 0){
                if (strncmp(buf + 5, "F\r\n", 3) == 0){
                    mensagem(200, cd);
                }
                else{
                    mensagem(504, cd); // Error. Only File(F) is supported.
                }
            }
            else if (strncmp(buf, "MODE ", 5) == 0){
                if (strncmp(buf + 5, "S\r\n", 3) == 0){
                    mensagem(200, cd);
                }
                else{
//                     mensagem(200, cd);
                    mensagem(504, cd); // Error. Only Stream(S) mode is supported.
                }
            }
            else if (strncmp(buf, "TYPE ", 5) == 0){
                // Always treat the transmission as binary.
                if (strncmp(buf + 5, "I\r\n", 3) == 0){
                    mensagem(200, cd);
                    image_mode = true;
                }
                else{
//                     mensagem(200, cd);
                    mensagem(504, cd);  // Error. Only Type(I) is supproted.
                }
            }
            else if (strncmp(buf, "PORT ", 5) == 0){
                velocidade = port_parser(buf, &ca);
                recurso -= velocidade;
                printf("Recurso total: %d\n", recurso);
                mensagem(200, cd);
            }
            else if (strncmp(buf, "LIST", 4) == 0){
                // the fifth character of the buf may be space if there is a
                // folder's name following, or '\r' if there is not. Therefore,
                // we have to compare only 4 characters:
                mensagem(150, cd);
                list(&ca, strtok(buf + 4, "\r\n"));
                mensagem(226, cd);
            }
            else if (strncmp(buf, "RETR ", 5) == 0){
                // get
                if (image_mode){
                    if(retr(cd, &ca, strtok(buf + 5, "\r\n"), velocidade) > 0)
                        mensagem(550, cd);
                    else
                        mensagem(226, cd);
                }
                else
                    mensagem(451, cd);
            }
            else if (strncmp(buf, "STOR ", 5) == 0){
                // put
                if (image_mode){
                    if(stor(cd, &ca, strtok(buf + 5, "\r\n"), velocidade) > 0)
                        mensagem(550, cd);
                    else
                        mensagem(226, cd);
                }
                else
                    mensagem(451, cd);
            }
            else if (strncmp(buf, "NOOP\r\n", 6) == 0){
                mensagem(220, cd);
            }
            else{
                fprintf(stderr, "Unsupport command %s.\n", buf);
                mensagem(202, cd);
            }
            length = read(cd, buf, BUF_SIZE - 1);
        } // end of the transaction with a particular client
        encerrarConexao(cd, sd); // shutdown connection with this client
        memset(buf, 0, sizeof(buf)); // clear buffer
    } // end of server
    close(sd);
    return 0;
}
