#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include "wrapped.h"


#define BUFSIZE 1024

int counter = 0;

int count_connections(char *buffer){ //return written bytes
        int bwritten = 0;

        const char * msg = "Contatore collegamenti fin ora: %d\n";
        bwritten = snprintf(buffer, BUFSIZE, msg ,counter);

        return bwritten;
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

        socklen_t len;
        int offset = 0;
        int pid;
        int port;

        int server_sockfd, client_sockfd;

        struct sockaddr_in client_address;
        struct sockaddr_in * ptr_client_address = &client_address;

        struct sockaddr_in server_address;
        struct sockaddr_in * ptr_server_address = &server_address;

        char IPaddress[INET6_ADDRSTRLEN];
        char *ip;

        char buffer[BUFSIZE];
        socklen_t client_len = sizeof(client_address);

        if (argc < 3){
                fprintf(stderr, "Inserisci un indirizzo e una porta in input\n");
                exit(1);
        }

        port = atoi(argv[2]);
        struct hostent * host = gethostbyname(argv[1]);

        if (host == NULL){
                fprintf(stderr, "Bad Address\n");
                exit(1);
        }

        ip = inet_ntoa( *((struct in_addr*)host->h_addr_list[0]) );

        Socket(&server_sockfd); //socket creation:
        makeSockaddr(ptr_server_address,ip, port, &len);
        Bind(server_sockfd, ptr_server_address, len);
        Listen(server_sockfd);

        signal(SIGCHLD, SIG_IGN);
        
        while(1){
                Accept(&client_sockfd, server_sockfd, ptr_client_address, &client_len );

                /*
                client_sockfd = accept(server_sockfd, (struct sockaddr*)&client_address, &len);

                if ( client_sockfd < 1){
		            perror("Error while accepting client ");
                    exit(1);
	            }
                */

                if( (pid = fork()) == 0){
                    Close(server_sockfd);
                    offset = count_connections(buffer);
                    current_time(buffer+offset);
                    FullWrite(client_sockfd, buffer, (size_t)BUFSIZE);
                    Close(client_sockfd);
                    exit(0);

                }else if(pid > 0){
                    //Logging
                    printf("\tNuova connessione: %s\n", inet_ntoa(client_address.sin_addr));
                    //Increase counter
                    counter++;
                    Close(client_sockfd);
                }else{
                    perror("Error while forking\n");
                    exit(1);
                }
            
	}

	exit(0);
}
