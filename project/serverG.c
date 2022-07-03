#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/select.h>

#include "wrapped.h"

int counter = 0;

int main(int argc, char *argv[]){

        socklen_t len;
        int offset = 0;
        int port=0, portv=0;
        int server_sockfd=0, client_sockfd=0, sockfdV=0;

        struct sockaddr_in client_address;
        struct sockaddr_in * ptr_client_address = &client_address;

        struct sockaddr_in server_address;
        struct sockaddr_in * ptr_server_address = &server_address;

        struct sockaddr_in serverV_address;
        struct sockaddr_in * ptr_serverV_address = &serverV_address;

        char *ip, *ipv;
        char myip[16];

        char i_buffer[BUFSIZE], o_buffer[BUFSIZE], l_buffer[BUFSIZE];
        socklen_t client_len = sizeof(client_address);

        if (argc < 5){
                fprintf(stderr, "Inserisci un indirizzo e una porta in input da cui accettare connessioni e l'indirizzo e la porta del server V\n");
                exit(1);
        }

        port = atoi(argv[2]);
        struct hostent * host = gethostbyname(argv[1]);

        if (host == NULL){
            fprintf(stderr, "Bad Address\n");
            exit(1);
        }
        ip = inet_ntoa( *((struct in_addr*)host->h_addr_list[0]) );
        strcpy(myip,ip);

        portv = atoi(argv[4]);
        struct hostent * hostV = gethostbyname(argv[3]);

        if ( hostV == NULL){
                fprintf(stderr, "Bad Address\n");
                exit(1);
        }
        
        ipv = inet_ntoa( *((struct in_addr*)hostV->h_addr_list[0]) );

        //Crea la socket per accettare connessioni
        Socket(&server_sockfd); //socket creation:
        makeSockaddr(ptr_server_address,myip, port, &len);
        Bind(server_sockfd, ptr_server_address, len);
        Listen(server_sockfd);

        fd_set set;
        int list_fd = server_sockfd;
        int fd=-1;
        int maxfd = list_fd; 
        int fd_open[FD_OPEN_MAXSIZE]; 

        for(int i=0;i<FD_OPEN_MAXSIZE;i++) fd_open[i]=0;

        int n=0,i=0;
        fd_open[maxfd] = 1;
        
        while(1){ //Multiplexing
                
                FD_ZERO(&set); //Bisogna reistanziare set prima di ogni chiamata select() su uno stesso set
                for(int i=list_fd;i<=maxfd;i++){ //Setta i file descriptor da list_fd a max_fd dentro set
                    if (fd_open[i]!= 0){
                        FD_SET(i,&set);
                    }

                }

                while ( ((n = select(maxfd+1, &set, NULL, NULL, NULL)) < 0) && (errno == EINTR)); //Aspetta che un descrittore in lettura sia libero
                //In n verrà messo il numero di descrittori di file disponibili.
                if ( n < 1){ //Non si è riusciti a selezionare un file descriptor
                    perror("E: ");
                    exit(1);
                }

                if (FD_ISSET(list_fd, &set)){ //Se il file descriptor in ascolto è settato cioè è attivo, si può accettare una nuova connessione
                    n--; //n è il contatore dei descrittori attivi
                    Accept(&fd, list_fd, ptr_client_address, &client_len); //Accetta la nuova connessione
                    FD_SET(fd, &set); //Setta il relativo file descriptor
                    fd_open[fd]=1; //Segnala che il relativo file descriptor è correntemente aperto da una connessione
                    if(maxfd < fd) //maxfd deve avere valore uguale al file descriptor con valore massimo.
                        maxfd = fd;

                }
                i = list_fd; //parti dal list_fd
                while(n!=0){ //Cicla finché ci sono connessioni attive
                    i++; //Tipicamente 0 1 2 sono input,output ed error, 3 è listening, partiamo da 4
                    if (fd_open[i] == 0){ //se è messo a 0, la connessione è chiusa quindi passa al successivo
                        continue;
                    }
                    if (FD_ISSET(i, &set)){ //i è un file descriptor attivo cioè pronto a ricevere/trasmettere dati
                        n--;

                        int nread = read(i, i_buffer, (size_t)BUFSIZE); //Leggi dal file descriptor del socket attivo
                        if (nread < 0){ 
                            perror("Errore lettura");
                            exit(1);
                        }

                        if (nread == 0){ //Se la connessione è chiusa
                            fd_open[i]=0; //Setta nell'array che il relativo fd è chiuso
                            if (maxfd == i){ //Se il socket attivo è quello con numero massimo
                                while(fd_open[--i]==0){ //cicla decrementando i finché non trovi un socket descriptor attivo
                                    Close(i+1); //Per evitare di fare overflow di descrittori di file dentro fd_open
                                }
                                Close(i+1); // forse è più opportuno chiudere dopo il costrutto if nread > 0 etc <----------- TEST
                                 
                                maxfd=i; //Il sockfd trovato sarà il nuovo massimo
                                break;
                                
                            }
                        continue;
                        }

                        if (nread > 0){
                            Socket(&sockfdV);
                            makeSockaddr(ptr_serverV_address, ipv, portv, &len);
                            Connect(sockfdV, ptr_serverV_address, len); // Connettiti a serverV
                            
                            printf("buffer: %s\n",i_buffer); 
                            strcpy(l_buffer, i_buffer); //Utilizziamo un buffer locale perché i_buffer potrebbe essere usato anche da altri client che si collegano.

                            if ( (strncmp(l_buffer,"CS",2)) == 0 ){ //Caso in cui si è ricevuta una stringa da Client S
                                FullWrite(sockfdV, l_buffer, (size_t)BUFSIZE); //Passa la stringa presa da clientS a serverG                       
                                FullRead(sockfdV, l_buffer, (size_t)BUFSIZE);  //Leggi la risposta di serverV
                                FullWrite(i, l_buffer, (size_t)BUFSIZE); //Invia la risposta a 

                            }else if( (strncmp(l_buffer,"CT",2)) == 0 ){ //Caso in cui si è ricevuta una stringa da Client T

                                //FullWrite(sockfdV, l_buffer, (size_t)BUFSIZE); //Passa la stringa presa da clientT a serverG
                                FullWrite(sockfdV, l_buffer, DATA_FORMAT_MAXSIZE_T);
                                //Il serverV si occuperà del parsing e capirà cosa farci con la stringa
                                FullRead(sockfdV, l_buffer, (size_t)BUFSIZE); //Leggi la risposta da serverV
                                FullWrite(i, l_buffer, (size_t)BUFSIZE); //Invia a clientT la risposta di serverV
                            }else{
                                FullWrite(i, "Errore non capisco la richiesta\n", (size_t)BUFSIZE);
                            }
                            memset(l_buffer, 0, BUFSIZE);
                            Close(sockfdV); //Chiudi la connessione al server V
                        }
                    }
                }
            
	}

	exit(0);
}
