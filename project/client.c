#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <netdb.h>

#include "wrapped.h"




int main(int argc, char *argv[]){

        int port=0, sockfd=0,c=0;
        socklen_t len;
        struct sockaddr_in address; //address

        char buffer[CODE_MAXSIZE+1];
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

        fgets(buffer, CODE_MAXSIZE+1, stdin);

        for(int i=0;i<CODE_MAXSIZE+1;i++){
                if (buffer[i]=='\n')
                        buffer[i]='\0';
        }

        FullWrite(sockfd, buffer, BUFSIZE);

        FullRead(sockfd, buffer, BUFSIZE);

        printf("%s\n", buffer);

        Close(sockfd); //Chiudi connessione

        exit(0);
}
