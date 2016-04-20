//
//  Project2: ftpserver.cpp
//
//
//  Created by Pavin Disatapundhu on 03/07/15.
//
//
/* This program takes is the server side program that waits for a client to connect and does one of the following options.
 if the client inputs -l option the server will list all the files in the current directory.
 if the client inputs -g option followed by the file name the server will validate and send the file. */


#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>
#include <dirent.h>
#include <netdb.h>



#define MAX 1024
#define NAMEMAX 20


int connectSocket();
int sendChat(int dsocket, char* chat);
int connectToSocket(int SSocket, char *port, char *serverhost);
void handleRequest(char *client);
void siginthandler(int sig);
int serverList(int socket1,int socket2);
int serverSend(int socket, int socket2, char* filename, char * dataport);
//global var.
int port, SSocket, cSocket;

/* ============================================================================================================
This main funciton take 1 arg
============================================================================================================ */
int main(int argc, char** argv) {
    //chatserve starts on host A.
    unsigned int clientin;
    struct sockaddr_in cli_addr;
    struct hostent *clientName;
    struct in_addr ipv4addr;
    
    //ftserve starts on host A, validates the parameter, and waits on <SERVER_PORT> for client request
    if(argc > 1) {
        sscanf(argv[1], "%d", &port);
    }
    else {
        perror("Please enter a port.\n");
        return 1;
    }
    //singals
    signal(SIGINT, siginthandler);
    
    //API sockets
    if(!connectSocket()) {
        exit(0);
    }
    
    //connection loop
    printf( "Waiting for Client...\n");
    while(1) {
        clientin = sizeof(cli_addr);
        //accept the process
        if((cSocket = accept(SSocket,(struct sockaddr*) &cli_addr, &clientin)) == -1) {
            perror("Error on accept\n");
        } //http://www.linuxhowtos.org/C_C++/socket.htm
        
        //get server name
        inet_pton(AF_INET, inet_ntoa(cli_addr.sin_addr), &ipv4addr);
        clientName = gethostbyaddr(&ipv4addr, sizeof ipv4addr, AF_INET);
        printf("Connection from %s\n", clientName->h_name);

        //http://stackoverflow.com/questions/10236204/how-to-get-the-name-of-the-client-when-receiving-an-http-request
        
        handleRequest(clientName->h_name);
        printf("Disconnecting... \nWaiting for a client... ('ctrl-c' to exit)\n");
    }
    close(SSocket);
    close(cSocket);
    return 0;
}

/* ============================================================================================================
//API connection socket
============================================================================================================ */
int connectSocket() {
    struct sockaddr_in serv_addr;
    
    SSocket = socket(PF_INET, SOCK_STREAM, 0);
    if(SSocket == -1) {
        perror("Socket");
        return 0;
    }
    //http://www.beej.us/guide/bgnet/output/html/multipage/perrorman.html
    
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;//TCP
    serv_addr.sin_port = htons(port);//Port
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);//Fill address
    
    //http://www.cas.mcmaster.ca/~qiao/courses/cs3mh3/tutorials/socket.html
    
    //bind
    if(bind(SSocket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("Cannot bind the socket.\n");
        return 0;
    }
    
    if(listen(SSocket, 1) == -1) {
        perror("Cannot listen to port.\n");
        return 0;
    }
    printf("Server open on %d\n", port);
    return 1;
}

/* ============================================================================================================
//This will establish the initial connection to the sockets
============================================================================================================ */
int connectToSocket(int SSocket, char *port, char *serverhost){
    struct addrinfo data_addr;
    struct addrinfo *serverinfo, *p;
    
    memset(&data_addr, 0, sizeof(data_addr));
    data_addr.ai_family = AF_UNSPEC;//TCP
    data_addr.ai_socktype = SOCK_STREAM;
    
    if(getaddrinfo(serverhost, port, &data_addr, &serverinfo) != 0){
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
//sending chat message
============================================================================================================ */
int sendChat(int dsocket, char* chat) {
    int len = strlen(chat);
    if(send(dsocket, chat, len , 0) == -1 ) {
        perror("Send");
        return 0;
    }
    
    return 1;
}
/* ============================================================================================================
//handleRequest this function will either run the serverList() or serverSend() function.
============================================================================================================ */
void handleRequest(char *client){
//    struct sockaddr_in cli_addr;
    int recieve, k;
    int datasocket;
    char *fname;
    char *option;
    char *dataport;
    fname = static_cast<char*>(malloc(MAX * sizeof(char)));
    dataport = static_cast<char*>(malloc(MAX * sizeof(char)));
    option = static_cast<char*>(malloc(MAX * sizeof(char)));
    
    //ftserve accepts and interprets command
    while(1) {
        //varify the option
        if((recieve = recv(cSocket, option, MAX, 0)) == -1) {
            printf("Recieve failed 1\n");
            exit(1);
        }else if(recieve == 0){
            printf("Connection closed\n");
            break;
        }
        sendChat(cSocket, option);
        //varify the data port
        if((recieve = recv(cSocket, dataport, MAX, 0)) == -1) {
            printf("Recieve failed 2\n");
            exit(1);
        }else if(recieve == 0){
            printf("Connection closed\n");
            break;
        }
        sendChat(cSocket, dataport);
        
        //ftserve and ftclient establish a data connection
        while((datasocket = connectToSocket(datasocket, dataport, client)) == 0){
            printf("Connecting to to data port");
            sleep(3);
        }
        printf("Dataport open on %s.\n", dataport);
        
        //connect to see which option
        if(!strcmp(option,"-l")){
            serverList(datasocket,cSocket);
        }else{
            if((recieve = recv(cSocket, fname, MAX, 0)) == -1) {
                printf("Recieve Failed\n");
                exit(1);
            }else if(recieve == 0){
                printf("Closing connection\n");
                break;
            }
            serverSend(datasocket, cSocket, fname, dataport);
        }
        close(datasocket);
    }
}
/* ============================================================================================================
//server searches dir and list all
============================================================================================================ */
int serverList(int socket,int socket2)
{
    DIR *dirc;
    struct dirent *name;
    int i = 0;
    char data[MAX];
    char server[6] = "flip1";
    sendChat(socket2, server);
    printf("List directory requested on port %d\n", port);
    if ((dirc = opendir(".")) != NULL) {
        while ((name = readdir (dirc)) != NULL) {
            if(i >1){
                strcat(name->d_name,"\n");
                strcat(data,name->d_name);
            }
            i++;
        }
        closedir (dirc);
    }
    if (send(socket, data, strlen(data), 0) < 0){
        perror("error");
    }
    memset(data, 0, MAX);

    //http://stackoverflow.com/questions/15925007/copying-contents-of-a-text-file-into-a-buffer-win32api
//    http://www.cplusplus.com/reference/cstdio/fread/
    //http://www.tutorialspoint.com/c_standard_library/c_function_fread.htm
    //http://stackoverflow.com/questions/612097/how-can-i-get-the-list-of-files-in-a-directory-using-c-or-c
    
    return 0;	
}
/* ============================================================================================================
send the file to the server
============================================================================================================ */
int serverSend(int socket, int socket2, char* filename, char * dataport)
{
    size_t bytesRead;
    char data[MAX];
//    char server[6] = "flip1";
    char valid[4] = "pass";
    char fail[4] = "fail";

    FILE* fd = fopen(filename, "r");
    
    printf("File '%s' requested on port %d\n", filename, port);
    if (fd == NULL) {
        printf("File not found. Sending error message to %s\n" ,dataport);
        sendChat(socket2, fail);
    } else {
        printf("Sending '%s' to %d\n",filename, port);
        sendChat(socket2, valid);
        do {
            bytesRead = fread(data, 1, MAX, fd);
            if (send(socket, data, bytesRead, 0) < 0){
                perror("error sending file\n");
            }
            
        } while (bytesRead > 0);
        printf("Finish sending\n");
    }
    fclose(fd);
    return 0;
}
/* ============================================================================================================

//Signal Handlers int!
============================================================================================================ */
void siginthandler(int sig) {
    printf("\nEnding connection...\n");
    exit(0);
}
    //http://www.tutorialspoint.com/c_standard_library/c_function_signal.htm

