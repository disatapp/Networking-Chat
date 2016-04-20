//
//  clientside.c
//
//
//  Created by Pavin Disatapundhu
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>


#define MAX 500
#define NAMEMAX 10
#define TIMEOUT 200

int port, SSocket;
struct sockaddr_in serv_addr;
time_t timer;

//http://www.thegeekstuff.com/2011/12/c-socket-programming/

//TCP connect
int connectedSocket(){
    SSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if((SSocket) < 0) {
        perror("Error creating the server socket.\n");
        return 0;
    }
    
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    return 1;
}

//alarm time out if connect takes too long
void sigalrmhandler(int sig) {
    if((time(NULL) - timer) > 280) {
        printf("Timeout\n");
        exit(0);
    }
    
    alarm(2);
}

//Signal Handlers int!
void siginthandler(int sig) {
    printf("\nEnding connection...\n");
    exit(0);
    
    //http://www.tutorialspoint.com/c_standard_library/c_function_signal.htm
}

//recieve current message
int chatServer(char* handle) {
    int recieve;
    recieve = recv(SSocket, handle, NAMEMAX, 0);
    while(recieve == -1) {
        perror("recv:");
    }
    
    if(recieve == 0) {
        perror("recv:");
        return 0;
    }
    return 1;
}

//talking to server this loop is the loop that makes sure ever chating can happend.
int talkToServer(char *handle, char *chat, char * buffer) {
    int recieve;
    timer = time(NULL);
    alarm(2);
    while(1) {
        //input from user
        printf("\n%s ", handle);
        fgets(chat, MAX, stdin);
        if(send(SSocket, chat, strlen(chat), 0) == -1) {
            perror("send:");
        }
        
        memset(chat, 0, MAX);
        
        //recieve
        recieve = recv(SSocket, buffer, MAX, 0);
        if(recieve <= 0) {
            perror("recv:");
            return 0;
        }
        printf("\nHost> %s", buffer);
        timer = time(NULL);
        //check if we need to quit
        if (strncmp(buffer,"\\quit",strlen("\\quit")) == 0){
            printf("Quiting program.\n");
            exit(0);
            
        }
        memset(buffer, 0, MAX);
    }
    close(SSocket);
    return 0;
}


int main(int argc, char** argv) {
    
    char* handle;
    char* chat;
    char* buffer;
    char* con_addr;
    chat = malloc(MAX * sizeof(char));
    buffer = malloc(MAX * sizeof(char));
    handle = malloc(NAMEMAX * sizeof(char));
    
    //check arg
    if(argc != 3) {
        printf("Missing arguments\n");
        exit(0);
    }
    signal(SIGALRM, sigalrmhandler);
    signal(SIGINT, siginthandler);
    
    con_addr = argv[1];
    sscanf(argv[2], "%d", &port);
    
    if(!connectedSocket()) {
        printf("socket issue\n");
        exit(0);
    }
    
    if(connect(SSocket,(struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1 ) {
        perror("connect");
        return 1;
    }
    printf("Connected!\n");
    
    if(chatServer(handle) == 0) {
        return 1;
    }
    //connection
    printf("Successfully connection. Max messege length is %d chars.\n", MAX);
    talkToServer(handle, chat, buffer);
    
    return 0;
}