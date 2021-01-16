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
#include "getrequest.h"

#define BUFFER_SIZE 16384
#define HEADER_SIZE 4096

void GETrequest(int cl, char *filename){
	int openVal = open(filename, O_RDONLY);
	char response[255];
	char readBuffer[BUFFER_SIZE];
	int sizeOfresp;
	if(openVal == -1){
		sizeOfresp = sprintf(response, "HTTP/1.1 404 Not Found\r\nContent-Length: %d\r\n\r\n", 0);
		send(cl, response, sizeOfresp, 0);
	}
	else{
		int readVal = read(openVal, readBuffer, BUFFER_SIZE);
		if(readVal == -1){
			sizeOfresp = sprintf(response, "HTTP/1.1 403 Forbidden\r\nContent-Length: %d\r\n\r\n", 0);
			send(cl, response, sizeOfresp, 0);
		}
		else{
			int totalRead = readVal;
			while(readVal == BUFFER_SIZE){
			readVal = read(openVal, readBuffer, BUFFER_SIZE);
			totalRead += readVal;
			}
			sizeOfresp = sprintf(response, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n", totalRead);
			send(cl, response, sizeOfresp, 0);
			close(openVal);
			openVal = open(filename, O_RDONLY);
			readVal = read(openVal, readBuffer, BUFFER_SIZE);
			send(cl, readBuffer, readVal, 0);
			while(readVal == BUFFER_SIZE){
				readVal = read(openVal, readBuffer, BUFFER_SIZE);
				send(cl, readBuffer, readVal, 0);
			}
		}
		close(openVal);					
	}	
}
