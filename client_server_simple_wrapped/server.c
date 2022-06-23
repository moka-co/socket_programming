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

int counter = 0;

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

        socklen_t len;
        int server_sockfd, client_sockfd;

        struct sockaddr_in client_address;
        struct sockaddr_in * ptr_client_address = &client_address;

        struct sockaddr_in server_address;
        struct sockaddr_in * ptr_server_address = &server_address;

        char buffer[BUFSIZE];
        socklen_t client_len = (socklen_t)BUFSIZE;

        Socket(&server_sockfd); //socket creation:
        makeSockaddr(ptr_server_address,"127.0.0.1", 9735, &len);
        Bind(server_sockfd, ptr_server_address, len);
        Listen(server_sockfd);

        while(1){
            Accept(&client_sockfd, server_sockfd, ptr_client_address, &client_len );
            counter++;
            snprintf(buffer, "Client collegati fin ora: %d\n",counter);
            current_time(buffer);
            FullWrite(client_sockfd, buffer, (size_t)BUFSIZE);
            


			if ( ( close(client_sockfd) ) == -1 ){
				perror("Error while closing ");
				exit(-1);
			}
	}

	exit(0);
}
