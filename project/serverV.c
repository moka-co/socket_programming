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


#define BUFSIZE 1024

//TODO
//Togliere le routine non usate
//Mettere le routine usate da più funzioni in un header
//Gestire il caso di Green Pass scaduto per CS

int counter = 0;
const char* filename = "data"; 

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

    while (string[i] != '\0'){
        if (string[i] != ' ' && string[i] != '\n' && string[i] != '\0'){
            c++;
        }
        i++;
    }

    return c;
}

int save_data(char *buffer, size_t sz){
	
	FILE* data= fopen(filename,"r"); //apertura file in modalità "append"

    char newcode[16];
    int newmonth;
    int newyear;

    sscanf(buffer,"%[a-zA-Z0-9]:%d-%d", newcode, &newmonth, &newyear);
    perror("scanf: ");
    printf("buffer: %s\nletto: %s e %d e %d",buffer, newcode,newmonth,newyear);

    char wbuffer[BUFSIZE];
    memset(wbuffer, 0, BUFSIZE);
    snprintf(wbuffer,BUFSIZE,"%s:%d-%d:%d",newcode, newmonth, newyear, 1);

    char oldcode[16];
    int oldmonth;
    int oldyear;

    char readbuffer[sz];
    memset(readbuffer, 0, sz);

    int flag = 0;
    int c=0;

    if (data){
        printf("\n____\n");
	    while(fgets(readbuffer, sz, data)){
            printf("%s\n",readbuffer);
            c = count_letters(buffer);
            if ( strncmp(readbuffer, buffer, c) == 0){
                printf("Codice fiscale già inserito\n");
                return 1;
            }

        }
        printf("____\n");
        fclose(data);
    }

    //while ( (data = fopen(filename,"a")) == NULL );
    data = fopen(filename,"a");
    if (data){

        if (flag == 0){
            fprintf(data, "%s", wbuffer);
        }


	    fclose(data);
    }

	return 0;
}

int is_valid(char *buffer, size_t sz){

    FILE* data= fopen(filename,"r");
    char readbuffer[sz];
    int c=0;

    if (data){
        printf("\n____\n");
	    while(fgets(readbuffer, sz, data)){
            printf("%s\n",readbuffer);
            c = count_letters(buffer);
            if ( strncmp(readbuffer, buffer, c) == 0){
                char oldcode[16];
                memset(oldcode, 0, 16);
                int oldmonth=0;
                int oldyear=0;
                int valid=1;
                sscanf(readbuffer, "%[a-zA-Z0-9]:%d-%d:%d", oldcode, &oldmonth, &oldyear, &valid);
                return valid;
            }

        }
        printf("____\n");
        fclose(data);
        return -1; //codice non registrato
    }

}

int validate(char *buffer, size_t sz){
    FILE* data= fopen(filename,"r+"); //il file data è aperto in modalità scrittura e lettura in questo caso perché dobbiamo aggiornare delle stringhe
    char readbuffer[sz]; //readbuffer è il buffer dove si tengono le righe che si leggono nel file data
    int c=0; //c serve per tenere conto del numero di lettere in una stringa

    int validity=0; //validity dovrà tenere il nuovo valore da sostituire, 0 indica green pass invalido, 1 valido
    char newcode[16]; //Qui si salva il codice che viene passato in input
    char savedcode[16]; //Qui si salva ad ogni ciclo del while, il codice letto
    char tmpcode[16]; //Un altro buffer che FORSE RIESCO A TOGLIERE

    memset(newcode, 0, 16); 
    memset(savedcode, 0, 16);
    strncpy(newcode, buffer, 16); //16 

    sscanf(buffer, "%[a-zA-Z0-9]\n:%d", tmpcode, &validity); //Dal buffer dobbiamo prenderci il codice e validity

    char *token_newcode = strtok(newcode, ":"); //Togliamo via tutto quello che c'è dopo ":" che non fa parte del codice del green pass
    char *token_savedcode;

    c = count_letters(newcode); //Il numero di lettere senza spazi o caratteri a capo, ci serve per la funzione strncmp

    long old_offset=0, new_offset=0; //Gli offset servono per aggiornare i dati nel file

    if (data){
	    while(fgets(readbuffer, sz, data)){
            //L'offset vecchio serve per usare fseek dopo e tornare al punto nel file dove va scritto
            old_offset = new_offset;
            new_offset = ftell(data);
            
            strncpy(savedcode, readbuffer, 16); //Ci prendiamo il codice salvato (cioè i primi 16 caratteri) e lo mettiamo in savedcode
            token_savedcode =  strtok(savedcode, ":"); //Però il codice può essere più corto di 16 caratteri massimi (temporaneo) quindi viene tagliata la parte dopo i :
            //Effettivamente ^^ potrebbe essere ridondante ed essere tolto in favore di readbuffer.

            c = count_letters(token_newcode); //Questo potrebbe essere tolto <--------__TEST
            
            if ( strncmp(token_savedcode, token_newcode, c) == 0){ //trovato il codice da dover validare/invalidare
                
                int oldmonth=0; //Probabilmente questi starebbero meglio in una struct
                int oldyear=0;
                int valid=1;

                sscanf(readbuffer, "%[a-zA-Z0-9]:%d-%d:%d", tmpcode, &oldmonth, &oldyear, &valid); //parsing della stringa e prendiamo i valori di interesse nella variabile

                if ( valid != validity){ //Se non c'è alcun cambiamento da fare, non ha senso scrivere
                    fseek(data, old_offset, SEEK_SET);
                    fprintf(data,"%s:%d-%d:%d\n", token_savedcode, oldmonth, oldyear, validity); //I valori presi con la sscanf precedente, solo con validity che è cambiato o uguale.

                }
    
                fclose(data);
                return validity;

            }

        }
        fclose(data);
        return -1; //codice non registrato
    }

}

int main(int argc, char *argv[]){

        socklen_t len;
        int offset = 0;
        int pid;
        int port;

        int server_sockfd, client_sockfd;

        struct sockaddr_in client_address;
        struct sockaddr_in * ptr_client_address = &client_address;

        struct sockaddr_in server_address;
        struct sockaddr_in * ptr_server_address = &server_address;

        char IPaddress[INET_ADDRSTRLEN];
        char *ip;

        char i_buffer[BUFSIZE], o_buffer[BUFSIZE];
        socklen_t client_len = sizeof(client_address);

        if (argc < 3){
                fprintf(stderr, "Inserisci un indirizzo e una porta in input\n");
                exit(1);
        }

        port = atoi(argv[2]);
        struct hostent * host = gethostbyname(argv[1]);

        if (host == NULL){
                fprintf(stderr, "Bad Address\n");
                exit(1);
        }

        ip = inet_ntoa( *((struct in_addr*)host->h_addr_list[0]) );

        Socket(&server_sockfd); //socket creation:
        makeSockaddr(ptr_server_address,ip, port, &len);
        Bind(server_sockfd, ptr_server_address, len);
        Listen(server_sockfd);

        signal(SIGCHLD, SIG_IGN);

        fd_set set;
        int list_fd = server_sockfd;
        int fd=-1;
        int maxfd = list_fd; 
        int fd_open[maxfd+5];

        for(int i=0;i<maxfd+5;i++) fd_open[i]=0;

        int n=0,i=0;
        fd_open[maxfd] = 1;
        FD_SET(list_fd, &set);
        
        while(1){
                //Questo blocco di istruzioni fino alla parentesi
                // c'è perché nel manuale di select si legge che bisogna reistanziare il set prima di ogni chiamata select() su un set.
                FD_ZERO(&set);
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

                if (FD_ISSET(list_fd, &set)){ //Se il file descriptor in ascolto è settato, quindi c'è una nuova connessione
                    n--;
                    Accept(&fd, list_fd, ptr_client_address, &client_len); //Accetta la nuova connessione
                    FD_SET(fd, &set);
                    fd_open[fd]=1; //Segnala che il relativo file descriptor è correntemente aperto da una connessione
                    if(maxfd < fd)
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
                                Close(i+1);            
                                 
                                maxfd=i; //Il sockfd trovato sarà il nuovo massimo
                                break;
                                
                            }
                        continue;
                        }

                        if (nread > 0){
                            printf("buffer: %s\n",i_buffer);
                            char l_buffer[BUFSIZE];
                            strcpy(l_buffer, i_buffer);
                            if ( (strncmp(l_buffer,"CV",2)) == 0){
                                if ( save_data(l_buffer+3, (size_t)BUFSIZE) == 0){
                                    snprintf(o_buffer, BUFSIZE, "Codice fiscale registrato con successo\n");
                                }else{
                                    snprintf(o_buffer, BUFSIZE, "Codice fiscale già inserito\n");
                                }
                            
                                FullWrite(i, o_buffer, (size_t)BUFSIZE);

                            }else if( (strncmp(l_buffer,"CS",2)) == 0 ){

                                int e = is_valid(l_buffer+3, BUFSIZE); //Controlla la validità del certificato
                                if ( e == 1  ){
                                    snprintf(o_buffer, BUFSIZE, "Codice fiscale valido\n");
                                }else if( e == 0 ){
                                    snprintf(o_buffer, BUFSIZE, "Codice fiscale NON valido\n");
                                }else{
                                    snprintf(o_buffer, BUFSIZE, "Codice fiscale NON registrato\n");
                                }

                                FullWrite(i, o_buffer, (size_t)BUFSIZE);
                            }else if( (strncmp(l_buffer,"CT",2)) == 0){
                                int e = validate(l_buffer+3, BUFSIZE);
                                if (e == 0){
                                    snprintf(o_buffer, BUFSIZE, "Il green pass corrispondente è stato invalidato\n");
                                }else if (e==1){
                                    snprintf(o_buffer, BUFSIZE, "Il green pass corrispondente è ora valido\n");
                                }else{
                                    snprintf(o_buffer, BUFSIZE, "Codice fiscale non trovato\n");
                                }

                                FullWrite(i, o_buffer, (size_t)BUFSIZE);
                            }else{
                                FullWrite(i,"Errore non capisco la richiesta\n", BUFSIZE);
                            }

                        }
                    }
                }
            
	}

	exit(0);
}
