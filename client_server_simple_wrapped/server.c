#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "wrapped.h"


#define BUFSIZE 1024

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

void current_time(char *buf){
        int hours, minutes, seconds;
        time_t now;
        time(&now);
        struct tm *local = localtime(&now);
        hours = local->tm_hour;
        minutes = local->tm_min;
        seconds = local->tm_sec;
        snprintf(buf, BUFSIZE-1, "%02d:%02d:%02d\n",hours,minutes,seconds);
}

int main(int argc, char *argv[]){
        int err;
        socklen_t len;
        int server_sockfd, client_sockfd;
        struct sockaddr_in client_address;

        struct sockaddr_in server_address;
        struct sockaddr_in * ptr_server_address = &server_address;

        char buffer[BUFSIZE];
        socklen_t client_len = (socklen_t)BUFSIZE;

		//socket creation:
        Socket(&server_sockfd);
        makeSockaddr(ptr_server_address,"127.0.0.1", 9734, &len);
        Bind(server_sockfd, ptr_server_address, len);
        //listen(server_sockfd, 5);
        Listen(server_sockfd);

        while(1){
			current_time(buffer);
			client_sockfd = accept(server_sockfd,
								   (struct sockaddr*)&client_address,
								   &client_len);
			if (client_sockfd < 1){
				perror("Error on accepting client!\n");
                exit(-1);
			}

			if ( (write(client_sockfd, buffer, BUFSIZE) ) < 0  ){
				perror("Error on writing on client socket\n");
                exit(-1);
			}


			if ( ( close(client_sockfd) ) == -1 ){
				perror("Error on closing\n");
				exit(-1);
			}
		}
	exit(0);
}
