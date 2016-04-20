//
//  Project2: ftclient.c
//
//
//  Created by Pavin Disatapundhu on 03/07/15.
//
//
/* This program takes is the client side program that waits for a client to connect and does one of the following options.
 if the client inputs -l option the server will list all the files in the current directory.
 if the client inputs -g option followed by the file name the server will validate and send the file. */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>

#define MAX 500
#define NAMEMAX 10
#define FILESIZE 1024

int count(char *buffer);
void sendChat(int SSocket, char* chat);
int connectToSocket(int SSocket, char *port, char *serverhost);
int connectSocket(int SSocket, int port);
int clientTranfer(int SSocket, int BSocket, char* filename, int port, int port2, char * hserver);
int clientList(int SSocket, int BSocket, int port, char * hserver);

struct command{
    char *serverhost;
    int serverport;
    char *cserverport;
    char *option;
    int dataport;
    char *cdataport;
    char *filename;
};

/* ============================================================================================================
 The main function takes up to 5 arguments the first argument is the takes the server name, the second thats the connection port, the third is the option -l or -g, the forth and fifth being the textname is "-g" is selected and the last argument is the dataport"
============================================================================================================ */
int main(int argc, char * argv[]){
//    char *chat; //sting that is send to server
    struct command cmd;
    struct sockaddr_in cli_addr;
    
    int DSocket, CSocket, datasocket;
    int flag = 0;
    unsigned int clientin;
    char *buffer;
    char url[40];
    char *orst = ".engr.oregonstate.edu";
    buffer = malloc(MAX * sizeof(char));
        //check arg
    if(argc < 5) {
        printf("Missing arguments\n");
        exit(1);
    }
    //assign serverport
    if ((cmd.serverport = atoi(argv[2])) <= 0)
    {
        printf("Please enter a valid serverport");
        //usagef();
        exit(0);
    }
    //assign host and option
    cmd.serverhost = argv[1];
    strcpy(url, argv[1]);
    strcat(url, orst);

    cmd.option = argv[3];
    cmd.cserverport = argv[2];
   
    if (!strcmp(cmd.option, "-l")){
        if(argc != 5){
            printf("Usage: ./ftclient [name] [connection port] -l [dataport]");
            exit(0);
        }
        flag = 1;
        cmd.cdataport =argv[4];
        cmd.dataport = atoi(argv[4]);
    }
    else if(!strcmp(cmd.option,"-g")){
        if(argc != 6){
            printf("Usage: ./ftclient [name] [connection port] -g [filename] [dataport]");
            exit(0);
        }
        flag = 2;
        cmd.filename = argv[4];
        cmd.cdataport = argv[5];
        cmd.dataport = atoi(argv[5]);
    }
    else {
        printf("Please enter a option");
        exit(0);
    }
    if (cmd.dataport <= 0)
    {
        printf("Please enter a valid serverport");
        exit(0);
    }

    //function for list
    if(!(CSocket = connectToSocket(CSocket, cmd.cserverport, url))) {
        printf("socket issue\n");
        exit(0);
    }
    
    sendChat(CSocket,cmd.option);
    if (recv(CSocket, cmd.option, sizeof(cmd.option), 0) == -1) {
        perror("Error retriving data from server\n");
        return -1;
    }
    
    sendChat(CSocket ,cmd.cdataport);
    if (recv(CSocket, cmd.cdataport, sizeof(cmd.cdataport), 0) == -1) {
        perror("Error retriving data from server\n");
        return -1;
    }
    
    if(!(DSocket = connectSocket(DSocket, cmd.dataport))) {
        printf("socket issue\n");
        exit(0);
    }
    clientin = sizeof(cli_addr);
    
    while (1){
        if((datasocket = accept(DSocket,(struct sockaddr*) &cli_addr, &clientin)) == -1) {
            perror("Error on accept\n");
            exit(0);
        } //http://www.linuxhowtos.org/C_C++/socket.htm
        close(DSocket);
        
        if(flag == 1){
            clientList(CSocket, datasocket, cmd.dataport, cmd.serverhost);
        }
        else if(flag == 2){
            sendChat(CSocket,cmd.filename);
            clientTranfer(CSocket,datasocket,cmd.filename,cmd.dataport, cmd.serverport, cmd.serverhost);
        }
        close(CSocket);
        close(datasocket);
    return 0;
    }
}

/* ============================================================================================================
this function is used for sending chat message to the server
============================================================================================================ */
void sendChat(int SSocket,char* chat) {
    int len = strlen(chat);
    if(send(SSocket, chat, len , 0) == -1 ) {
        perror("Send");
        exit(0);
    }
}

/* ============================================================================================================
This will establish the initial connection to the sockets
============================================================================================================ */
int connectToSocket(int SSocket, char *port, char *serverhost){
    struct addrinfo serv_addr;
    struct addrinfo *serverinfo, *p;
    
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.ai_family = AF_UNSPEC;//TCP
    serv_addr.ai_socktype = SOCK_STREAM;
    
    if(getaddrinfo(serverhost, port, &serv_addr, &serverinfo) != 0){
        perror("Address not found");
        return 0;
    }
    //http://www.cas.mcmaster.ca/~qiao/courses/cs3mh3/tutorials/socket.html
    
    for(p = serverinfo; p != NULL; p = p->ai_next) {
        SSocket = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if(SSocket == -1) {
            perror("Socket");
            return 0;
        }

        if(connect(SSocket, p->ai_addr, p->ai_addrlen) == -1 ) {
            perror("connect");
            return 0;
        }
        break;
    }
    if (p == NULL) {
        fprintf(stderr, "failed to connect\n");
        exit(2);
    }
    //http://www.beej.us/guide/bgnet/output/html/multipage/getaddrinfoman.html
    
    return SSocket;
}
/* ============================================================================================================
 //Create the data connection socket
 ============================================================================================================ */
int connectSocket(int SSocket, int port) {
    struct sockaddr_in data_addr;
    
    SSocket = socket(PF_INET, SOCK_STREAM, 0);
    if(SSocket == -1) {
        perror("Socket");
        return 0;
    }
    //http://www.beej.us/guide/bgnet/output/html/multipage/perrorman.html
    
    
    memset(&data_addr, 0, sizeof(data_addr));
    data_addr.sin_family = AF_INET;//TCP
    data_addr.sin_port = htons(port);//Port
    data_addr.sin_addr.s_addr = htonl(INADDR_ANY);//Fill local
    //http://www.cas.mcmaster.ca/~qiao/courses/cs3mh3/tutorials/socket.html
    
    //bind
    if(bind(SSocket, (struct sockaddr*)&data_addr, sizeof(data_addr)) == -1) {
        perror("Cannot bind the socket.\n");
        return 0;
    }
    
    if(listen(SSocket, 1) == -1) {
        perror("Cannot listen to port.\n");
        return 0;
    }
    return SSocket;
}
/* ============================================================================================================
 //transferfile to client this function is called if -g option is choosens
 ============================================================================================================ */
int clientTranfer(int SSocket, int BSocket, char* filename, int port, int port2, char * hserver)
{
    char fname[FILESIZE];
    char validate[4];
    int bytesRecieve;
    
   
    
     printf("Receiving '%s' from %s:%d\n",filename,hserver,port);
    //validates the file
    if (recv(SSocket, validate, sizeof(validate), 0) == -1) {
        perror("Error retriving data from server\n");
        return -1;
    }
    printf("dataSock:%s\n", validate);
    if(!strcmp(validate, "pass")){
        FILE* fd = fopen(filename, "w");
        printf("Receiving '%s' from :%d\n",filename,port);
        while ((bytesRecieve = recv(BSocket, fname, FILESIZE, 0)) > 0) {
            fwrite(fname, 1, bytesRecieve, fd);
        }

        if (bytesRecieve < 0){
            return -1;
        }
        printf("File transfer complete.\n");
        fclose(fd);
    }
    else {
        printf("%s:%d says FILE NOT FOUND.\n",hserver,port2);
        return -1;
    }
    
    return 0;	
}
//http://stackoverflow.com/questions/10806754/send-and-receive-int-c
//http://stackoverflow.com/questions/20332622/file-transfer-using-thread
/* ============================================================================================================
 //list folders
 ============================================================================================================ */
int clientList(int SSocket, int BSocket, int port, char * hserver)
{
    char fname[FILESIZE];
    char sname[FILESIZE];
    int byteRecieve;
    
    // Wait for server
    if (recv(SSocket, sname, FILESIZE, 0) == -1) {
        perror("Error retriving data from server\n");
        return -1;
    }
    printf("Receiving directory structure from %s:%d\n",hserver,port);
    
    memset(fname, 0, sizeof(fname));
    
    //recieve the list
    while ((byteRecieve = recv(BSocket, fname, FILESIZE, 0)) > 0) {
        printf("%s", fname);
        memset(fname, 0, sizeof(fname));
    }
    if (byteRecieve < 0){
        perror("error");
    }
    return 0;
}
