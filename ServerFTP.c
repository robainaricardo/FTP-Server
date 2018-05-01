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

//Passar para o .h
void quit();
void user();
void pass();
void comando_desconhecido();


int servidorSocket; //Socket do servidor
int clienteSocket; //Socket do cliente conectado ao servidor

int estado = 0;

int tamanho_msg_recebida;

char bufferIda[5];
char bufferVolta[TAMANHO_MENSAGEM];

struct sockaddr_in servidor; //Estrutura que mantem os dados do socket após o bind (Ip e porta)
struct sockaddr_in cliente;

int main(){



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

  //loop para manter o servidor online quando um cliente da QUIT
  do{
    printf("+++++++++++++++++++++++++++++++++++++++\n");
    printf("Servidor Liberado Esperando clientes...\n");
    printf("+++++++++++++++++++++++++++++++++++++++\n");
    //++++++++++++++++++++++ACCEPT+++++++++++++++++++++++++++++
    clienteSocket = accept(servidorSocket, NULL, NULL);
    strcpy(bufferIda,"sOn");
    send(clienteSocket, bufferIda, strlen("sOn"), 0);

  	do{ //Loop do Send/recive
      //clienteSocket = accept(servidorSocket, NULL, NULL);
      tamanho_msg_recebida = recv(clienteSocket, bufferVolta, TAMANHO_MENSAGEM, 0);


      //====Comandos FTP
      if(tamanho_msg_recebida > 0){
        printf("Recebido: %s",bufferVolta);
        bufferVolta[tamanho_msg_recebida] = '\0';

        //pega o comando da mensagem
        char *comando = strtok(bufferVolta, " ");

        //Aqui vai os condicionais com os comandos e chama as funções dos comandos
        if(strcasecmp(comando, "USER") == 0){
          user();
        }else if(strcasecmp(comando, "PASS") == 0){
          pass();
        }else if(strcasecmp(comando, "QUIT") == 0){
          quit();
          break;
        }else{
          comando_desconhecido();
        }





      }
  	}while(1);
  }while (1);


  close(servidorSocket);
  //fflush(stdout);

  return 0;
}


//Funções dois colocar em outro arquivo

void quit(){
  estado = 0; //esperando cliente
  strcpy(bufferIda,"211\n");
  send(clienteSocket, bufferIda, sizeof(bufferIda), 0);
  printf("Coneceção encerrada com o cliente %d\n", clienteSocket);
  close(clienteSocket);
}

void user(){
  estado = 1; // esperando senha
  strcpy(bufferIda,"331\n");
  send(clienteSocket, bufferIda, sizeof(bufferIda), 0);
}

void pass(){
  estado = 2; // usuário logado
  strcpy(bufferIda,"230\n");
  send(clienteSocket, bufferIda, sizeof(bufferIda), 0);
}

void comando_desconhecido(){
  strcpy(bufferIda,"502\n");
  send(clienteSocket, bufferIda, sizeof(bufferIda), 0);
}
