/*
 * planificador.c
 *
 *  Created on: Apr 19, 2013
 *      Author: lucas
 */

#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "uncommons/SocketsBasic.h"
#include "uncommons/SocketsServer.h"
#include "planificador.h"
#include "commons/string.h"

//TODO CHANGE SIGNALS VALUES AND RESPONSE LENGTH
#define SIGNAL_OK "OK"
#define SIGNAL_BLOCKED "BLOCKED"
#define MAXDATASIZE 1024
#define TERMINE_NIVEL "Termine nivel"

void socket_listener(queue_n_locks *queue);
void analize_response(char *response, queue_n_locks *queue, int *fd, pthread_mutex_t *readLock);

void planificador(t_scheduler_queue *scheduler_queue) {

	pthread_mutex_t *readLock = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
	pthread_mutex_t *writeLock = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));

	queue_n_locks *queue = (queue_n_locks*) malloc(sizeof(queue_n_locks));

	pthread_mutex_init(readLock, NULL);

	pthread_mutex_init(writeLock, NULL);

	queue->character_queue = scheduler_queue->character_queue;
	queue->blocked_queue = scheduler_queue->blocked_queue;
	queue->readLock = readLock;
	queue->writeLock = writeLock;
	queue->portNumber = scheduler_queue->port;

	socket_listener(queue);

	char *response = (char *) malloc(MAXDATASIZE); //CHECK LENGTH

	int *fd;

	t_level_attributes *level;

	int turno = 0;

	while (1) {

		level = getLevelAttributes();

		int turnoActual = atoi(level->turnos);

		int sleepTime = atoi(level->sleep);

		for(turno = 0 ; turno < turnoActual; turno++){

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

		analize_response(response, queue, fd, readLock);

		pthread_mutex_unlock(writeLock);

		sleep(sleepTime);

		}

	}

}

void socket_listener(queue_n_locks *queue) {

	pthread_t t;

	pthread_create(&t, NULL, (int *) openSocketServer,(queue_n_locks *)queue);

}

void analize_response(char *response, queue_n_locks *queue, int *fd, pthread_mutex_t *readLock) {

	if (string_equals_ignore_case(response, SIGNAL_OK)) {
		queue_push(queue->character_queue, fd);
	} else if (string_starts_with(response, SIGNAL_BLOCKED)) {
		response = string_substring_from(response, sizeof(SIGNAL_BLOCKED));
		blocked_character *blockedCharacter = (blocked_character*) malloc(sizeof(blocked_character));
		blockedCharacter->fd = fd;
		blockedCharacter->recurso = response[0];
		blockedCharacter->readlock = readLock;
		queue_push(queue->blocked_queue, blockedCharacter);
	}
	else if (string_equals_ignore_case(response, TERMINE_NIVEL)) {
		close(fd);
	}
}
