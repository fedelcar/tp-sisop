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
#include <pthread.h>
#include <uncommons/SocketsServer.h>
#include <uncommons/SocketsBasic.h>
#include "nivelBase.h"
#include "movimiento.h"
#define MAXSIZE 1024
#define DOSPUNTOS ":"
#define HONGO "Hongos"
#define FLOR "Flores"
#define MONEDA "Monedas"

void openListener(char* argv, queue_n_locks *queue);

resource_struct *getLevelStructure(t_level_config *level_config, int *fd,
		pthread_mutex_t *resourcesReadLock, pthread_mutex_t *resourcesWriteLock);

ITEM_NIVEL* cambiarEstructura(t_level_config* levelConfig);

int main(int argc, char **argv) {

	t_dictionary *niveles = getLevels();

	char*mensaje = "nivel1";

	t_level_config *nivel = (t_level_config*) malloc(sizeof(t_level_config));

	nivel = dictionary_get(niveles, mensaje);

	ITEM_NIVEL *listaItems = cambiarEstructura(nivel);

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

	int *fd;

	openListener(mensaje, queue);

	pthread_mutex_t *resourcesReadLock = (pthread_mutex_t*) malloc(
			sizeof(pthread_mutex_t));
	pthread_mutex_t *resourcesWriteLock = (pthread_mutex_t*) malloc(
			sizeof(pthread_mutex_t));

	pthread_mutex_init(resourcesReadLock, NULL );

	pthread_mutex_init(resourcesWriteLock, NULL );

//	nivel_gui_inicializar();

	while (1) {

		if (queue_size(queue->character_queue) == 0) {
			pthread_mutex_lock(readLock);
		}

		printf("Paso el lock\n");

		pthread_mutex_lock(readLock);

		fd = (int *) queue_pop(queue->character_queue);

		resource_struct *resourceStruct = getLevelStructure(nivel, fd,
				resourcesReadLock, resourcesWriteLock);

		resourceStruct->listaItems = listaItems;

		pthread_t *t = (pthread_t*) malloc(sizeof(pthread_t));

		pthread_create(t, NULL, (void*) movimientoPersonaje,
				(resource_struct*) resourceStruct);

	}

	//TODO PENDIENTE BORRAR ITEMS

}

void openListener(char* mensaje, queue_n_locks *queue) {

	t_dictionary *levelsMap = dictionary_create();

	levelsMap = getLevelsMap();

	t_level_address *addresses = (t_level_address *) malloc(
			sizeof(t_level_address));

	addresses = (t_level_address*) dictionary_get(levelsMap, mensaje);

	char **port = string_split(addresses->nivel, DOSPUNTOS);

	queue->portNumber = port[1];

	pthread_t t;

	pthread_create(&t, NULL, (int *) openSocketServer, (queue_n_locks *) queue);

	free(port);
	free(levelsMap);
	free(addresses);
}

resource_struct *getLevelStructure(t_level_config *level_config, int *fd,
		pthread_mutex_t *resourcesReadLock, pthread_mutex_t *resourcesWriteLock) {

	resource_struct *resources = (resource_struct*) malloc(
			sizeof(resource_struct));

	resources->level_config = level_config;
	resources->readLock = resourcesReadLock;
	resources->writeLock = resourcesWriteLock;
	resources->fd = fd;

	return resources;
}

ITEM_NIVEL* cambiarEstructura(t_level_config* levelConfig) {

	char simbolo;
	ITEM_NIVEL* listaItems = (ITEM_NIVEL*) malloc(sizeof(ITEM_NIVEL));

	if (dictionary_has_key(levelConfig->cajas, FLOR)) {
		t_caja *caja = dictionary_get(levelConfig->cajas, FLOR);
		simbolo = 'F';
		CrearCaja(&listaItems, simbolo, (int) caja->posX, (int) caja->posY,
				(int) caja->instancias);
	}

	if (dictionary_has_key(levelConfig->cajas, MONEDA)) {
		t_caja *caja = dictionary_get(levelConfig->cajas, MONEDA);
		simbolo = 'M';
		CrearCaja(&listaItems, simbolo, (int) caja->posX, (int) caja->posY,
				(int) caja->instancias);
	}

	if (dictionary_has_key(levelConfig->cajas, HONGO)) {
		t_caja *caja = dictionary_get(levelConfig->cajas, HONGO);
		simbolo = 'H';
		CrearCaja(&listaItems, simbolo, (int) caja->posX, (int) caja->posY,
				(int) caja->instancias);
	}

	return listaItems;
}
