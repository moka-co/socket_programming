#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <netdb.h>

#include "wrapped.h"


#define BUFSIZE 1024

int count_letters(char *string){
    int i=0, c=0;

    while (string[i] != '\0'){
        if (string[i] != ' ' && string[i] != '\n' && string[i] != '\0'){
            c++;
        }
        i++;
    }

    return c;
}

int main(int argc, char *argv[]){

        int port, sockfd;
        socklen_t len;
        struct sockaddr_in address; //address

        char i_buffer[BUFSIZE],o_buffer[BUFSIZE+3];
        char *IPaddress;

        struct sockaddr_in * ptr_address = &address;

        if (argc < 3){
                fprintf(stderr, "Inserisci un indirizzo e una porta in input\n");
                exit(1);
        }

        port = atoi(argv[2]);

        struct hostent * host = gethostbyname(argv[1]);

        IPaddress = inet_ntoa( *((struct in_addr*)host->h_addr_list[0]) );


        if (host == NULL){
                fprintf(stderr, "Bad Address\n");
                exit(1);
        }

        Socket(&sockfd);
        makeSockaddr(ptr_address, IPaddress, port, &len);
        Connect(sockfd, ptr_address, len); // Connetti

        printf("Inserisci il codice fiscale\n");
        
        int c = 0;
        /*
        while ( c != 16){
                fgets(buffer, BUFSIZE, stdin);
                c = count_letters(buffer);
                if ( c != 16){
                        printf("\n\nCodice fiscale non corretto: i caratteri devono essere 16\n");
                        printf("Inserisci il codice fiscale\n");
                }
        }
        */
        fgets(i_buffer, BUFSIZE, stdin);
        snprintf(o_buffer, BUFSIZE+3, "CS|%s", i_buffer );

        FullWrite(sockfd, o_buffer, BUFSIZE);

        FullRead(sockfd, i_buffer, BUFSIZE);

        printf("%s\n", i_buffer);

        Close(sockfd); //Chiudi connessione


        exit(0);
}
