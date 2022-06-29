#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <netdb.h>

#include "wrapped.h"

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

        fgets(i_buffer, BUFSIZE, stdin);
        snprintf(o_buffer, BUFSIZE+3, "CS|%s", i_buffer ); //Scrivi i dati nel formato accettato dal server

        FullWrite(sockfd, o_buffer, BUFSIZE); //Invia i dati al server G

        FullRead(sockfd, i_buffer, BUFSIZE); //Leggi la risposta

        printf("%s\n", i_buffer);

        Close(sockfd); //Chiudi connessione


        exit(0);
}
