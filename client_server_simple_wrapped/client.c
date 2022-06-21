#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFSIZE 1024

int main(int argc, char *argv[]){

        int err, sockfd;
        socklen_t len;
        struct sockaddr_in address; //address
        char buffer[BUFSIZE];
        //socket creation
        if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0) ) < 0 ){
                perror("Error on socket!\n");
                exit(-1);
        }
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = inet_addr("127.0.0.1");
        address.sin_port=9734;
        len = (socklen_t)sizeof(address);
        err = connect(sockfd, (struct sockaddr*)&address, len);

        read(sockfd, buffer, BUFSIZE);
        printf("%s\n", buffer);
        close(sockfd);


        exit(0);
}
