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
    

    //bind(servidorSocket, (struct sockaddr*)&servidor, sezeof(servidor));

    //listen(servidorSocket, 1);

    printf("A biblioteca exite!\n");
    return 0;
}
