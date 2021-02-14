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

int clientCount = 0;

struct client{

	int index;
	int sockfd;
	struct sockaddr_in clientAddr;
	int len;
	int alive;
	char name[1024];
};

struct client Client[1024];
pthread_t thread[1024];
pthread_t close_t;
pthread_mutex_t lock;


void * doNetworking(void * ClientDetail){

	struct client* clientDetail = (struct client*) ClientDetail;
	int index = clientDetail -> index;
	int clientSocket = clientDetail -> sockfd;
	clientDetail -> alive = 1;
	char name[128];
	recv(clientSocket, name, 128, 0);
	strcpy(clientDetail -> name, name);


	//Announce Client has joined.
	char joinmsg[2048];
	snprintf(joinmsg, 2048, "[+]%s has connected.\n", name);
	printf("%s", joinmsg);		
	for (int i = 0; i < clientCount; i++){
		if (i != index && Client[i].alive != 0)
		send(Client[i].sockfd, joinmsg, 2048, 0);
	}

	

	while(1){

		char data[1024];
		char msg[2048];
		int read = recv(clientSocket,data,1024,0);
		data[read] = '\0';

		char output[1024];

		if(strcmp(data,"LIST") == 0){
			pthread_mutex_lock(&lock);
			int l = 0;

			for(int i = 0 ; i < clientCount ; i ++){
				
			  if(Client[i].alive == 0){
				  continue;
			  }



			  else if(i != index)
					l += snprintf(output + l,1024,"%s on socket %d.\n",Client[i].name,Client[i].sockfd);

				
			  else
					l += snprintf(output + l,1024,"(You)%s on socket %d.\n",name,clientSocket);

			}

			send(clientSocket,output,1024,0);
			pthread_mutex_unlock(&lock);
			continue;

		}
		if(strcmp(data,"SEND") == 0){

			pthread_mutex_lock(&lock);
			bzero(data, 1024);

			read = recv(clientSocket,data,1024,0);
			data[read] = '\0';
			
			snprintf(msg, 2048, "%s: %s\n", name, data);

			for (int i = 0; i < clientCount; i++){
					if (i != index && Client[i].alive != 0)
							send(Client[i].sockfd, msg, 2048, 0);
			}

			pthread_mutex_unlock(&lock);
		}
		
		if(strcmp(data, "EXIT") == 0 || read == 0){
			pthread_mutex_lock(&lock);
			Client[index].alive = 0;
			printf("[-]%s has left the room.\n", name);
			snprintf(data, 1024, "[-]%s has left the room.\n", name);		
			for (int i = 0; i < clientCount; i++){
					if (i != index && Client[i].alive != 0)
							send(Client[i].sockfd, data, 1024, 0);
			}
			pthread_mutex_unlock(&lock);
			bzero(data, 1024);
			int *temp;
			pthread_exit(temp);
		}
	}
	fprintf(stderr,"Here somehow");
	return NULL;

}

void error (const char *msg){
    perror(msg);
    exit(1);
}

void * closeServer(void* sockfd){
  char closer[1024];
  while(1){
	scanf("%s",closer);
	if(strcmp(closer,"CLOSE") == 0)
	  break;
  }
	int sockfd1 = *((int *) sockfd);
	close(sockfd1);
	printf("Server shutting down.\n");
	exit(1);
}

int main(int argc, char * argv[]){
	
    if (argc < 2){
        fprintf(stderr, "Port Number was not provided. Exitting.\n");
        exit(1);
    }

    int sockfd, newsockfd, portno, n;
    
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serv_addr;
    
    
    
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[1]));
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	if(bind(sockfd,(struct sockaddr *) &serv_addr , sizeof(serv_addr)) < 0) {
  		error("Binding Failed.");
  }
    

	listen(sockfd,1024);

	//Checking if the server is to be closed
	pthread_create(&close_t, NULL, closeServer,(void*) &sockfd);

	printf("[+]Server started listenting on port %s\n", argv[1]);

	while(1){

		Client[clientCount].sockfd = accept(sockfd, (struct sockaddr*) &Client[clientCount].clientAddr, &Client[clientCount].len);
		Client[clientCount].index = clientCount;
		//Creates a client thread.
		pthread_create(&thread[clientCount], NULL, doNetworking, (void *) &Client[clientCount]);

		clientCount ++;
 
	}


}
