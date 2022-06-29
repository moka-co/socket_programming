#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <netdb.h>

#include "wrapped.h"


int main(int argc, char *argv[]){

        int port=0, sockfd=0, c=0;
        socklen_t len;
        struct sockaddr_in address; //address

        char i_buffer[BUFSIZE],o_buffer[BUFSIZE+3];
        char *IPaddress;
        char i;

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

        printf("Inserisci un codice fiscale\n");
        
        fgets(i_buffer, CODE_MAXSIZE+2, stdin);
        
        for(int i=0;i<CODE_MAXSIZE+2;i++){
                if (i_buffer[i]=='\n'){
                        i_buffer[i]='\0';
                }
        }
        

        printf("Scrivi 0 per invalidare il green pass, scrivi 1 per riprisintare la validità\n");

        i = (char) fgetc(stdin);
        c = atoi(&i);
        
        snprintf(o_buffer, BUFSIZE+6, "CT|%s:%d", i_buffer, c );

        Socket(&sockfd);
        makeSockaddr(ptr_address, IPaddress, port, &len);
        Connect(sockfd, ptr_address, len); // Connetti

        FullWrite(sockfd, o_buffer, BUFSIZE);

        FullRead(sockfd, i_buffer, BUFSIZE);

        printf("%s\n", i_buffer);

        Close(sockfd); //Chiudi connessione

        exit(0);
}
