#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAXDATASIZE 1024

void sendMessage(int *sockfd,char *msg)
{

	int len;
	len = strlen(msg);

	send(sockfd, msg, len, 0);
}


char* recieveMessage(int *sockfd)
{

	char *buffer = (char *) malloc(MAXDATASIZE);

	recv(sockfd,(char *) buffer, MAXDATASIZE, 0);

	return buffer;
}

void *get_in_addr(struct sockaddr *sa)
{
	return &(((struct sockaddr_in*)sa)->sin_addr);
}
