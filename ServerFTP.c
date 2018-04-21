#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h> //biblioteca para o uso de sockts
#include<netdb.h>

int main(){

    int servidorSocket;
    int clienteSocket;

    struct sockaddr_in servidor, cliente;


    //Cria um socket com (,Servico com conexão TCP,)
    servidorSocket = socket(AF_INET, SOCK_STREAM, 0);
    printf("Criou o socket com número %d\n", servidorSocket);
    //se socket retornar -1 é que não conseguiu

    //bzero(&servidor, sizeof(servidor)); //limpa os bytes
    servidor.sin_family = AF_INET;  //protocolo internet
    servidor.sin_addr.s_addr = htonl(INADDR_ANY); //qualquer interface de rede
    servidor.sin_port = htons(2121); //porta



    bind(servidorSocket, (struct sockaddr*)&servidor, sizeof(servidor));
    printf("Bind OK\n");


    //listem pra uma conexão
    listen(servidorSocket, 1);
    printf("Listen OK!\n");

    while(1){
      int clienteSocket = accept(servidorSocket, (struct  sockaddr *) NULL, NULL);
    }


    return 0;
}
