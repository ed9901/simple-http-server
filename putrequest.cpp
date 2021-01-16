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
#include "putrequest.h"

#define BUFFER_SIZE 16384
#define HEADER_SIZE 4096


void PUTrequest(int cl, char *filename, char *buffer, int &current_buffer_size, int &recvVal, int length){
	int openVal = open(filename, O_WRONLY | O_CREAT, S_IRWXU);
	char response[255];
	int sizeOfresp;
	if(openVal == -1){
		sizeOfresp = sprintf(response, "HTTP/1.1 500 Internal Server Error\r\nContent-Length: %d\r\n\r\n", 0);
		send(cl, response, sizeOfresp, 0);
	}
	else{
		if (current_buffer_size == 0){
			recvVal = recv(cl, buffer, BUFFER_SIZE, 0);
			current_buffer_size = recvVal;
		}
		int totalWrite = 0;
		char *p1, *p2;
		char temp[HEADER_SIZE];
		while(totalWrite != length){
			if(length - totalWrite <= current_buffer_size){
				p1 = buffer + (length-totalWrite);
				totalWrite += write(openVal, buffer, (length-totalWrite));
				p2 = buffer + current_buffer_size;
				current_buffer_size -= (p1 - buffer);
				if(current_buffer_size != 0){ 
					strncpy(temp, p1, p2-p1);
					strncpy(buffer, temp, current_buffer_size);
				}
			}
			else{
				totalWrite += write(openVal, buffer, current_buffer_size);
				recvVal = recv(cl, buffer, BUFFER_SIZE, 0);
				current_buffer_size = recvVal;
			}
		}
		close(openVal);
		sizeOfresp = sprintf(response, "HTTP/1.1 201 Created\r\nContent-Length: %d\r\n\r\n", 0);
		send(cl, response, sizeOfresp, 0);	
	}
}
