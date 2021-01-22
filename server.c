/*
TCP_Server. This Program will will create the Server side for TCP_Socket Programming.
It will receive the data from the client and then send the same data back to client.
*/

#include <stdio.h> 
#include <string.h> 
#include <sys/socket.h> //socket
#include <arpa/inet.h> //inet_addr
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <pthread.h>
#define MAXSEATS 20

pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;

int seat[MAXSEATS]={0};

void* firstClass(void* par){
	// ftok to generate unique key 
    key_t key = ftok("shmfile",65); 
  
    // shmget returns an identifier in shmid 
    int shmid = shmget(key,1024,0666|IPC_CREAT); 
  
    // shmat to attach to shared memory 
    char *str = (char*) shmat(shmid,(void*)0,0);
    if(!strcmp(str,"1")){
    
		printf("\nSeats available: ");
		int i=0;
		for(i=0;i<MAXSEATS/2;i++){
			if(seat[i]==0){
				printf("%d",i+1);
			}
		}
		int no=0;
		do{
			printf("\nEnter seat no: ");
			scanf("%d",&no);
			if(no<=0||no>MAXSEATS/2)
				printf("Wrong Selection\n");
			else if(seat[no-1]==1)
				printf("Seat already filled\n");
		}while(seat[no-1]==1||no<=0||no>MAXSEATS/2);
		pthread_mutex_lock(&mutex);
		seat[no-1]=1;
		pthread_mutex_unlock(&mutex);
		printf("\n");
		printf("--------------------------\n");
		printf("First class seat confirmed\n");
		printf("Seat no : %d\n",no);
		printf("--------------------------\n\n");
	}
	pthread_exit(NULL);
}

void* secondClass(void* par){
	// ftok to generate unique key 
    key_t key = ftok("shmfile",65); 
  
    // shmget returns an identifier in shmid 
    int shmid = shmget(key,1024,0666|IPC_CREAT); 
  
    // shmat to attach to shared memory 
    char *str = (char*) shmat(shmid,(void*)0,0);
    if(!strcmp(str,"2")){
		printf("\nSeats available: ");
		int i=0;
		for(i=MAXSEATS/2;i<MAXSEATS;i++){
			if(seat[i]==0){
				if(i!=MAXSEATS-1)
					printf("%d ",i+1);
				else
					printf("%d",i+1);
			}
		}
		int no=0;
		do{
			printf("\nEnter seat no: ");
			scanf("%d",&no);
			if(no<=MAXSEATS/2||no>MAXSEATS)
				printf("Wrong Selection\n");
			else if(seat[no-1]==1)
				printf("Seat already filled\n");
		}while(seat[no-1]==1||no<=MAXSEATS/2||no>MAXSEATS);
		pthread_mutex_lock(&mutex);
		seat[no-1]=1;
		pthread_mutex_unlock(&mutex);
		printf("\n");
		printf("--------------------------\n");
		printf("Second class seat confirmed\n");
		printf("Seat no : %d\n",no);
		printf("--------------------------\n\n");
	}
	pthread_exit(NULL);
}


int main(void)
{
	int socket_desc, client_sock, client_size;
	struct sockaddr_in server_addr, client_addr;         //SERVER ADDR will have all the server address
	char server_message[2000], client_message[2000];    // Sending values from the server and receive from the server we need this

	//Cleaning the Buffers

	memset(server_message, '\0', sizeof(server_message));
	memset(client_message, '\0', sizeof(client_message));     // Set all bits of the padding field//

	 //Creating Socket

	socket_desc = socket(AF_INET, SOCK_STREAM, 0);

	if (socket_desc < 0)
	{
		printf("Could Not Create Socket. Error!!!!!\n");
		return -1;
	}

	printf("Socket Created\n");

	//Binding IP and Port to socket

	server_addr.sin_family = AF_INET;               /* Address family = Internet */
	server_addr.sin_port = htons(2000);               // Set port number, using htons function to use proper byte order */
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");    /* Set IP address to localhost */



	 // BINDING FUNCTION

	if (bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr))<0)    // Bind the address struct to the socket.  /
   //bind() passes file descriptor, the address structure,and the length of the address structure
	{
		printf("Bind Failed. Error!!!!!\n");
		return -1;
	}

	printf("Bind Done\n");

	//Put the socket into Listening State

	if (listen(socket_desc, 1) < 0)                               //This listen() call tells the socket to listen to the incoming connections.
 // The listen() function places all incoming connection into a "backlog queue" until accept() call accepts the connection.
	{
		printf("Listening Failed. Error!!!!!\n");
		return -1;
	}
	while (1) {
		printf("Listening for Incoming Connections.....\n");

		//Accept the incoming Connections
		client_size = sizeof(client_addr);
		client_sock = accept(socket_desc, (struct sockaddr*)&client_addr, &client_size); // creating socket for particular client

		if (client_sock < 0)
		{
			printf("Accept Failed. Error!!!!!!\n");
			return -1;
		}

		printf("Client Connected with IP: %s and Port No: %i\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
	
		//inet_ntoa() function converts the Internet host address in, given in network byte order, to a string in IPv4 dotted-decimal notation

		//Receive the message from the client

		if (recv(client_sock, client_message, sizeof(client_message), 0) < 0)
		{
			printf("Receive Failed. Error!!!!!\n");
			return -1;
		}
		
		// ftok to generate unique key 
		key_t key = ftok("shmfile",65); 

		// shmget returns an identifier in shmid 
		int shmid = shmget(key,1024,0666|IPC_CREAT); 

		// shmat to attach to shared memory 
		char *str = (char*) shmat(shmid,(void*)0,0);
		
		strcpy(str,client_message);
		
		if(!strcmp(str,"1")){
			printf("Passenger choice: First class\n");	
			pthread_t fid;
			pthread_create(&fid,NULL,&firstClass,NULL);
			pthread_join(fid,NULL);
		}
		else if(!strcmp(str,"2")){
			printf("Passenger choice: Second class\n");	
			pthread_t sid;
			pthread_create(&sid,NULL,&secondClass,NULL);
			pthread_join(sid,NULL);
		}
		else{
			printf("Wrong Choice\n");
		}

		//Send the message back to client
		strcpy(server_message, "Booking Confirmed");

		if (send(client_sock, server_message, strlen(server_message), 0)<0)
		{
			printf("Send Failed. Error!!!!!\n");
			return -1;
		}
		
		shmdt(str); 
		// destroy the shared memory 
		shmctl(shmid,IPC_RMID,NULL);

		memset(server_message, '\0', sizeof(server_message));
		memset(client_message, '\0', sizeof(client_message));
	}
	//Closing the Socket
	close(client_sock);
	close(socket_desc);
	return 0;
}

