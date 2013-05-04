/*
 * planificador.c
 *
 *  Created on: Apr 19, 2013
 *      Author: lucas
 */

#include <stdlib.h>
#include <pthread.h>
#include <commons/collections/queue.h>
#include <unistd.h>
#include "commons/SocketsBasic.h"
#include "commons/SocketsServer.h"
#include "planificador.h"
#include "commons/string.h"

//TODO CHANGE SIGNALS VALUES AND RESPONSE LENGTH
#define SIGNAL_OK "OK"
#define SIGNAL_BLOCKED "BLOCKED"
#define MAXDATASIZE 1024

void socket_listener(char* socket, queue_n_locks *queue);
void analize_response(char *response, t_queue *character_queue, int *fd);

void planificador(struct scheduler_struct *schedulerStruct) {

	pthread_mutex_t *readLock = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
	pthread_mutex_t *writeLock = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));

	queue_n_locks *queue = (queue_n_locks*) malloc(sizeof(queue_n_locks));

	pthread_mutex_init(readLock, NULL);

	pthread_mutex_init(writeLock, NULL);

	queue->character_queue = queue_create();
	queue->readLock = readLock;
	queue->writeLock = writeLock;

	socket_listener(schedulerStruct->port, queue);

	char *response = (char *) malloc(MAXDATASIZE); //CHECK LENGTH

	int *fd; //CHECK

	while (1) {

		printf("Entro al while\n");

		if (queue_size(queue->character_queue) == 0) {
			pthread_mutex_lock(readLock);
		}

		printf("Paso el lock\n");

		pthread_mutex_lock(readLock);

		fd = (int *) queue_pop(queue->character_queue);

		printf("Realizo el pop\n");

		pthread_mutex_unlock(readLock);

		sendMessage(fd, "Paso por el planificador\n");

		printf("Mando mensaje\n");

		response = recieveMessage(fd);

		pthread_mutex_lock(writeLock);

		printf("Respondio mensaje\n");

		analize_response(response, queue->character_queue, fd);

		pthread_mutex_unlock(writeLock);

		sleep(schedulerStruct->timeToWait);

	}

}

void socket_listener(char* socket, queue_n_locks *queue) {

	pthread_t t;

	pthread_create(&t, NULL, (int *) openSocketServer,(queue_n_locks *)queue);

}

void analize_response(char *response, t_queue *character_queue, int *fd) {

	if (string_equals_ignore_case(response, SIGNAL_OK)) {
		queue_push(character_queue, fd);
	} else if (string_equals_ignore_case(response, SIGNAL_BLOCKED)) {
		close(*fd); //TODO PUSH FD TO THE OTHER QUEUE
	}
	else {
		printf("No se loco.\n");
	}
}
