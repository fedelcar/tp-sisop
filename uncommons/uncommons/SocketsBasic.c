#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define BROKEN "BROKEN"
#define MAXDATASIZE 1024
#define GREAT "GREAT"

char* sendMessage(int *sockfd,char *msg)
{

	int len;
	len = strlen(msg);

	if(send(sockfd, msg, len, 0) == 0){
		return BROKEN;
	}
	return GREAT;
}


char* recieveMessage(int *sockfd)
{

	char *buffer = (char *) malloc(MAXDATASIZE);

	if(recv(sockfd,(char *) buffer, MAXDATASIZE, 0) == 0){
		return BROKEN;
	}

	return buffer;
}

void *get_in_addr(struct sockaddr *sa)
{
	return &(((struct sockaddr_in*)sa)->sin_addr);
}
