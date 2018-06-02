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

//Definição das bibliotecas utilizadas
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h> //para threads

//Definição das constantes utilizadas
#define BUF_SIZE 128        // tamanho do buffer
#define MAX_CONNECTIONS 4  // numero maximo de coneções
#define DATA_PORT 2000      // numero da porta da conexão de dados
#define PORTA 2121          // porta que o servidor escuta

//char data[BUF_SIZE] = "";

struct arg_struct{
  int tipo;
  int length;
  int sd;
  FILE * fp;
};

//Assinatura das funções
void iniciaSocket(struct sockaddr_in * sa, char * ip_addr, unsigned int port);
int criarSocket(struct sockaddr_in * local);
void mensagem(int msg_num, int cd);
int autencicacao(int cd);
void encerrarConexao(int cd, int sd);
void port_parser(char * buf, struct sockaddr_in * ca);
int criarConexaoDados(const struct sockaddr_in * ca);
void list(const struct sockaddr_in * ca, char * folder);
int retr(int cd, const struct sockaddr_in * ca, char * file);
int stor(int cd, const struct sockaddr_in * ca, char * file);

//threads
//void *receberThread(void *arg); //put

void *enviarThread(void *arg); //get
