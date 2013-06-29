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
#include <uncommons/SocketsServer.h>
#include <uncommons/SocketsBasic.h>
#include <uncommons/SocketsCliente.h>
#include <string.h>
#include <unistd.h>
#include "nivelBase.h"
#include "movimiento.h"
#include "interbloqueo.h"
#define MAXSIZE 1024
#define DOSPUNTOS ":"
#define EMPTY "EMPTY"
#define HONGO "Hongos"
#define FLOR "Flores"
#define MONEDA "Monedas"
#define START "START"
#define RESOURCES "RSC"
#define COMA ","
#define DEATH "DEATH"


void openListener(char* argv, queue_n_locks *queue);
void agregarRecursos(char* buffer, ITEM_NIVEL *listaItems);
void saveList(pthread_t *t, resource_struct *resourceStruct, t_list *threads,
		int id);
void deteccionInterbloqueo(deadlock_struct *deadlockStruct);

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

	char* bufferSocket = (char*) malloc(MAXSIZE);

	t_list *threads = list_create();

	int id = 0;

	pthread_t detectionThread;

	deadlock_struct *deadlockStruct = (deadlock_struct*) malloc(
			sizeof(deadlock_struct));

	deadlockStruct->items = listaItems;
	deadlockStruct->list = threads;
	deadlockStruct->socket = nivel->orquestador;
	deadlockStruct->recovery = nivel->recovery;

	pthread_create(&detectionThread, NULL, (void*) deteccionInterbloqueo,
			(deadlock_struct*) deadlockStruct);

	nivel_gui_inicializar();

	while (1) {

		if (queue_size(queue->character_queue) == 0) {
			pthread_mutex_lock(readLock);
		}

		pthread_mutex_lock(readLock);

		fd = (int *) queue_pop(queue->character_queue);

		bufferSocket = recieveMessage(fd);

		if (string_starts_with(bufferSocket, START)) {

			recursos_otorgados * recursosAt = (recursos_otorgados*) malloc(
					sizeof(recursos_otorgados));
			recursosAt->F = 0;
			recursosAt->H = 0;
			recursosAt->M = 0;

			resource_struct *resourceStruct = getLevelStructure(nivel, fd,
					resourcesReadLock, resourcesWriteLock);

			resourceStruct->recursosAt = recursosAt;

			resourceStruct->recursoBloqueado = "0";

			resourceStruct->listaItems = listaItems;

			pthread_t *t = (pthread_t*) malloc(sizeof(pthread_t));

			saveList(t, resourceStruct, threads, id);

			pthread_create(t, NULL, (void*) movimientoPersonaje,
					(resource_struct*) resourceStruct);

			id++;
		} else if (string_starts_with(bufferSocket, RESOURCES)) {
			agregarRecursos(bufferSocket, listaItems);
		}

		pthread_mutex_unlock(readLock);
	}

	//TODO PENDIENTE BORRAR ITEMS

}

void agregarRecursos(char* buffer, ITEM_NIVEL *listaItems) {

	buffer = string_substring_from(buffer, sizeof(RESOURCES));

	char** data = string_split(buffer, COMA);

	ITEM_NIVEL* temp;

	temp = buscarRecurso(listaItems, 'F');
	temp->quantity = temp->quantity + atoi(data[0]);

	temp = buscarRecurso(listaItems, 'M');
	temp->quantity = temp->quantity + atoi(data[1]);

	temp = buscarRecurso(listaItems, 'H');
	temp->quantity = temp->quantity + atoi(data[2]);

	free(data);
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

void deteccionInterbloqueo(deadlock_struct *deadlockStruct) {

	char **ipPuerto = string_split(deadlockStruct->socket, DOSPUNTOS);

	char *bufferDeadlock = (char*) malloc(MAXSIZE);

	int socketDeadlock = openSocketClient(ipPuerto[1], ipPuerto[0]);
	sendMessage(socketDeadlock, EMPTY);

	int j = 0;

	char *deadlockMessage = (char*) malloc(MAXSIZE);

	t_list *deadlockList;
	while (1) {

	sleep(3);//levantar de archivo de configuracion, inotify

		deadlockList = detectionAlgorithm(deadlockStruct->items, deadlockStruct->list);

		if (list_size(deadlockList) > 1 && atoi(deadlockStruct->recovery) == 1) {

			deadlockMessage = string_from_format("DEADLOCK,%d,",
					list_size(deadlockList));

			datos_personaje *datos;

			for (j = 0; j < list_size(deadlockList); j++) {

				datos = (datos_personaje*) list_get(
						deadlockList, j);

				string_append(&deadlockMessage,
						string_from_format("%d,", datos->id));

			}

			sendMessage(socketDeadlock, deadlockMessage);
			memset(deadlockMessage, 0, sizeof(deadlockMessage));
			bufferDeadlock = recieveMessage(socketDeadlock);

			char** response = string_split(bufferDeadlock, COMA);

			for (j = 0; j < list_size(deadlockList); j++) {

				datos = (datos_personaje*) list_get(
						deadlockList, j);

				if(datos->id == atoi(response[0])){
					sendMessage(datos->fd, DEATH);
				}

			}

		} else if (list_size(deadlockList) > 1
				&& atoi(deadlockStruct->recovery) == 0) {
			printf("DEADLOCK\n");
			printf("DEADLOCK\n");
			printf("DEADLOCK\n");
			printf("DEADLOCK\n");
			printf("DEADLOCK\n");
			//TODO logear que hubo interbloqueo, pero como recovery == 0 no se hace nada
		} else {
			//TODO loguear que no hubo interbloqueo
		}

	}

	free(ipPuerto);
	free(bufferDeadlock);
	free(deadlockMessage);
	close(socketDeadlock);
}

void saveList(pthread_t *t, resource_struct *resourceStruct, t_list *threads,
		int id) {

	datos_personaje *datos = (datos_personaje*) malloc(sizeof(datos_personaje));

	datos->F = &(resourceStruct->recursosAt->F);
	datos->H = &(resourceStruct->recursosAt->H);
	datos->M = &(resourceStruct->recursosAt->M);
	datos->thread = &t;
	datos->recurso = &(resourceStruct->recursoBloqueado);
	datos->id = &id;
	datos->fd = &resourceStruct->fd;

	list_add(threads, datos);

}
