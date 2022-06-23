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

int count_letters(char *string){
    int i=0, c=0;
    int try_size = (int)sizeof(string);

    printf("try_size: %d\n", try_size);

    while (string[i] != '\0'){
        if (string[i] != ' '){
            c++;
        }
        i++;
    }
    printf("c: %d\n",c);
    return c;
}

int main(int argc, char *argv[]){

        socklen_t len;
        int offset = 0;
        int pid;

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

        signal(SIGCHLD, SIG_IGN);
        
        while(1){
                Accept(&client_sockfd, server_sockfd, ptr_client_address, &client_len );

                sleep(5+counter);
                if( (pid = fork()) == 0){
                    Close(server_sockfd);

                    FullRead(client_sockfd, buffer, (size_t)BUFSIZE);

                    snprintf(buffer, BUFSIZE, "Numero di caratteri: %d", count_letters(buffer));
                    FullWrite(client_sockfd, buffer, (size_t)BUFSIZE);

                    Close(client_sockfd);
                    exit(0);

                }else if(pid > 0){
                    Close(client_sockfd);
                    counter++;
                }else{
                    perror("Error while forking\n");
                    exit(1);
                }
            
	}

	exit(0);
}
