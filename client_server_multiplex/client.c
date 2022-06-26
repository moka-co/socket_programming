#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <netdb.h>

#include "wrapped.h"


#define BUFSIZE 1024


int main(int argc, char *argv[]){

        int port, sockfd;
        socklen_t len;
        struct sockaddr_in address; //address

        char buffer[BUFSIZE];
        char *IPaddress;

        struct sockaddr_in * ptr_address = &address;

        if (argc < 3){
                fprintf(stderr, "Inserisci un indirizzo e una porta in input\n");
                exit(1);
        }

        port = atoi(argv[2]);

        struct hostent * host = gethostbyname(argv[1]);


        print_hostent(host);


        if (host == NULL){
                fprintf(stderr, "Bad Address\n");
                exit(1);
        }

        char *stuff = host->h_addr_list[0];
        struct hostent * host2 = gethostbyaddr( stuff, sizeof(stuff), AF_INET);
        print_hostent(host2);

        IPaddress = inet_ntoa( *((struct in_addr*)host->h_addr_list[0]) );

        Socket(&sockfd);

        makeSockaddr(ptr_address, IPaddress, port, &len);

        Connect(sockfd, ptr_address, len); // Connetti

        //int c=5;
        //FullRead(sockfd, buffer, BUFSIZE);
        //printf("%s\n",buffer);
        printf("Inserisci una stringa\n");

        fgets(buffer, BUFSIZE, stdin);
        FullWrite(sockfd, buffer, BUFSIZE);

        FullRead(sockfd, buffer, BUFSIZE);

        printf("%s\n", buffer);

        Close(sockfd); //Chiudi connessione


        exit(0);
}
