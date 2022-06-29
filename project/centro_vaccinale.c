#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <netdb.h>

#include "wrapped.h"

struct tm* expires_on(){
        time_t rawtime;
        struct tm * timeinfo;
        
        time (&rawtime);
        timeinfo = localtime (&rawtime);
    
        if (timeinfo->tm_mon + 3 > 11 ){
                timeinfo->tm_year += 1;
                timeinfo->tm_mon = (timeinfo->tm_mon + 3)%11;
        }else{
        timeinfo->tm_mon = timeinfo->tm_mon + 3;
        }

        return timeinfo;
}

int main(int argc, char *argv[]){

        int port, portV;
        int sockfd, peer_sockfd, sockfdV;
        socklen_t len = BUFSIZE, plen = BUFSIZE;
        struct sockaddr_in addr; //address
        struct sockaddr_in peer_addr;
        struct sockaddr_in addrV;

        char i_buffer[BUFSIZE],o_buffer[BUFSIZE];

        char *code = (char *)malloc(sizeof(char)*CODE_MAXSIZE);
        
        char *ip, *ipv;

        struct sockaddr_in * ptr_address = &addr;
        struct sockaddr_in * ptr_peer_address = &peer_addr;
        struct sockaddr_in * ptr_addrV = &addrV;

        if (argc < 5){
                fprintf(stderr, "Inserisci un indirizzo e una porta in input da cui un client può connettersi e un indirizzo e la porta del server V\n");
                exit(1);
        }

        port = atoi(argv[2]);
        struct hostent * host = gethostbyname(argv[1]);
        ip = inet_ntoa( *((struct in_addr*)host->h_addr_list[0]) );

        //Crea la socket per accettare connessioni dal peer
        Socket(&sockfd);
        makeSockaddr(ptr_address, ip, port, &len);
        Bind(sockfd, ptr_address, len);
        Listen(sockfd);

        Accept(&peer_sockfd, sockfd, ptr_peer_address, &plen);

        //Leggi il codice fiscale
        FullRead(peer_sockfd, i_buffer, BUFSIZE);

        Close(sockfd); //Chiudi la socket per accettare nuove connessioni

        printf("Ricevuto codice fiscale: %s\n", i_buffer);

        strncpy(code, i_buffer, CODE_MAXSIZE);

        struct tm* timeinfo = expires_on(); // Valido per 3 mesi

        struct hostent *host2 = gethostbyname(argv[3]);

        if (host2 != NULL){
                ipv = inet_ntoa( *((struct in_addr*)host2->h_addr_list[0]) );
        }else{
                herror("Error: ");
                exit(1);
        }

        portV = atoi(argv[4]);

        //Connessione con il server V
        Socket(&sockfdV);
        makeSockaddr(ptr_addrV, ipv, portV, &len);
        Connect(sockfdV, ptr_addrV, len); 

        memset(o_buffer,0,BUFSIZE);
        snprintf(o_buffer, DATA_FORMAT_MAXSIZE_T, "CV|%s:%d-%d",code,timeinfo->tm_mon, 1900+timeinfo->tm_year);

        FullWrite(sockfdV, o_buffer, BUFSIZE); //Invia al serverV i dati da memorizzare nel formato opportuno
        FullRead(sockfdV, i_buffer, BUFSIZE); //Leggi la risposta del serverV
        printf("%s\n",i_buffer);
        Close(sockfdV); //Chiudi la connessione con serverV

        FullWrite(peer_sockfd, i_buffer, BUFSIZE); //Invia la risposta al client

        exit(0);
}
