#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "errno.h"
#include "unistd.h"
//lib padrão para uso da socket
#include "netinet/in.h"
#include "arpa/inet.h"
#include "sys/types.h"
#include "sys/socket.h"
// lib para usar para converter host para ip
#include "netdb.h"

#define TAMANHO_MENSAGEM 100

int main(){

  int servidorSocket; //Socket do servidor
  int clienteSocket; //Socket do cliente conectado ao servidor

  int estado = 0;

  int tamanho_msg_recebida;

  char bufferIda[4];
  char bufferVolta[TAMANHO_MENSAGEM];

  struct sockaddr_in servidor; //Estrutura que mantem os dados do socket após o bind (Ip e porta)
  struct sockaddr_in cliente;

  //++++++++++++++++++++++SOCKET+++++++++++++++++++++++++++++
  //se socket retornar -1 é que não conseguiu
  //Cria um socket com (Domínio,Servico com conexão TCP, Ip)
  if((servidorSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    perror("Erro ao Criar O Socket");
    exit(-1);
  }else{
    printf("Criou o socket com número %d\n", servidorSocket);
  }
  //++++++++++++++++++++++SOCKET+++++++++++++++++++++++++++++

  //++++++++++++++++++++++BIND+++++++++++++++++++++++++++++
  //atribui os valores desejados na estrutura do socket
  //memset(buffervolta, 0x0, TAMANHO_MENSAGEM);
  bzero((char*)&servidor, sizeof(servidor)); //limpa os bytes
  servidor.sin_family = AF_INET;  //protocolo internet
  servidor.sin_addr.s_addr = htonl(INADDR_ANY); //qualquer interface de rede (ip)
  servidor.sin_port = htons(2121); //porta
  //função bind, linka a estrutura com o socket
  if((bind(servidorSocket, (struct sockaddr*)&servidor, sizeof(servidor))) < 0){
    perror("Erro no Bind");
    exit(-1);
  }else{
    printf("Bind OK\n");
  }
  //++++++++++++++++++++++BIND+++++++++++++++++++++++++++++

  //++++++++++++++++++++++LISTEN+++++++++++++++++++++++++++++
  //listem pra uma conexão
  if((listen(servidorSocket, 1)) < 0){
    perror("Erro no Listen");
    exit(-1);
  }else{
      printf("Listen OK!\n");
  }
  //++++++++++++++++++++++LISTEN+++++++++++++++++++++++++++++

  printf("Servidor Ligado Esperando clientes...\n");
  //++++++++++++++++++++++ACCEPT+++++++++++++++++++++++++++++
  clienteSocket = accept(servidorSocket, NULL, NULL);
  strcpy(bufferIda,"sOn");
  send(clienteSocket, bufferIda, strlen("sOn"), 0);

	do{
    //clienteSocket = accept(servidorSocket, NULL, NULL);
    tamanho_msg_recebida = recv(clienteSocket, bufferVolta, TAMANHO_MENSAGEM, 0);


    //====Comandos FTP
    if(tamanho_msg_recebida > 0){
      printf("Recebido: %s",bufferVolta);
      bufferVolta[tamanho_msg_recebida] = '\0';


      //Não consigo COmparar essa Caraleas
      if(strcmp("User ricardo", bufferVolta) == 0){
        estado = 1;
        strcpy(bufferIda,"331");
        send(clienteSocket, bufferIda, sizeof(bufferIda), 0);
        printf("Entrou n User\n");
      }

      if(strcmp("PASS ok", bufferVolta) == 0){
        estado = 2;
        strcpy(bufferIda,"332");
        send(clienteSocket, bufferIda, sizeof(bufferIda), 0);
        printf("Entrou n PASS\n");
      }


      //ISSO FUNCIONA
      if('0' == *bufferVolta)
        printf("Teste\n");

      /*
      if(strcmp(bufferVolta,"QUIT")){
        strcpy(bufferIda,"331");
        send(clienteSocket, bufferIda, sizeof(bufferIda), 0);
        close(clienteSocket);
        break;
      }
      */
    }


	}while(1);
  printf("Saiu do while\n");

  close(servidorSocket);
  //fflush(stdout);

  return 0;
}
