/*
 * orquestador.c
 *
 *  Created on: May 1, 2013
 *      Author: lucas
 */
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include "planificador.h"
#include <commons/collections/dictionary.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <pthread.h>
#include "uncommons/SocketsBasic.h"
#include "uncommons/SocketsServer.h"
#include "uncommons/SocketsCliente.h"
#include "commons/string.h"
#include <unistd.h>

#define DOSPUNTOS ":"
#define MAXSIZE 1024
#define PORT "9930"
#define LEVEL "LVL"
#define PIPE "|"
#define FREERESC "FREERESC"
#define COMA ","
#define OKEY "ok"

void executeResponse(char* response, t_dictionary *levelsMap, int *fd, t_dictionary *levels_queues);

void main() {

	/**
	 * Lista de niveles
	 */

	t_list *levelsList = getLevelsList();

	/**
	 * Usado para retener nombres de niveles
	 */

	char *levelName = (char *) malloc(MAXSIZE);

	/**
	 *Usado para retener addresses
	 */

	t_level_address *addresses = (t_level_address *) malloc(
			sizeof(t_level_address));

	/**
	 * Mapa de niveles
	 */
	t_dictionary *levelsMap = dictionary_create();

	levelsMap = getLevelsMap();

	char** port = (char*) malloc(MAXSIZE);

	int i;

	t_dictionary *levels_queues = dictionary_create();

	for (i = 0; i < list_size(levelsList); i++) {

		pthread_t *t = (pthread_t*) malloc(sizeof(pthread_t));

		levelName = (char *) list_get(levelsList, i);

		addresses = (t_level_address*) dictionary_get(levelsMap, levelName);

		port = string_split(addresses->planificador, DOSPUNTOS);

		t_scheduler_queue *scheduler_queue = (t_scheduler_queue*) malloc(sizeof(t_scheduler_queue));

		scheduler_queue->blocked_queue = queue_create();
		scheduler_queue->character_queue = queue_create();
		scheduler_queue->port = port[1];

		dictionary_put(levels_queues, levelName, scheduler_queue);

		pthread_create(t, NULL, (void *) planificador, (t_scheduler_queue*) scheduler_queue);

	}

	free(levelName);
	free(addresses);
	free(levelsList);

	pthread_mutex_t *readLock = (pthread_mutex_t*) malloc(
			sizeof(pthread_mutex_t));
	pthread_mutex_t *writeLock = (pthread_mutex_t*) malloc(
			sizeof(pthread_mutex_t));

	queue_n_locks *queue = (queue_n_locks*) malloc(sizeof(queue_n_locks));

	pthread_mutex_init(readLock, NULL );

	pthread_mutex_init(writeLock, NULL );

	queue->character_queue = queue_create();
	queue->readLock = readLock;
	queue->writeLock = writeLock;
	queue->portNumber = PORT;

	pthread_t t;

	pthread_create(&t, NULL, (int *) openSocketServer, (queue_n_locks *) queue);

	char *response = (char *) malloc(MAXSIZE); //CHECK LENGTH
	int *fd;

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

		response = recieveMessage(fd);

		executeResponse(response, levelsMap, fd, levels_queues);

	}

}

void executeResponse(char* response, t_dictionary *levelsMap, int *fd, t_dictionary *levels_queues) {

	if (string_starts_with(response, LEVEL)) {
		response = string_substring_from(response, sizeof(LEVEL));
		t_level_address *level = (t_level_address *) malloc(
				sizeof(t_level_address));
		char *socketsToGo = (char*) malloc(MAXSIZE);
		memset(socketsToGo, 0, sizeof(socketsToGo));
		string_append(&socketsToGo,
				((t_level_address*) (dictionary_get(levelsMap,
						(string_split(response, PIPE))[0])))->planificador);
		string_append(&socketsToGo, COMA);
		string_append(&socketsToGo,
				((t_level_address*) (dictionary_get(levelsMap,
						(string_split(response, PIPE))[0])))->nivel);
		sendMessage(fd, socketsToGo);
		free(socketsToGo);
		free(response);
		free(level);
		close(fd);
	} else if (string_starts_with(response, FREERESC)) {

		response = string_substring_from(response, sizeof(FREERESC));

		char** data = (char*) malloc(MAXSIZE);

		data = string_split(response, COMA);

		t_scheduler_queue *queues = dictionary_get(levels_queues, data[0]);

		if(queue_size(queues->blocked_queue) == 0){
			sendMessage(fd, OKEY);
		}
	}

}
