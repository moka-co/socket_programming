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
const char* filename = "data"; 

struct data_formatv {
    char *code; //Il codice fiscale
    int month; //Il mese
    int year; //L'anno
    int valid; //Validità
};

int count_letters(char *string){
    int i=0, c=-1;
    int try_size = (int)sizeof(string);

    for(int i=0;i<CODE_MAXSIZE+1;i++){
        if (string[i] == ' ' || string[i] == '\n' || string[i] == '\0'){
            c = i;
            break;
        }
    }

    if (c == -1){
        c = 16;
    }
    return c;
}

int save_data(char *buffer, size_t sz){
    struct data_formatv newdt, savedt;
    newdt.code = (char *)malloc(sizeof(char)*CODE_MAXSIZE);
    savedt.code = (char *)malloc(sizeof(char)*CODE_MAXSIZE);

	FILE* data= fopen(filename,"r"); //apertura file in modalità "append"

    fseek(data, 0, SEEK_END);
    long size = ftell(data);
    fseek(data, 0, SEEK_SET);

    sscanf(buffer,"%[a-zA-Z0-9]:%d-%d", newdt.code, &newdt.month, &newdt.year);

    char *readbuffer = (char *)malloc(sizeof(char)*sz);
    memset(readbuffer, 0, sz);

    int c1=0,c2=0;

    c1 = count_letters(newdt.code);

    if (data){
        while(fgets(readbuffer,sz,data) != NULL){ 
            
            sscanf(readbuffer,"%[a-zA-Z0-9]:%d-%d\n", savedt.code, &savedt.month, &savedt.year);
            c2 = count_letters(savedt.code);

            if ( (c1 == c2) && strncmp(buffer, readbuffer, c1) == 0){
                printf("Codice fiscale già inserito\n");
                fclose(data);
                return 1;
            }
            memset(readbuffer,0,sz);

            if (ftell(data)==size){
                break;
            }

        }
    }
    fclose(data);

    data = fopen(filename,"a");
    if (data){
        fprintf(data, "%s:1\n", buffer);
	    fclose(data);
        return 0;
    }else{
        printf("Non riesco ad aprire il file\n");
    }

	return 0;
}

int is_valid(char *buffer, size_t sz){
    time_t rawtime;
    struct tm * timeinfo;
    time(&rawtime);
    timeinfo = localtime (&rawtime);


    struct data_formatv saved;
    saved.code = (char *)malloc(sizeof(char)*CODE_MAXSIZE);

    FILE* data= fopen(filename,"r");
    char *readbuffer = (char *)malloc(sz * sizeof(char));
    int c1=0,c2=0;

    c2 = count_letters(buffer);

    if (data){
	    while(fgets(readbuffer, sz, data) != NULL){
            sscanf(readbuffer, "%[a-zA-Z0-9]\n:%d-%d:%d\n", saved.code, (&saved.month), (&saved.year), (&saved.valid) );

            c1 = count_letters(saved.code);
            if ( (c1 == c2) && strncmp(readbuffer, buffer, c1) == 0){
                if (saved.valid == 1){ //Se è valido, controlla che non sia scaduto
                    if ( saved.year == (timeinfo->tm_year +1900) && saved.month < timeinfo->tm_mon){ //Scaduto
                        return 0;
                    }
                    if (saved.year < timeinfo->tm_year+1900){ //Scaduto
                        return 0;
                    }
                }
                return saved.valid;
            }
            memset(readbuffer,0,sz);
            memset(saved.code, 0, CODE_MAXSIZE);
        }
        fclose(data);    
    }
    return -1; //codice non registrato

}

int validate(char *buffer, size_t sz){
    struct data_formatv newd;
    struct data_formatv saved;
    newd.code = (char *)malloc(sizeof(char)*CODE_MAXSIZE);
    saved.code = (char *)malloc(sizeof(char)*CODE_MAXSIZE);

    FILE* data= fopen(filename,"r+"); //il file data è aperto in modalità scrittura e lettura in questo caso perché dobbiamo aggiornare delle stringhe
    int c=0; //c serve per tenere conto del numero di lettere in una stringa
    long offset = 0;

    char *readbuffer = (char *)malloc(sz * sizeof(char));

    sscanf(buffer, "%[a-zA-Z0-9]\n:%d", newd.code, (&newd.valid)); //Dal buffer dobbiamo prenderci il codice e validity

    c = count_letters(newd.code); //Il numero di lettere senza spazi o caratteri a capo, ci serve per la funzione strncmp

    if (data){
	    while(getline(&readbuffer, &sz, data) != -1){
            offset = ftell(data);

            //parsing della stringa e salvo nella struct i valori ricavati
            sscanf(readbuffer, "%[a-zA-Z0-9]:%d-%d:%d\n", saved.code, (&saved.month), (&saved.year), (&saved.valid)); 
            
            if ( strncmp(saved.code, newd.code, CODE_MAXSIZE) == 0){ //trovato il codice da dover validare/invalidare

                fseek(data, offset-2, SEEK_SET); //nel formato in cui sono salvati i dati, l'ultima variabile indica la validità (1) o meno (0)
                fprintf(data,"%d",newd.valid); //cambia l'ultima variabile con quella indicata
    
                fclose(data);
                return newd.valid;

            }
            memset(readbuffer,0,sz); //pulisci il buffer

        }
        fclose(data);      
    }else{
        perror("Non posso aprire il file\n");
    }
    return -1; //codice non trovato

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

        char i_buffer[BUFSIZE], o_buffer[BUFSIZE], l_buffer[BUFSIZE];
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

        fd_set set;
        int list_fd = server_sockfd;
        int fd=-1;
        int maxfd = list_fd; 
        int fd_open[FD_OPEN_MAXSIZE];

        for(int i=0;i<FD_OPEN_MAXSIZE;i++) fd_open[i]=0;

        int n=0,i=0;
        fd_open[maxfd] = 1;
        
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

                if (FD_ISSET(list_fd, &set)){ //Se il file descriptor in ascolto è settato
                    n--; //Decrementa il numero di file descriptor attivi
                    Accept(&fd, list_fd, ptr_client_address, &client_len); //Accetta la nuova connessione
                    FD_SET(fd, &set);
                    fd_open[fd]=1; //Segnala che il relativo file descriptor è correntemente aperto da una connessione
                    if(maxfd < fd)
                        maxfd = fd;
                }

                i = list_fd; //parti dal list_fd

                while(n!=0){ //Cicla finché ci sono connessioni attive
                    i++; 
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

                            strcpy(l_buffer, i_buffer);
                            if ( (strncmp(l_buffer,"CV",2)) == 0){
                                if ( save_data(l_buffer+3, DATA_FORMAT_MAXSIZE) == 0){
                                    snprintf(o_buffer, BUFSIZE, "Codice fiscale registrato con successo\n");
                                }else{
                                    snprintf(o_buffer, BUFSIZE, "Codice fiscale già inserito\n");
                                }
                            
                                FullWrite(i, o_buffer, BUFSIZE);

                            }else if( (strncmp(l_buffer,"CS",2)) == 0 ){

                                int e = is_valid(l_buffer+3, DATA_FORMAT_MAXSIZE); //Controlla la validità del certificato
                                if ( e == 1  ){
                                    snprintf(o_buffer, BUFSIZE, "Green Pass valido\n");
                                }else if( e == 0 ){
                                    snprintf(o_buffer, BUFSIZE, "Green Pass NON valido\n");
                                }else{
                                    snprintf(o_buffer, BUFSIZE, "Green Pass NON registrato: il codice fiscale non e' stato trovato\n");
                                }

                                FullWrite(i, o_buffer, (size_t)BUFSIZE);

                            }else if( (strncmp(l_buffer,"CT",2)) == 0){

                                int e = validate(l_buffer+3, DATA_FORMAT_MAXSIZE);
                                if (e == 0){
                                    snprintf(o_buffer, BUFSIZE, "Il green pass corrispondente è stato invalidato\n");
                                }else if (e==1){
                                    snprintf(o_buffer, BUFSIZE, "Il green pass corrispondente è ora valido\n");
                                }else{
                                    snprintf(o_buffer, BUFSIZE, "Green Pass NON registrato: il codice fiscale non e' stato trovato\n");
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
