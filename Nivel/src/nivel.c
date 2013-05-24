/*
 * nivel.c
 *
 *  Created on: May 12, 2013
 *      Author: lucas
 */

#include <stdio.h>
#include <stdlib.h>
#include <commons/collections/dictionary.h>
#include <commons/string.h>
#include <uncommons/fileStructures.h>
#include <pthread.h>
#include <uncommons/SocketsServer.h>
#include <uncommons/SocketsBasic.h>
#define MAXSIZE 1024
#define DOSPUNTOS ":"

void openListener(char* argv, queue_n_locks *queue );

int main(char *argv) {

	t_dictionary *niveles = getLevels();

	t_level_config *nivel = (t_level_config*) malloc(sizeof(t_level_config));

	nivel = dictionary_get(niveles, argv);

	pthread_mutex_t *readLock = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
	pthread_mutex_t *writeLock = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));

	queue_n_locks *queue = (queue_n_locks*) malloc(sizeof(queue_n_locks));

	pthread_mutex_init(readLock, NULL);

	pthread_mutex_init(writeLock, NULL);

	queue->character_queue = queue_create();
	queue->readLock = readLock;
	queue->writeLock = writeLock;

	int *fd;

	openListener(argv, queue);

	while(1){

		if (queue_size(queue->character_queue) == 0) {
			pthread_mutex_lock(readLock);
		}

		printf("Paso el lock\n");

		pthread_mutex_lock(readLock);

		fd = (int *) queue_pop(queue->character_queue);


	}

}


void openListener(char* argv, queue_n_locks *queue){

	t_dictionary *levelsMap = dictionary_create();

	levelsMap = getLevelsMap();

	t_level_address *addresses = (t_level_address *) malloc(sizeof(t_level_address));

	addresses = (t_level_address*) dictionary_get(levelsMap, argv);

	char **port = string_split(addresses->nivel, DOSPUNTOS);

	queue->portNumber = port[1];

	pthread_t t;

	pthread_create(&t, NULL, (int *) openSocketServer,(queue_n_locks *)queue);

	free(port);
	free(levelsMap);
	free(addresses);
}
