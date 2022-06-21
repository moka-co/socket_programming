#include <stdlib.h>
#include <stdio.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

void Listen(int sockfd){
    if ( listen(sockfd, 5) != 0){
        perror("Error on listening\n");
        exit(1);
    }

}
