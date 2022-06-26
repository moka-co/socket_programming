#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <netdb.h>

#include "wrapped.h"


#define BUFSIZE 1024

struct tm* validity(){
time_t rawtime;
    struct tm * timeinfo;

    time (&rawtime);
    timeinfo = localtime (&rawtime);
    timeinfo->tm_mon = timeinfo->tm_mon + 3;

    return timeinfo;
}


int main(int argc, char *argv[]){

        int port, sockfd, peer_sockfd;
        socklen_t len = BUFSIZE, plen = BUFSIZE;
        struct sockaddr_in addr; //address
        struct sockaddr_in peer_addr;

        char i_buffer[BUFSIZE],o_buffer[BUFSIZE], code[17];
        char *IPaddress;

        struct sockaddr_in * ptr_address = &addr;
        struct sockaddr_in * ptr_peer_address = &peer_addr;

        if (argc < 3){
                fprintf(stderr, "Inserisci un indirizzo e una porta in input\n");
                exit(1);
        }

        port = atoi(argv[2]);
        struct hostent * host = gethostbyname(argv[1]);
        IPaddress = inet_ntoa( *((struct in_addr*)host->h_addr_list[0]) );

        //Crea la socket per accettare connessioni dal client
        Socket(&sockfd);
        makeSockaddr(ptr_address, IPaddress, port, &len);
        Bind(sockfd, ptr_address, len);
        Listen(sockfd);

        Accept(&peer_sockfd, sockfd, ptr_peer_address, &plen);

        //Leggi il codice fiscale
        FullRead(peer_sockfd, i_buffer, BUFSIZE);

        printf("Ricevuto codice fiscale: %s\n", i_buffer);

        //Segnala al client che hai ricevuto con successo i dati
        snprintf(o_buffer,BUFSIZE,"Codice fiscale ricevuto con successo\n");
        FullWrite(peer_sockfd, o_buffer, BUFSIZE);
        Close(sockfd); //Chiudi connessione

        strncpy(code, i_buffer, (size_t)16);

        // Valido per 3 mesi
        struct tm* timeinfo = validity();
        //Connettiti a ServerV
        printf("Inserisci l'indirizzo o il nome simbolico del server a cui connettersi\n");
        fgets(i_buffer,BUFSIZE, stdin);
        strncpy(IPaddress, i_buffer, (size_t) 16 );
        printf("Inserito: %s\n",IPaddress);
        printf("Inserisci la porta aperta dal processo server\n");
        fgets(i_buffer, BUFSIZE, stdin);
        port = atoi(i_buffer);
        printf("Inserito: %d\n", port);

        Socket(&sockfd);
        makeSockaddr(ptr_address, IPaddress, port, &len);
        Connect(sockfd, ptr_address, len); // Connetti

        snprintf(o_buffer, BUFSIZE, "%s\t%s",code,asctime(timeinfo));
        FullWrite(sockfd, o_buffer, BUFSIZE);
        FullRead(sockfd, i_buffer, BUFSIZE);
        printf("%s\n",i_buffer);

        exit(0);
}
