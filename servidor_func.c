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


//inclui a biblioteca lib.h
#include"lib.h"

char data[BUF_SIZE] = "";
int meio, novoMeio;

//Função que inicial as variaveis do sockaddr_in, onde:
//     sa      -   é p ponteiro para a estrutura sockaddr_in
//     port    -   é o número da porta que o servidor ira ouvir
void iniciaSocket(struct sockaddr_in * sa, char * ip_addr, unsigned int port){
    memset(sa, 0, sizeof(struct sockaddr_in));
    sa->sin_family = AF_INET;
    sa->sin_port = htons(port);
    if (ip_addr == NULL)
        sa->sin_addr.s_addr = htonl(INADDR_ANY);
    else
        sa->sin_addr.s_addr = inet_addr(ip_addr);
}

// Função que cria um socket TCP
int criarSocket(struct sockaddr_in * local){
    int sd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sd == -1){
        fprintf(stderr, "Erro ao criar o socket\n");
        return -1;
    }
    int on = 1;
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))){
        perror("Erro no Setseckopt.");
        return -2;
    }
    if(bind(sd, (struct sockaddr *)local, sizeof(struct sockaddr_in)) == -1){
        fprintf(stderr, "Erro no bind\n");
        close(sd);
        return -3;
    }
    return sd;
}

// Função que envia as mensagens na conexão de controle, aonde:
//     cd         -   é o descritor da coneção (a mensagem será enviada para esta conexão)
//     msg_num    -   é o numero da mensagem que será enviada
void mensagem(int msg_num, int cd){
    static char message[BUF_SIZE + 1] = "";
    switch(msg_num){
        case 150:
            strncpy(message, "150 data is coming.\r\n", BUF_SIZE);
            break;
        case 200:
            strncpy(message, "200 command successful.\r\n", BUF_SIZE);
            break;
        case 202:
            strncpy(message, "202 command is not implemented.\r\n", BUF_SIZE);
            break;
        case 220:
            strncpy(message, "220 my FTP server.\r\n", BUF_SIZE);
            break;
        case 221:
            strncpy(message, "221 Goodbye.\r\n", BUF_SIZE);
            break;
        case 215:
            strncpy(message, "215 UNIX Type: L8\r\n", BUF_SIZE);
            break;
        case 226:
            strncpy(message, "226 data sent OK.\r\n", BUF_SIZE);
            break;
        case 230:
            strncpy(message, "230 Login successful.\r\n", BUF_SIZE);
            break;
        case 331:
            strncpy(message, "331 Please specify the password.\r\n", BUF_SIZE);
            break;
        case 451:
            strncpy(message, "451 Requested action aborted: local error in processing\r\n", BUF_SIZE);
            break;
        case 504:
            strncpy(message, "504 Command not implemented for that parameter\r\n", BUF_SIZE);
            break;
        case 5301:
            strncpy(message, "530 Login incorrect.\r\n", BUF_SIZE);
            break;
        case 5302:
            strncpy(message, "530 Please login with USER.\r\n", BUF_SIZE);
            break;
        case 550:
            strncpy(message, "550 Failed to open or create file.\r\n", BUF_SIZE);
            break;
        default:
            // Mensagens desconhecida
            return;
    }
    write(cd, message, strlen(message));
}


//Função de autencicacao aceita qualquer logui e senha
//Comandos USER e PASS
int autencicacao(int cd){
    char buf[BUF_SIZE] = "";

    read(cd, buf, BUF_SIZE - 1);
    if (strncmp(buf, "USER ", 5) == 0){
        mensagem(230, cd);
        return 0;
        mensagem(331, cd);
        read(cd, buf, BUF_SIZE - 1);
        if (strncmp(buf, "PASS ", 5) == 0){
             mensagem(230, cd);
            return 0;
        }
        else{
             mensagem(5301, cd);
             return 1;
         }
    }
    mensagem(5302, cd);
    return 2;
}

//Função que encerra conexão atual, se der zebra encerra o programa. Aonde:
//     cd   -   é o descritor da coneção
//     sd   -   é o socket
void encerrarConexao(int cd, int sd){
    if (shutdown(cd, SHUT_RDWR) == -1){
        fprintf(stderr, "O encerramento falhou.");
        close(cd);
        close(sd);
        exit(EXIT_FAILURE);
    }
    close(cd);
}

// Função de parsering analisa o parâmetro do comando port e extrair o ip e porta
// que o cliente está escutando, aonde:
//     ca   -   é o ponteiro do sockaddr_in variavel que a função principal tem acesso.
//              o resultado vai atualizar o endereço.
//     buf  -   buffer que guarda os parâmetro
int port_parser(char * buf, struct sockaddr_in * ca){
    unsigned int i = 0;
    unsigned int port_high, port_low;
    int length = strlen(buf);
    if (buf){
        for(int c = 0; i < length; i++){
            if (buf[i] == ','){
                c++;
                buf[i] = '.';
                if (c == 4){         // ipv4
                    buf[i] = '\0'; // mark the end of the ip address
                    i++;            // i will be the start of the port
                    break;
                }
            }
        }
    }
    sscanf(buf + i, "%d,%d", &port_high, &port_low);
    iniciaSocket(ca, buf + 5, (port_high << 8) | port_low);
    printf("%s\n", inet_ntoa(ca->sin_addr));
    return QoS(inet_ntoa(ca->sin_addr));
}


//Função que cria a conexão de dados para a tranferencia de arquivos
// A função cria um novo socket e faz o bind com o ip e porta local  e conecta com os dados guardados em ca (parsering)
int criarConexaoDados(const struct sockaddr_in * ca){
    struct sockaddr_in sa;
    iniciaSocket(&sa, NULL, DATA_PORT);

    int sd = criarSocket(&sa);
    if (sd < 0){
      perror("Não foi possivel criar o socke da conexão de dados.");
      exit(EXIT_FAILURE);
    }
    if (connect(sd, (struct sockaddr *)ca, sizeof(struct sockaddr_in)) == -1){
      fprintf(stderr, "A conexão falhou.");
      close(sd);
      exit(EXIT_FAILURE);
    }
    return sd;
}

//Função list que basicamente executa o comando linux ls-l
//      Cria uma conexão de dados e mando o resultado do comando linux ls -l

//      ca      -   client address, storing ip and port we will use the create
//                  the data channel socket
//      folder  -   the parameter of the 'ls -l' command
void list(const struct sockaddr_in * ca, char * folder){

    char cmd[BUF_SIZE] = "ls -l ";
    data[BUF_SIZE] = "";
    int sd = criarConexaoDados(ca);

    // append the parameter if needed.
    if(folder)
        strncat(cmd, folder, BUF_SIZE - strlen(cmd) - 1);

    FILE * fp = popen(cmd, "r");
    int length = 0;
    if (fp == NULL){
        fprintf(stderr, "Falha ao executar o comando list.\n" );
        exit(1);
    }

    // captura o resultado do comando ls -l
    while (fgets(data, sizeof(data)-1, fp) != NULL){
        length = strlen(data);
        strncpy(data + length - 1, "\r\n", 3);
        write(sd, data, strlen(data));
    }
    // fecha o arquivo
    pclose(fp);
    shutdown(sd, SHUT_RDWR);
    close(sd);
}


//Função que implementa o comando FTP RETR para enviar aquivo para o cliente
//      ca      -   client address, storing ip and port we will use the create
//                  the data channel socket
//      file    -   the name of the file that the client is trying to get
// RETURN:
//      0       -   success
//      1       -   file's name is is NULL
//      2       -   cannot find the file
int retr(int cd, const struct sockaddr_in * ca, char * file, int velocidade){
    if(file == NULL){
        fprintf(stderr, "Nome do arquivo não enviado.\n");
        return 1;
    }
    FILE * fp = fopen(file, "rb");
    if (fp == NULL){
        fprintf(stderr, "Não foi possivel encontar o arquivo %s.\n", file);
        return 2;
    }

    mensagem(150, cd);

    data[BUF_SIZE] = "";
    int sd = criarConexaoDados(ca);

    // length equals to the bytes read from the file.
    int length = fread(data, 1, BUF_SIZE, fp);

    meio = length/2;
    novoMeio = (meio-BUF_SIZE);
    //data2 = data[meio];


    struct arg_struct args1, args2;
    args1.tipo = 1;
    args1.fp = fp;
    args1.length = length;
    args1.sd = sd;
    args1.vel = velocidade;

    args2.tipo = 2;
    args2.fp = fp;
    args2.length = length;
    args2.sd = sd;
    args2.vel = velocidade;

    pthread_t t1;
    //cria as threads
    pthread_create(&t1, NULL, enviarThread, (void*)&args1);
    pthread_t t2;
    pthread_create(&t2, NULL, enviarThread, (void*)&args2);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);



    fclose(fp);
    shutdown(sd, SHUT_RDWR);
    close(sd);
    return 0;
}


// Função que implementa o comando FTP STOR utilizado para recer um arquivo do cliente
//      ca      -   client address, storing ip and port we will use the create
//                  the data channel socket
//      file    -   the name of the file that the client is trying to store
// RETURN:
//      0       -   success
//      1       -   file's name is is NULL
//      2       -   cannot find the file
int stor(int cd, const struct sockaddr_in * ca, char * file, int velocidade){
  if(file == NULL){
      fprintf(stderr, "Nome do arquivo não enviado.\n");
      return 1;
  }
  FILE * fp = fopen(file, "wb");
  if (fp == NULL){
      fprintf(stderr, "Não foi possivel salvar o arquivo %s.\n", file);
      return 2;
  }

  mensagem(150, cd);

  char data[BUF_SIZE] = "";
  int sd = criarConexaoDados(ca);
  int length = read(sd, data, BUF_SIZE);
  while(length > 0){
      usleep(velocidade);
      fwrite(data, 1, length, fp);
      length = read(sd, data, BUF_SIZE);
  }
  fclose(fp);
  shutdown(sd, SHUT_RDWR);
  close(sd);
  return 0;
}

/*
//Funcionando!!!
void *receberThread(void *arg){
  struct arg_struct *args = (struct arg_struct *)arg;
  FILE * fp = args->fp;
  int length = args->length;
  int sd = args->sd;
  //int meio = (length/2);
  int tipo = args->tipo;

  printf("%d\n", tipo);

  if(tipo == 1){
    printf("Entrou na thread 1\n");
    while(length > meio){
        fwrite(data, 1, length, fp);
        length = read(sd, data, BUF_SIZE);
    }
  }else{
    printf("Entrou na thread 2\n");
    while(novoMeio > 0){
        fwrite(data, 1, novoMeio, fp);
        novoMeio = read(sd, data, BUF_SIZE);
    }
  }

}
*/

void *enviarThread(void *arg){
  struct arg_struct *args = (struct arg_struct *)arg;
  FILE * fp = args->fp;
  int length = args->length;
  int sd = args->sd;
  //int meio = (length/2);
  int tipo = args->tipo;
  int velocidade = args->vel;

  printf("%d\n", tipo);

  if(tipo == 1){
    printf("Entrou na thread 1\n");
    while(length > meio){
        usleep(velocidade);
        write(sd, data, length);
        length = fread(data, 1, BUF_SIZE, fp);
    }
  }else{
    printf("Entrou na thread 2\n");
    while(novoMeio > 0){
        usleep(velocidade);
        write(sd, data, novoMeio);
        novoMeio = fread(data, 1, BUF_SIZE, fp);
    }
  }

}

int QoS(char ipCliente[]){
    int velocidade = 11;
    char linhaArquivo[20];
    FILE *arq;
    arq = fopen("qos.txt", "r");

    if(arq == NULL)
        printf("Erro, nao foi possivel abrir o arquivo\n");

    while( (fgets(linhaArquivo, sizeof(linhaArquivo), arq))!=NULL ){
      int len = strlen(linhaArquivo);
      if (len == 0) { /* normalmente isto nunca acontece */ }
      /* alguns "ficheiros de texto" nao tem '\n' na ultima linha */
      if (linhaArquivo[len - 1] == '\n') linhaArquivo[--len] = 0; // remove '\n' e actualiza len
      //printf("%s\n", linhaArquivo);
      //printf("%s\n", inet_ntoa(ca->sin_addr));
      //printf("Igual: %d\n", strcmp(inet_ntoa(ca->sin_addr), linhaArquivo));
      //printf("Igual: %d\n", strcmp(inet_ntoa(ca->sin_addr), "0.0.0.0"));
      if( strncmp( ipCliente, linhaArquivo, sizeof(linhaArquivo) ) == 0){
        printf("Cliente: %s\n", linhaArquivo);
        fgets(linhaArquivo, sizeof(linhaArquivo), arq);
        velocidade = atoi(linhaArquivo);
        printf("Velocidae: %d KBYtes/s\n", velocidade);
        break;
      }
    }

    fclose(arq);
    printf("Tempo do Sleep: %d\n", (BUF_SIZE*1000)/velocidade );
    return velocidade = (BUF_SIZE*1000)/velocidade;

}
