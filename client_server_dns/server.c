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

uint32_t reverse_bytes(uint32_t bytes)
{
    uint32_t aux = 0;
    uint8_t byte;
    int i;

    for(i = 0; i < 32; i+=8)
    {
        byte = (bytes >> i) & 0xff;
        aux |= byte << (32 - 8 - i);
    }
    return aux;
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
        socklen_t client_len = (socklen_t)BUFSIZE;

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

                if( (pid = fork()) == 0){
                    Close(server_sockfd);
                    offset = count_connections(buffer);
                    current_time(buffer+offset);
                    FullWrite(client_sockfd, buffer, (size_t)BUFSIZE);
                    Close(client_sockfd);
                    exit(0);

                }else if(pid > 0){
                    Close(client_sockfd);
                    //Logging
                    inet_ntop(AF_INET, &client_address.sin_addr, IPaddress, sizeof(IPaddress));

                    struct in_addr raddr = client_address.sin_addr;
                    ip = inet_ntoa(raddr);

                    struct hostent * chost = gethostbyaddr((const char*)&raddr, sizeof(raddr), AF_INET);
                    if ( chost == NULL){
                        printf("Non sono riuscito a risalire a\n");
                    }else{
                        printf("Ok\n");
                    }

                    printf("Nuovo collegamento, IPaddress: %s\n Confronto con inet_nota: %s\n\n", IPaddress, inet_ntoa(client_address.sin_addr) );
                    //Increase counter
                    counter++;
                }else{
                    perror("Error while forking\n");
                    exit(1);
                }
            
	}

	exit(0);
}
