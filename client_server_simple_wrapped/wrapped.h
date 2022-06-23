#include <stdlib.h>
#include <stdio.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

void Listen(int sockfd){
    if ( listen(sockfd, 5) != 0){
        perror("Error on listening\n");
        exit(1);
    }

}

void makeSockaddr(struct sockaddr_in* struct_address, char* addr, int port, socklen_t *len ){
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
        perror("Error while binding");
        exit(1);
    }
}

void Connect(int sockfd, struct sockaddr_in * address,int len){
    if ( connect(sockfd, (struct sockaddr*)address, len ) != 0){
        perror("Error while connecting");
        exit(1);
    }
}

void Accept(int * client_sockfd, int server_sockfd, struct sockaddr_in * address,  socklen_t * len){

    (* client_sockfd) = accept(server_sockfd, (struct sockaddr*)&address, len);

    if ( (*client_sockfd) < 1){
		perror("Error while accepting client ");
        exit(1);
	}
}

void FullWrite(int sockfd, char *buffer, size_t count){
    int nwritten=0;
    size_t nleft = count;

    while (nleft > 0){ //Finché ci sono byte da scrivere
        nwritten = write(sockfd, buffer, nleft); //Scrivi

        if ( nwritten < 0){ //Errore

            if (errno = EINTR){ //Ignora se interrotto da una system call
                continue;
            }else{
                perror("Error while writing through the socket\n");
                exit(1);
            }

        }

        nleft = nleft - nwritten;
        buffer += nwritten;
        
    }

}

void FullRead(int sockfd, char *buffer, size_t count){
    int nread=0;
    size_t nleft = count;

    while (nleft > 0){
        nread = read(sockfd, buffer, count);

        if (nread < 0){
            if (errno = EINTR){
                continue;
            }else{
                perror("Error while reading through the socket\n");
                exit(1);
            }
        }
        nleft = nleft - nread;
        buffer += nread;
    }

}
