#include <stdlib.h>
#include <stdio.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

void Listen(int sockfd){
    if ( listen(sockfd, 5) != 0){
        perror("Error on listening\n");
        exit(1);
    }

}

void makeSockaddr(struct sockaddr_in* struct_address, char* addr, int port, int *len ){
    struct_address->sin_family = AF_INET;
    struct_address->sin_addr.s_addr = inet_addr(addr);
    struct_address->sin_port=port;
    (*len) = (socklen_t)sizeof(*struct_address);
}

void Socket(int *server_sockfd){
    if ( ( (*server_sockfd) = socket(AF_INET, SOCK_STREAM, 0) ) < 0 ){
            perror("Error on socket!\n");
            exit(1);
        }
}

void Bind(int sockfd, struct sockaddr_in* address ,int len){
    if ( bind(sockfd, (struct sockaddr*)address, len) != 0 ){
        perror("Error on binding\n");
        exit(1);
    }
}

void Connect(int sockfd, struct sockaddr_in * address,int len){
    if ( connect(sockfd, (struct sockaddr*)address, len ) != 0){
        perror("Error on connect\n");
        exit(1);
    }
}
