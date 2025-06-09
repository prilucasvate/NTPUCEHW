#include <stdio.h> 
#include <unistd.h> 
#include <string.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>

 //this is client part of bonus part 
 
int main(int argc, char **argv) {
    struct sockaddr_in server;
    int sockfd;
    char ibuf[20], obuf[20];
    while(1){
    server.sin_addr.s_addr = inet_addr("127.0.0.1"); //127.0.0.1: localhost address.
    server.sin_family = AF_INET;
    server.sin_port = htons(8888);
 
    //Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("Error: socket\n");
	return 0;
    }
     
 
    //Connect to remote server
    if (connect(sockfd, (struct sockaddr *)&server , sizeof(server))==-1)
    {
        printf("Error: connect\n");
        return 0;
    }
     
        //Communicate with server
        bzero(ibuf, sizeof(ibuf));
        bzero(obuf, sizeof(obuf));
        
        printf("[Input source node]: ");
        scanf("%s", ibuf);
        write(sockfd, ibuf, strlen(ibuf));     
        read(sockfd, obuf, sizeof(obuf));
        printf("[Shortest path]: %s\n",obuf);
        close(sockfd);
    }
    
    return 0;
}
