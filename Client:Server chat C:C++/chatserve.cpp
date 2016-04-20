//
//  chatserve.cpp
//  
//
//  Created by Pavin Disatapundhu
//
//

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX 500
#define NAMEMAX 10
#define SERV_HOST_ADDR  "127.0.0.1"

int port, SSocket, CSocket;
struct sockaddr_in cli_addr;

//API connection socket
int connectSocket(){
    struct sockaddr_in serv_addr;
    SSocket = socket(PF_INET, SOCK_STREAM, 0);
    if(SSocket == -1) {
        perror("Socket");
        return 0;
    }
    //http://www.beej.us/guide/bgnet/output/html/multipage/perrorman.html
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    //http://www.cas.mcmaster.ca/~qiao/courses/cs3mh3/tutorials/socket.html
    
    
    if(bind(SSocket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("Cannot bind the socket.\n");
        return 0;
    }
    
    if(listen(SSocket, 1) == -1) {
        perror("Cannot listen to port.\n");
        return 0;
    }
    return 1;
}

//sending chat message
int sendChat(char* chat) {
    int len;
    len = strlen(chat);
    if(send(CSocket, chat, len , 0) == -1 ) {
        perror("Send");
        return 0;
    }
    return 1;
}

//server recieve chat promt message
int recChat(char* cHandle, char* chat) {
    printf("Host> ");
    fgets(chat, MAX, stdin);
    while(sendChat(chat) != 1) {
        printf("Resending.\n");
        sleep(3);
    }
    memset(chat, 0, MAX);
    
    return 1;
}

int connectionLoop(char* cHandle, char *chat,char *buffer){
    //connection loop.
    int recieve;
    if(strncmp(SERV_HOST_ADDR, inet_ntoa(cli_addr.sin_addr), strlen(SERV_HOST_ADDR)) == 0) {
        sprintf(cHandle, "Pavin");
        printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n%s has connected to the server. (Max)\n",cHandle);
    }
    strcpy(chat,cHandle);
    strcat(chat,">");
    
    
    //client handle chat messeage
    while(sendChat(chat) != 1) {
        printf("Resending\n");
        sleep(3);
    }
    
    //recieve chat messeges
    while(1) {
        if((recieve = recv(CSocket, buffer, MAX, 0)) == -1) {
            printf("Recieve failed\n");
            exit(1);
        }else if(recieve == 0){
            printf("Connection close\n");
            break;
        }
        printf("\n%s> %s\n",cHandle, buffer);
        
        //check if we need to quit
        if (strncmp(buffer,"\\quit", strlen("\\quit")) == 0){
            printf("Quiting program.\n");
            exit(0);
        }
        memset(buffer, 0, MAX);
        recChat(cHandle, chat);
    }
    close(CSocket);
    return 0;
}

//Signal Handlers int!
void siginthandler(int sig) {
    printf("\nEnding connection...\n");
    exit(0);
    
    //http://www.tutorialspoint.com/c_standard_library/c_function_signal.htm
}


int main(int argc, char** argv) {
    //chatserve starts on host A.

    char* chat;
    char* cHandle;
    char* buffer;
    chat = static_cast<char*>(malloc(MAX * sizeof(char)));
    cHandle = static_cast<char*>(malloc(NAMEMAX * sizeof(char)));
    buffer = static_cast<char*>(malloc(MAX * sizeof(char)));
    unsigned int clientin;
    
    //chatserve on host A waits on a port, check if there a port is entered
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
    clientin = sizeof(cli_addr);
    
    //connection loop
    printf("Waiting for Client...\n");
    
    while(1) {
        //accept the process
        if((CSocket = accept(SSocket,(struct sockaddr*) &cli_addr, &clientin)) == -1) {
            perror("Error on accept\n");
        } //http://www.linuxhowtos.org/C_C++/socket.htm
        
        connectionLoop(cHandle, chat, buffer);
        printf("Client disconnected. \nWaiting for another client ('ctrl-c' to exit)\n");
    }
    
    return 0;
}
