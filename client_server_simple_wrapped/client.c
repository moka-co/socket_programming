#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "wrapped.h"


#define BUFSIZE 1024

int main(int argc, char *argv[]){

        int err, sockfd;
        socklen_t len;
        struct sockaddr_in address; //address

        char buffer[BUFSIZE];
        /*
        //socket creation
        if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0) ) < 0 ){
                perror("Error on socket!\n");
                exit(-1);
        }
        */

        struct sockaddr_in * ptr_address = &address;

        Socket(&sockfd);

        makeSockaddr(ptr_address,"127.0.0.1", 9734, &len);

        Connect(sockfd, ptr_address, len);

        if ( read(sockfd, buffer, BUFSIZE) == -1 ){
                perror("Error while reading through the socket\n");
                exit(1);
        }

        printf("%s", buffer);
        close(sockfd);


        exit(0);
}
