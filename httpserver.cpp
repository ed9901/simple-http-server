#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdbool.h>
#include <ctype.h>
#include <pthread.h>
#include <semaphore.h>
#include "getrequest.h"
#include "putrequest.h"


#define BUFFER_SIZE 16384
#define HEADER_SIZE 4096

void *handleClient(void* ptr);

pthread_mutex_t mutex1;
sem_t threadsAvailable, jobsAvailable;

int main(int argc, char *argv[]){

	/* Set up cmd-line arguments and program options */
	int opt, numThreads = 4;
	char *hostname;
	char *port;
	while((opt = getopt(argc, argv, "N:")) != -1)
	{
		switch(opt){
			case 'N':
					sscanf(optarg, "%d", &numThreads);
					break;
		}

	}
	if(argv[optind] == NULL){
		fprintf(stderr, "Fatal error: must specify hostname\n");
		exit(-1);
	}
	else{
		hostname = argv[optind];
		if(argv[optind+1] == NULL){
			port = (char*)"80";
		}
		else{
			port = argv[optind+1];
		}
	}

	/* Set up socket for Http server */
	int errorVal;
	struct addrinfo *addrs, hints = {};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	errorVal = getaddrinfo(hostname, port, &hints, &addrs);
	if(errorVal != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(errorVal) );
		exit(-1);
	}
	int main_socket = socket(addrs->ai_family, addrs->ai_socktype, addrs->ai_protocol);
	int enable = 1;
	errorVal = setsockopt(main_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
	if(errorVal == -1){
		fprintf(stderr, "setsockopt: %s\n", strerror(errno) );
		exit(-1);
	}
	errorVal = bind(main_socket, addrs->ai_addr, addrs->ai_addrlen);
	if(errorVal == -1){
		fprintf(stderr, "bind: %s\n", strerror(errno) );
		exit(-1);
	}
	errorVal = listen (main_socket, 16);
	if(errorVal == -1){
		fprintf(stderr, "listen: %s\n", strerror(errno) );
		exit(-1);
	}

	/* Initialize semephores and mutex*/
	sem_init(&threadsAvailable, 0 , numThreads);
	sem_init(&jobsAvailable, 0 , 0);
	pthread_mutex_init(&mutex1, NULL);

	int connection_fd = 0;
	int *client = &connection_fd;

	pthread_t *pool = (pthread_t *)malloc(numThreads * sizeof(pthread_t));
	for(int i = 0; i < numThreads; i++){
		pthread_create(&pool[i], NULL, handleClient, (void *)client);
	}

	/* main loop to dispatch threads when connections are accepted */
	while(true){
		int connection = accept(main_socket, NULL, NULL);
		sem_wait(&threadsAvailable);
		pthread_mutex_lock(&mutex1);
		connection_fd = connection;
		sem_post(&jobsAvailable);
	}
	return 0;

}


/****************************************************************************
*  Main client handler function that parses the given client's HTTP-header  *
*  and calls the corresponding request function 			    			*
****************************************************************************/
void *handleClient(void *ptr){

	char buffer [BUFFER_SIZE];
	char temp   [BUFFER_SIZE];
	char header [HEADER_SIZE];

	int size_of_recv, current_buffer_size;


	int *client;
	client = (int *) ptr;
	while(true){
		sem_wait(&jobsAvailable);
		int cl = *client;
		pthread_mutex_unlock(&mutex1);
		
		size_of_recv = recv(cl, buffer, BUFFER_SIZE, 0);
		current_buffer_size = size_of_recv;
		/* While the connection is open */
		while( size_of_recv > 0){
			char *end_of_header = NULL;
			char *p1 = NULL;
			char *p2 = NULL;

			/* Begin to recv data into buffer until the entire HTTP-header is recv'd */
			end_of_header = strstr(buffer, "\r\n\r\n");
			while(end_of_header == NULL && size_of_recv > 0){
				p1 = buffer + current_buffer_size;
				size_of_recv = recv(cl, p1, BUFFER_SIZE-current_buffer_size, 0);
				current_buffer_size += size_of_recv;
				end_of_header = strstr(buffer, "\r\n\r\n");
			}

			/*HTTP-header is now ready to be parsed*/
			if(end_of_header != NULL){
				strncpy(header, buffer, (end_of_header+4) - buffer);
				p2 = buffer + current_buffer_size;
				current_buffer_size -= ((end_of_header+4) - buffer );
				if(current_buffer_size != 0){
					strncpy(temp, (end_of_header + 4), p2 - (end_of_header +4));
					strncpy(buffer, temp, current_buffer_size);
				}
				char req [10];
				char filename [HEADER_SIZE - 20];
				char version [10];
				p1 = strstr(header, "\r\n");
				p2 = strstr(header, "\r\n\r\n");
				sscanf(header, "%s %s %s", req, filename, version);

				/* If req is GET, call GETrequest() function to handle a GET request*/
				if(strcmp (req, "GET") == 0 ){
					GETrequest(cl, filename);
				}

				/* Else if req is PUT, extract its content length and call PUTrequest()*/
				else if(strcmp (req, "PUT") == 0){
					bool useLen = false;
					int length;
					char token[20];
					while(p1 != p2 && useLen == false){
						p1 = p1 + 2;
						sscanf(p1, "%s %d", token, &length);
						if(strcasecmp(token, "content-length:") == 0){
							useLen = true;
						}
						p1 = strstr(p1, "\r\n");
					}
					if(useLen == true){
						PUTrequest(cl, filename, buffer, current_buffer_size, size_of_recv, length);
					}
				}
				/* Else Bad request */
				else{
					char response [255];
					int sizeOfresp = sprintf(response, "HTTP/1.1 400 Bad request\r\nContent-Length: %d\r\n\r\n", 0);
					send(cl, response, sizeOfresp, 0);
				}
				if(current_buffer_size == 0){
					size_of_recv = recv(cl, buffer, BUFFER_SIZE, 0);
					current_buffer_size = size_of_recv;
				}
				char *clearer = buffer + current_buffer_size;
				memset(clearer, 0, BUFFER_SIZE - current_buffer_size);
			}	
		}
		sem_post(&threadsAvailable);
	}
}
