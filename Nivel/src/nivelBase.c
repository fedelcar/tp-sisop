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
#define TRUE             1
#define FALSE            0
#define SIMBOLO "Simbolo"
#define DEFAULTPORT 9930

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <uncommons/select.h>

#include <sys/ioctl.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>

//void openListener(char* argv, queue_n_locks *queue);
void agregarRecursos(char* buffer, ITEM_NIVEL *listaItems);
void saveList(resource_struct *resourceStruct, t_list *threads);
void deteccionInterbloqueo(deadlock_struct *deadlockStruct);
void analize_response(int fd, t_list *threads, t_level_config *nivel,
		ITEM_NIVEL *listaItems, t_dictionary *listaPersonajes, int rows,
		int cols, fd_set *master_set, int socketOrquestador);
resource_struct *getLevelStructure(t_level_config *level_config, int *fd);

ITEM_NIVEL* cambiarEstructura(t_level_config* levelConfig);

int id;

int main(int argc, char **argv) {

	id = 0;

	t_level_config *nivel = (t_level_config*) malloc(sizeof(t_level_config));

	nivel = getLevel("/home/tp/config/niveles/nivel1.config"); //argv[0]

	ITEM_NIVEL *listaItems = cambiarEstructura(nivel);

	t_dictionary *listaPersonajes = dictionary_create();

	char **ipPuerto = string_split(nivel->orquestador, DOSPUNTOS);

	int socketOrquestador = openSocketClient(ipPuerto[1], ipPuerto[0]);

	t_list *threads = list_create();

	int *id = (int*) malloc(sizeof(int));

	id = 0;

	pthread_t detectionThread;

	deadlock_struct *deadlockStruct = (deadlock_struct*) malloc(
			sizeof(deadlock_struct));

	deadlockStruct->items = listaItems;
	deadlockStruct->list = threads;
	deadlockStruct->socket = socketOrquestador;
	deadlockStruct->recovery = nivel->recovery;
	deadlockStruct->checkDeadlock = nivel->tiempoChequeoDeadlock;
	deadlockStruct->path = "/home/tp/config/niveles/nivel1.config"; //argv[0]

	pthread_create(&detectionThread, NULL, (void*) deteccionInterbloqueo,
			(deadlock_struct*) deadlockStruct);

	int *rows = (int*) malloc(sizeof(int));
	int *cols = (int*) malloc(sizeof(int));
	rows = (int*) 10;
	cols = (int*) 10;

	nivel_gui_inicializar();

	nivel_gui_get_area_nivel(&rows, &cols);

	int j, len, rc, on = 1;
	int listen_sd, max_sd, new_sd;
	int desc_ready, end_server = FALSE;
	int close_conn;
	char buffer[MAXSIZE];
	struct sockaddr_in addr;
	struct timeval timeout;
	fd_set master_set;
	fd_set working_set;

	/*************************************************************/
	/* Create an AF_INET stream socket to receive incoming       */
	/* connections on                                            */
	/*************************************************************/
	listen_sd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sd < 0) {
		perror("socket() failed");
		exit(-1);
	}

	/*************************************************************/
	/* Allow socket descriptor to be reuseable                   */
	/*************************************************************/
	rc = setsockopt(listen_sd, SOL_SOCKET, SO_REUSEADDR, (char *) &on,
			sizeof(on));
	if (rc < 0) {
		perror("setsockopt() failed");
		close(listen_sd);
		exit(-1);
	}

	/*************************************************************/
	/* Set socket to be non-blocking.  All of the sockets for    */
	/* the incoming connections will also be non-blocking since  */
	/* they will inherit that state from the listening socket.   */
	/*************************************************************/
	rc = ioctl(listen_sd, FIONBIO, (char *) &on);
	if (rc < 0) {
		perror("ioctl() failed");
		close(listen_sd);
		exit(-1);
	}

	/*************************************************************/
	/* Bind the socket                                           */
	/*************************************************************/

	int portForLevel = DEFAULTPORT;

	while (1) {
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_ANY );
		addr.sin_port = htons(portForLevel);
		rc = bind(listen_sd, (struct sockaddr *) &addr, sizeof(addr));
		if (rc < 0) {
			portForLevel++;
			continue;
		}
		break;
	}
	/*************************************************************/
	/* Set the listen back log                                   */
	/*************************************************************/
	rc = listen(listen_sd, 32);
	if (rc < 0) {
		perror("listen() failed");
		close(listen_sd);
		exit(-1);
	}

	//Resuelvo que puerto tengo.
	socklen_t length;
	struct sockaddr_storage addrr;
	char ipstr[INET_ADDRSTRLEN];

	len = sizeof addr;
	getpeername(listen_sd, (struct sockaddr*) &addrr, &length);

	struct sockaddr_in *s = (struct sockaddr_in *) &addr;

	sendMessage(socketOrquestador,
			string_from_format("NEWLVL,%d,%s,", ntohs(s->sin_port),
					nivel->nombre));

	/*************************************************************/
	/* Initialize the master fd_set                              */
	/*************************************************************/
	FD_ZERO(&master_set);
	max_sd = listen_sd;
	FD_SET(listen_sd, &master_set);

	do {
		/**********************************************************/
		/* Copy the master fd_set over to the working fd_set.     */
		/**********************************************************/
		memcpy(&working_set, &master_set, sizeof(master_set));

//		      printf("Waiting on select()...\n");
		rc = select(FD_SETSIZE, &working_set, NULL, NULL, NULL );

		/**********************************************************/
		/* Check to see if the select call failed.                */
		/**********************************************************/
		if (rc < 0) {
			perror("  select() failed");
			break;
		}

		/**********************************************************/
		/* Check to see if the 5 minute time out expired.         */
		/**********************************************************/
		if (rc == 0) {
			printf("  select() timed out.  End program.\n");
			break;
		}

		/**********************************************************/
		/* One or more descriptors are readable.  Need to         */
		/* determine which ones they are.                         */
		/**********************************************************/
		desc_ready = rc;
		for (j = 0; j <= max_sd && desc_ready > 0; ++j) {
			/*******************************************************/
			/* Check to see if this descriptor is ready            */
			/*******************************************************/
			if (FD_ISSET(j, &working_set)) {
				/****************************************************/
				/* A descriptor was found that was readable - one   */
				/* less has to be looked for.  This is being done   */
				/* so that we can stop looking at the working set   */
				/* once we have found all of the descriptors that   */
				/* were ready.                                      */
				/****************************************************/
				desc_ready -= 1;

				/****************************************************/
				/* Check to see if this is the listening socket     */
				/****************************************************/
				if (j == listen_sd) {
					//printf("  Listening socket is readable\n");
					/*************************************************/
					/* Accept all incoming connections that are      */
					/* queued up on the listening socket before we   */
					/* loop back and call select again.              */
					/*************************************************/
					do {
						/**********************************************/
						/* Accept each incoming connection.  If       */
						/* accept fails with EWOULDBLOCK, then we     */
						/* have accepted all of them.  Any other      */
						/* failure on accept will cause us to end the */
						/* server.                                    */
						/**********************************************/
						new_sd = accept(listen_sd, NULL, NULL );
						if (new_sd < 0) {
							if (errno != EWOULDBLOCK) {
								perror("  accept() failed");
								end_server = TRUE;
							}
							break;
						}

						/**********************************************/
						/* Add the new incoming connection to the     */
						/* master read set                            */
						/**********************************************/
						//printf("  New incoming connection - %d\n", new_sd);
						FD_SET(new_sd, &master_set);
						if (new_sd > max_sd)
							max_sd = new_sd;

						/**********************************************/
						/* Loop back up and accept another incoming   */
						/* connection                                 */
						/**********************************************/
					} while (new_sd != -1);
				}

				/****************************************************/
				/* This is not the listening socket, therefore an   */
				/* existing connection must be readable             */
				/****************************************************/
				else {
					// printf("  Descriptor %d is readable\n", j);
					close_conn = FALSE;
					/*************************************************/
					/* Receive all incoming data on this socket      */
					/* before we loop back and call select again.    */
					/*************************************************/

					/**********************************************/
					/* Receive data on this connection until the  */
					/* recv fails with EWOULDBLOCK.  If any other */
					/* failure occurs, we will close the          */
					/* connection.                                */
					/**********************************************/
					//	                  rc = recv(j, buffer, sizeof(buffer), 0);
					//	                  if (rc < 0)
					//	                  {
					//	                     if (errno != EWOULDBLOCK)
					//	                     {
					//	                        perror("  recv() failed");
					//	                        close_conn = TRUE;
					//	                     }
					//	                     break;
					//	                  }
					/**********************************************/
					/* Check to see if the connection has been    */
					/* closed by the client                       */
					/**********************************************/
					//	                  if (rc == 0)
					//	                  {
					//	                     printf("  Connection closed\n");
					//	                     close_conn = TRUE;
					//	                     break;
					//	                  }
					/**********************************************/
					/* Data was recevied                          */
					/**********************************************/
					//	                  len = rc;
					//	                  printf("  %d bytes received\n", len);
					/**********************************************/
					/* Echo the data back to the client           */
					/**********************************************/
					analize_response(j, threads, nivel, listaItems,
							listaPersonajes, rows, cols, &master_set,
							socketOrquestador);
					//	                  rc = send(i, buffer, len, 0);
					//	                  if (rc < 0)
					//	                  {
					//	                     perror("  send() failed");
					//	                     close_conn = TRUE;
					//	                     break;
					//	                  }

					/*************************************************/
					/* If the close_conn flag was turned on, we need */
					/* to clean up this active connection.  This     */
					/* clean up process includes removing the        */
					/* descriptor from the master set and            */
					/* determining the new maximum descriptor value  */
					/* based on the bits that are still turned on in */
					/* the master set.                               */
					/*************************************************/
					if (close_conn) {
						close(j);
						FD_CLR(j, &master_set);
						if (j == max_sd) {
							while (FD_ISSET(max_sd, &master_set) == FALSE)
								max_sd -= 1;
						}
					}
				} /* End of existing connection is readable */
			} /* End of if (FD_ISSET(i, &working_set)) */
		} /* End of loop through selectable descriptors */

	} while (end_server == FALSE);

}

void analize_response(int fd, t_list *threads, t_level_config *nivel,
		ITEM_NIVEL *listaItems, t_dictionary *listaPersonajes, int rows,
		int cols, fd_set *master_set, int socketOrquestador) {

	char* bufferSocket = (char*) malloc(MAXSIZE);
	memset(bufferSocket, 0, sizeof(bufferSocket));
	bufferSocket = recieveMessage(fd);
	if (string_starts_with(bufferSocket, "BROKEN")) {
		if (dictionary_has_key(listaPersonajes, string_from_format("%d", fd))) {
			resource_struct *personajeABorrar = dictionary_get(listaPersonajes,
					string_from_format("%d", fd));
			agregarRecursos(
					string_from_format("RSC,%d,%d,%d",
							personajeABorrar->recursosAt->F,
							personajeABorrar->recursosAt->M,
							personajeABorrar->recursosAt->H), listaItems);
			nivel_gui_dibujar(listaItems);
			dictionary_remove(listaPersonajes, string_from_format("%d", fd));
			free(personajeABorrar);
		}
		FD_CLR(fd, master_set);
		return;
	}

	if (string_starts_with(bufferSocket, START)) {
		bufferSocket = string_substring_from(bufferSocket, sizeof(START));
		bufferSocket = string_substring_from(bufferSocket, sizeof(SIMBOLO));
		char** split = string_split(bufferSocket, PIPE);
		recursos_otorgados * recursosAt = (recursos_otorgados*) malloc(
				sizeof(recursos_otorgados));
		recursosAt->F = 0;
		recursosAt->H = 0;
		recursosAt->M = 0;

		resource_struct *resourceStruct = getLevelStructure(nivel, fd);

		resourceStruct->recursosAt = recursosAt;

		resourceStruct->recursoBloqueado = "0";

		resourceStruct->simbolo = split[0][0];

		resourceStruct->posicion = (t_posicion*) malloc(sizeof(t_posicion));

		resourceStruct->posicion->posX = 1;
		resourceStruct->posicion->posY = 1;

		CrearPersonaje(&listaItems, resourceStruct->simbolo, 1, 1);

		resourceStruct->listaItems = listaItems;

		saveList(resourceStruct, threads);

		dictionary_put(listaPersonajes, string_from_format("%d", fd),
				resourceStruct);

		sendMessage(fd, "ok");

		id++;
	} else if (string_starts_with(bufferSocket, RESOURCES)) {
		agregarRecursos(bufferSocket, listaItems);
	} else if (string_starts_with(bufferSocket, MOVIMIENTO)) {
		bufferSocket = string_substring_from(bufferSocket, sizeof(MOVIMIENTO));
		resource_struct *personaje = (resource_struct*) dictionary_get(
				listaPersonajes, string_from_format("%d", fd));
		movimientoPersonaje(personaje, rows, cols, bufferSocket, master_set, fd,
				socketOrquestador);
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
	listaItems = NULL;

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

	char *bufferDeadlock = (char*) malloc(MAXSIZE);

	int j = 0;

	char *deadlockMessage = (char*) malloc(MAXSIZE);

	t_list *deadlockList;
	while (1) {

		sleep(deadlockStruct->checkDeadlock); //levantar de archivo de configuracion, inotify

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
			sendMessage(deadlockStruct->socket, deadlockMessage);
			memset(deadlockMessage, 0, sizeof(deadlockMessage));
			bufferDeadlock = recieveMessage(deadlockStruct->socket);

			char** response = string_split(bufferDeadlock, COMA);

			for (j = 0; j < list_size(deadlockList); j++) {

				datos = (datos_personaje*) list_get(deadlockList, j);

				if (datos->id == atoi(response[0])) {
					printf("Muerte al personaje, %d", datos->fd);
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

	free(bufferDeadlock);
	free(deadlockMessage);
}

void saveList(resource_struct *resourceStruct, t_list *threads) {

	datos_personaje *datos = (datos_personaje*) malloc(sizeof(datos_personaje));

	datos->F = &(resourceStruct->recursosAt->F);
	datos->H = &(resourceStruct->recursosAt->H);
	datos->M = &(resourceStruct->recursosAt->M);
	datos->recurso = &(resourceStruct->recursoBloqueado);
	datos->id = id;
	datos->fd = &resourceStruct->fd;

	list_add(threads, datos);

}
