/*
 * SocketsBasic.h
 *
 *  Created on: Apr 26, 2013
 *      Author: lucas
 */
#ifndef SOCKETSBASIC_H_
#define SOCKETSBASIC_H_


#include "commons/collections/queue.h"
#include <sys/socket.h>
#include <pthread.h>

typedef struct {
	t_queue *character_queue;
	pthread_mutex_t *readLock;
	pthread_mutex_t *writeLock;
} queue_n_locks;
/*
 * Sends a message given a socket.
 * msg stands from the message you want to send.
 * sockfd stands for the file descriptor of the socket you want
 * 		  to send the message.
 */
void sendMessage(int *sockfd, char *msg);


/*
 * Given a socket, you recieve a message.
 * sockfd stands for the file descriptor of the socket you want
 * 		  to recieve messages.
 */

char* recieveMessage(int *sockfd);

/*
 * Gets the sockaddr.
 */
void *get_in_addr(struct sockaddr *sa);

#endif SOCKETSBASIC_H_
