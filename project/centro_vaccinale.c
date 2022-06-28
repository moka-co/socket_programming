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

// TODO
// - usare due socket diverse
// - restituire un messaggio di errore al client in caso il codice sia già stato registrato

const char* parse_string(char *string, size_t sz){

        int c=0;

        for(int i=0; i<sz;i++){
                if (string[i+1] == ' ' || string[i+1] == '\n' || string[i+1] == '\0'){
                        c = 1 + i;
                        i = 20;
                }
        }

        char *newstr = (char *)malloc(sizeof(char)*c);

        strncpy(newstr,string,c);

        return newstr;
}


int main(int argc, char *argv[]){

        int port, sockfd, peer_sockfd;
        socklen_t len = BUFSIZE, plen = BUFSIZE;
        struct sockaddr_in addr; //address
        struct sockaddr_in peer_addr;

        char i_buffer[BUFSIZE],o_buffer[BUFSIZE];

        char *code = (char *)malloc(sizeof(char)*16);
        
        char *IPaddress;

        struct sockaddr_in * ptr_address = &addr;
        struct sockaddr_in * ptr_peer_address = &peer_addr;

        if (argc < 5){
                fprintf(stderr, "Inserisci un indirizzo e una porta in input da cui un client può connettersi e un indirizzo e la porta del server V\n");
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

        strncpy(code, i_buffer, 16);
        printf("code: %s|vado a capo\n",code);

        // Valido per 3 mesi
        struct tm* timeinfo = validity();
        //Connettiti a ServerV
        //printf("Inserisci l'indirizzo o il nome simbolico del server a cui connettersi\n");
        //fgets(i_buffer,(size_t)16, stdin);
        //strncpy(IPaddress, i_buffer, (size_t)16);

        //const char *ip = parse_string(IPaddress,16);

        struct hostent *host2 = gethostbyname(argv[3]);

        if (host2 != NULL){
                IPaddress = inet_ntoa( *((struct in_addr*)host2->h_addr_list[0]) );
        }else{
                herror("Error: ");
                exit(1);
        }

        port = atoi(argv[4]);

        Socket(&sockfd);
        makeSockaddr(ptr_address, IPaddress, port, &len);
        Connect(sockfd, ptr_address, len); // Connetti

        /*
        memset(o_buffer,0,BUFSIZE);
        for(int i=0; i<16; i++){
                if (code[i]=='\n')
                        code[i]='\0';
        }*/


        memset(o_buffer,0,BUFSIZE);
        snprintf(o_buffer, 31, "CV|%s:%d-%d",code,timeinfo->tm_mon, 1900+timeinfo->tm_year);
        printf("obuffer: %s\n",o_buffer);

        FILE* f = fopen("test","a");
        fprintf(f,"%s",o_buffer);
        fclose(f);

        FullWrite(sockfd, o_buffer, BUFSIZE);
        FullRead(sockfd, i_buffer, BUFSIZE);
        printf("%s\n",i_buffer);
        Close(sockfd);

        exit(0);
}
