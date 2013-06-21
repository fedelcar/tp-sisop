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
#include <commons/collections/queue.h>
#include "uncommons/fileStructures.h"
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include "planificador.h"
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
static const char* COMA = ",";

void executeResponse(char* response, t_dictionary *levelsMap, int *fd);

void main() {

	/*
	 * Lista de personajes bloqueados
	 */
	t_queue *blocked_characters = queue_create();

	/**
	 * No me acuerdo que es request, TODO investigar
	 */

	t_queue *request = queue_create();

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

	for (i = 0; i < list_size(levelsList); i++) {

		pthread_t *t = (pthread_t*) malloc(sizeof(pthread_t));

		levelName = (char *) list_get(levelsList, i);

		addresses = (t_level_address*) dictionary_get(levelsMap, levelName);

		port = string_split(addresses->planificador, DOSPUNTOS);

		pthread_create(t, NULL, (void *) planificador, port[1]);

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
		memset(response, 0, sizeof(response));
		response = recieveMessage(fd);

		executeResponse(response, levelsMap, fd);

	}

}

void executeResponse(char* response, t_dictionary *levelsMap, int *fd) {

	if (string_starts_with(response, LEVEL)) {
		response = string_substring_from(response, sizeof(LEVEL));
		t_level_address *level = (t_level_address *) malloc(
				sizeof(t_level_address));
		char *socketsToGo = (char*) malloc(MAXSIZE);
		memset(socketsToGo, 0, sizeof(socketsToGo));
//		memcpy(level->nivel, ((t_level_address*) (dictionary_get(levelsMap, (string_split(response, "|"))[0])))->nivel, sizeof(((t_level_address*) (dictionary_get(levelsMap, (string_split(response, "|"))[0])))->nivel));
//		memcpy(level->planificador, ((t_level_address*) (dictionary_get(levelsMap, (string_split(response, "|"))[0])))->planificador, sizeof(((t_level_address*) (dictionary_get(levelsMap, (string_split(response, "|"))[0])))->planificador));
//		level = dictionary_get(levelsMap, (string_split(response, "|"))[0]);
		string_append(&socketsToGo, ((t_level_address*) (dictionary_get(levelsMap, (string_split(response, "|"))[0])))->planificador);
		string_append(&socketsToGo, COMA);
		string_append(&socketsToGo, ((t_level_address*) (dictionary_get(levelsMap, (string_split(response, "|"))[0])))->nivel);
		sendMessage(fd, socketsToGo);
		free(socketsToGo);
		free(response);
		free(level);
		close(fd);
	} else {
		//TODO OTHER ACTIONS
	}

}
