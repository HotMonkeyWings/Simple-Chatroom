#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>

void * doRecieving(void * sockID){

	int sockfd = *((int *) sockID);

	while(1){

		char data[2048];
		int read = recv(sockfd,data,2048,0);
		if (read == 0){
			printf("[-]Server has closed connection.\n");
			exit(1);
			break;
		}
		data[read] = '\0';
		printf("%s",data);
		
	}

}

int main(int argc, char * argv[]){

    int sockfd, newsockfd, portno, n;
    struct sockaddr_in serv_addr; //Gives internet address
    struct hostent *server;
    char name[1024];

    if(argc < 3){
        fprintf(stderr, "Port Number/Host name not provided. Exitting.\n");
        exit(1);
    }

    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
        fprintf(stderr,"ERROR opening socket.\n");
    }

    server = gethostbyname(argv[1]);
    if (server == NULL){
        fprintf(stderr, "Error, no such host. \n");
    }
    bzero((char*) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char*)server->h_addr, (char*) &serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
	if(connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1) 
		return 0;

	pthread_t thread;
	printf("Connection established\n");
    printf("Enter your name: ");
    scanf("%[^\n]s", name);
    send(sockfd, name,128, 0);
	printf("SEND <Message> : to send a message.\nLIST : to view all participants in the room.\nEXIT : to exit the chatroom.\n\n");

	pthread_create(&thread, NULL, doRecieving, (void *) &sockfd );
	while(1){

        char input[1024];
		scanf("%s",input);
			
		if(strcmp(input,"LIST") == 0){

			send(sockfd,input,1024,0);

		}
		if(strcmp(input,"SEND") == 0){

			send(sockfd,input,1024,0);
			
			//scanf("%s",input);
			//send(sockfd,input,1024,0);
			
			scanf("%[^\n]s",input);
			send(sockfd,input,1024,0);

		}
		if(strcmp(input,"EXIT") == 0){

			send(sockfd, input, 1024, 0);
			break;

		}

	}
	close(sockfd);
	exit(1);
	return(0);
}
