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
#define MAXQUEUE 100
#define MOVIMIENTO "MOVIMIENTO"
#define CONNECTED "CONNECTED"
#define PIPE "|"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <uncommons/select.h>

void openListener(char* argv, queue_n_locks *queue);
void agregarRecursos(char* buffer, ITEM_NIVEL *listaItems);
void saveList(resource_struct *resourceStruct, t_list *threads, int id);
void deteccionInterbloqueo(deadlock_struct *deadlockStruct);
void analize_response(int *fd, t_list *threads, t_level_config *nivel, int *id,
		ITEM_NIVEL *listaItems, t_dictionary *listaPersonajes, fd_set *socks, int rows, int cols);
resource_struct *getLevelStructure(t_level_config *level_config, int *fd);

ITEM_NIVEL* cambiarEstructura(t_level_config* levelConfig);

int main(int argc, char **argv) {

	t_dictionary *niveles = getLevels();

	char*mensaje = "nivel1";

	t_level_config *nivel = (t_level_config*) malloc(sizeof(t_level_config));

	nivel = dictionary_get(niveles, mensaje);

	ITEM_NIVEL *listaItems = cambiarEstructura(nivel);

	t_dictionary *listaPersonajes = dictionary_create();

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

	int sock; /* fd del listener*/
	int connectlist[MAXQUEUE]; /* array de sockets conectados */
	fd_set socks; /* lista de fds */
	int highsock;
	int listnum;
	int readsocks; /* Number of sockets ready for reading */

	/* Obtain a file descriptor for our "listening" socket */
	sock = socketServer("9931");

	listen(sock, MAXQUEUE);

	highsock = sock;
	memset((char *) &connectlist, 0, sizeof(connectlist));

	int *rows = (int*) malloc(sizeof(int));
	int *cols = (int*) malloc(sizeof(int));
	rows = (int*) 10;
	cols = (int*) 10;

//	nivel_gui_inicializar();
//
//	nivel_gui_get_area_nivel(&rows, &cols);

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	while (1) {
		build_select_list(sock, connectlist, highsock, &socks);

		readsocks = select(FD_SETSIZE, &socks, (fd_set *) 0, (fd_set *) 0,
				&tv );

		if (readsocks < 0) {
			perror("select");
			exit(1);
		}
		if (FD_ISSET(sock, &socks)){
			int fd = handle_new_connection(sock, &connectlist, &highsock, &socks);
			FD_SET(fd, &socks);
//			sendMessage(fd, CONNECTED);
		}
		if(readsocks > -1){
		for (listnum = 0; listnum < MAXQUEUE; listnum++) {
			if (FD_ISSET(connectlist[listnum], &socks))
				analize_response(connectlist[listnum], threads, nivel, &id, listaItems, listaPersonajes, &socks, rows, cols);
			}
		}
	}
}


void analize_response(int *fd, t_list *threads, t_level_config *nivel, int *id,
		ITEM_NIVEL *listaItems, t_dictionary *listaPersonajes, fd_set *socks, int rows, int cols) {

	char* bufferSocket = (char*) malloc(MAXSIZE);
	memset(bufferSocket, 0, sizeof(bufferSocket));
	bufferSocket = recieveMessage(fd);
	if(string_starts_with(bufferSocket, "BROKEN")){
		return;
	}

	if (string_starts_with(bufferSocket, START)) {
		bufferSocket = string_substring_from(bufferSocket, sizeof(START));
		char** split = string_split(bufferSocket, PIPE);
		recursos_otorgados * recursosAt = (recursos_otorgados*) malloc(
				sizeof(recursos_otorgados));
		recursosAt->F = 0;
		recursosAt->H = 0;
		recursosAt->M = 0;

		resource_struct *resourceStruct = getLevelStructure(nivel, fd);

		resourceStruct->recursosAt = recursosAt;

		resourceStruct->recursoBloqueado = "0";

		resourceStruct->listaItems = listaItems;

		resourceStruct->simbolo = split[0][0];

		saveList(resourceStruct, threads, id);

		dictionary_put(listaPersonajes, string_from_format("%d",fd), resourceStruct);

		sendMessage(fd, "ok");

		id++;
	} else if (string_starts_with(bufferSocket, RESOURCES)) {
		agregarRecursos(bufferSocket, listaItems);
	} else if (string_starts_with(bufferSocket, MOVIMIENTO)){
		bufferSocket = string_substring_from(bufferSocket, sizeof(MOVIMIENTO));
		resource_struct *personaje = (resource_struct*) dictionary_get(listaPersonajes, string_from_format("%d",fd));
		movimientoPersonaje(personaje, rows, cols, bufferSocket);
	}
	free(bufferSocket);
}

//TODO PENDIENTE BORRAR ITEMS

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

resource_struct *getLevelStructure(t_level_config *level_config, int *fd) {

	resource_struct *resources = (resource_struct*) malloc(
			sizeof(resource_struct));

	resources->level_config = level_config;
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

	int j = 0;

	char *deadlockMessage = (char*) malloc(MAXSIZE);

	t_list *deadlockList;
	while (1) {

		sleep(3); //levantar de archivo de configuracion, inotify

		deadlockList = detectionAlgorithm(deadlockStruct->items,
				deadlockStruct->list);

		if (list_size(deadlockList) > 1
				&& atoi(deadlockStruct->recovery) == 1) {

			deadlockMessage = string_from_format("DEADLOCK,%d,",
					list_size(deadlockList));

			datos_personaje *datos;

			for (j = 0; j < list_size(deadlockList); j++) {

				datos = (datos_personaje*) list_get(deadlockList, j);

				string_append(&deadlockMessage,
						string_from_format("%d,", datos->id));

			}
			int *socketDeadlock = openSocketClient(ipPuerto[1], ipPuerto[0]);
			sendMessage(socketDeadlock, deadlockMessage);
			memset(deadlockMessage, 0, sizeof(deadlockMessage));
			bufferDeadlock = recieveMessage(socketDeadlock);
			close(socketDeadlock);

			char** response = string_split(bufferDeadlock, COMA);

			for (j = 0; j < list_size(deadlockList); j++) {

				datos = (datos_personaje*) list_get(deadlockList, j);

				if (datos->id == atoi(response[0])) {
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
}

void saveList(resource_struct *resourceStruct, t_list *threads, int id) {

	datos_personaje *datos = (datos_personaje*) malloc(sizeof(datos_personaje));

	datos->F = &(resourceStruct->recursosAt->F);
	datos->H = &(resourceStruct->recursosAt->H);
	datos->M = &(resourceStruct->recursosAt->M);
	datos->recurso = &(resourceStruct->recursoBloqueado);
	datos->id = id;
	datos->fd = &resourceStruct->fd;

	list_add(threads, datos);

}
