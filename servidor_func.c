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

// FUNCTION: void messenger(int msg_num, int cd)
// DESCRIPTION:
//     based on the provided message number and connection descriptor, send
//     the message.
// PARAMETER:
//     cd         -   connection descriptor
//     msg_num    -   message number
// RETURN:
//     void
void messenger(int msg_num, int cd)
{
    static char message[BUF_SIZE + 1] = "";
    switch(msg_num)
    {
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
            // Unsupported message code. Do nothing.
            return;
    }
    write(cd, message, strlen(message));
}


// FUNCTION: int auth(int cd)
// DESCRIPTION:
//     controlling the authentication of the login.
// PARAMETER:
//     cd   -   connection descriptor
// RETURN:
//     0    -   success
//     1    -   password is needed
//     2    -   username is needed

int auth(int cd)
{
    char buf[BUF_SIZE] = "";

    read(cd, buf, BUF_SIZE - 1);
    if (strncmp(buf, "USER ", 5) == 0)
    {
        messenger(230, cd);
        return 0;
//         messenger(331, cd);
//         read(cd, buf, BUF_SIZE - 1);
//         if (strncmp(buf, "PASS ", 5) == 0)
//         {
//             messenger(230, cd);
//             return 0;
//         }
//         else
//         {
//             messenger(5301, cd);
//             return 1;
//         }
    }
    messenger(5302, cd);
    return 2;
}

// FUNCTION: void shutdown_connection(int cd, int sd)
// DESCRIPTION:
//     shutdown the current connection. If any error is encountered, the
//     whole program will exit.
// PARAMETER:
//     cd   -   connection descriptor
//     sd   -   socket descriptor
// RETURN:
//     void
void shutdown_connection(int cd, int sd)
{
    if (shutdown(cd, SHUT_RDWR) == -1)
    {
        fprintf(stderr, "shutdown failed");
        close(cd);
        close(sd);
        exit(EXIT_FAILURE);
    }
    close(cd);
}

// FUNCTION: void port_parser(char * buf, struct sockaddr_in * ca)
// DESCRIPTION:
//    parse the parameter of the port command and extract the ip and port
//    that the client is listening.
// PARAMETER:
//     ca   -   pointer to the sockaddr_in variable that the main function
//              keeps tracking. The result will be updated into this address.
//     buf  -   buffer storing the parameters
// RETURN:
//     void

void port_parser(char * buf, struct sockaddr_in * ca)
{
    unsigned int i = 0;
    unsigned int port_high, port_low;
    int length = strlen(buf);
    if (buf)
    {
        for(int c = 0; i < length; i++)
        {
            if (buf[i] == ',')
            {
                c++;
                buf[i] = '.';
                if (c == 4)         // ipv4
                {
                    buf[i] = '\0'; // mark the end of the ip address
                    i++;            // i will be the start of the port
                    break;
                }
            }
        }
        sscanf(buf + i, "%d,%d", &port_high, &port_low);
        iniciaSocket(ca, buf + 5, (port_high << 8) | port_low);
    }
}
// FUNCTION: int connect_data_channel(const struct sockaddr_in * ca)
// DESCRIPTION:
//      This function will create a new socket binding with the local ip and
//      port, and connect to the ip and port stored in parameter ca.
// PARAMETERS:
//      ca  -   client address, storing ip and port we will connect to
// RETURN:
//     socket id

int create_data_channel(const struct sockaddr_in * ca)
{
    struct sockaddr_in sa;
    iniciaSocket(&sa, NULL, DATA_PORT);

    int sd = criarSocket(&sa);
    if (sd < 0)
    {
      perror("cannot create socket");
      exit(EXIT_FAILURE);
    }
    if (connect(sd, (struct sockaddr *)ca, sizeof(struct sockaddr_in)) == -1)
    {
      fprintf(stderr, "connect failed");
      close(sd);
      exit(EXIT_FAILURE);
    }
    return sd;
}
// FUNCTION: void list(const struct sockaddr_in * ca, char * folder)
// DESCRIPTION:
//      worker function for the list command. It will create a data_channel
//      and send the output of 'ls -l' through the channel to the client.
// PARAMETERS:
//      ca      -   client address, storing ip and port we will use the create
//                  the data channel socket
//      folder  -   the parameter of the 'ls -l' command
// RETURN:
//     void

void list(const struct sockaddr_in * ca, char * folder)
{

    char cmd[BUF_SIZE] = "ls -l ";
    char data[BUF_SIZE] = "";
    int sd = create_data_channel(ca);

    // append the parameter if needed.
    if(folder)
        strncat(cmd, folder, BUF_SIZE - strlen(cmd) - 1);

    FILE * fp = popen(cmd, "r");
    int length = 0;
    if (fp == NULL)
    {
        fprintf(stderr, "Failed to run command\n" );
        exit(1);
    }

    // collect output from ls command
    while (fgets(data, sizeof(data)-1, fp) != NULL)
    {
        length = strlen(data);
        // replace the "\n" with "\r\n". size 3 guarantees the '\0' is
        // appended.
        strncpy(data + length - 1, "\r\n", 3);
        write(sd, data, strlen(data));
    }
    // close FILE pointer and the socket:
    pclose(fp);
    shutdown(sd, SHUT_RDWR);
    close(sd);
}


// FUNCTION: int retr(int cd, const struct sockaddr_in * ca, char * file)
// DESCRIPTION:
//      worker function for the retr command.
// PARAMETERS:
//      ca      -   client address, storing ip and port we will use the create
//                  the data channel socket
//      file    -   the name of the file that the client is trying to get
// RETURN:
//      0       -   success
//      1       -   file's name is is NULL
//      2       -   cannot find the file

int retr(int cd, const struct sockaddr_in * ca, char * file)
{
    if(file == NULL)
    {
        fprintf(stderr, "file name is NULL.\n");
        return 1;
    }
    FILE * fp = fopen(file, "rb");
    if (fp == NULL)
    {
        fprintf(stderr, "Cannot find the file %s.\n", file);
        return 2;
    }

    messenger(150, cd);

    char data[BUF_SIZE] = "";
    int sd = create_data_channel(ca);

    // length equals to the bytes read from the file.
    int length = fread(data, 1, BUF_SIZE, fp);
    while(length > 0)
    {
        write(sd, data, length);
        length = fread(data, 1, BUF_SIZE, fp);
    }
    fclose(fp);
    shutdown(sd, SHUT_RDWR);
    close(sd);
    return 0;
}
// FUNCTION: int stor(int cd, const struct sockaddr_in * ca, char * file)
// DESCRIPTION:
//      worker function for the stor command.
// PARAMETERS:
//      ca      -   client address, storing ip and port we will use the create
//                  the data channel socket
//      file    -   the name of the file that the client is trying to store
// RETURN:
//      0       -   success
//      1       -   file's name is is NULL
//      2       -   cannot find the file

int stor(int cd, const struct sockaddr_in * ca, char * file)
{
    if(file == NULL)
    {
        fprintf(stderr, "file name is NULL.\n");
        return 1;
    }
    FILE * fp = fopen(file, "wb");
    if (fp == NULL)
    {
        fprintf(stderr, "Cannot create the file %s.\n", file);
        return 2;
    }

    messenger(150, cd);

    char data[BUF_SIZE] = "";
    int sd = create_data_channel(ca);
    int length = read(sd, data, BUF_SIZE);
    while(length > 0)
    {
        fwrite(data, 1, length, fp);
        length = read(sd, data, BUF_SIZE);
    }
    fclose(fp);
    shutdown(sd, SHUT_RDWR);
    close(sd);
    return 0;
}
