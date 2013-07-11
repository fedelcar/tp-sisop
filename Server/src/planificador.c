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
#include "uncommons/inotify.h"
#include "planificador.h"
#include "commons/string.h"
#include "uncommons/select.h"
#include <string.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#define TRUE             1
#define FALSE            0

#include <sys/ioctl.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <netinet/in.h>

//TODO CHANGE SIGNALS VALUES AND RESPONSE LENGTH
#define SIGNAL_OK "ok"
#define SIGNAL_BLOCKED "BLOCKED"
#define MAXDATASIZE 1024
#define MAXSIZE 1024
#define TERMINE_NIVEL "Termine nivel"
#define PEDIR "PEDIRRECURSO"
#define TRUE 1
#define FALSE 0
#define BROKEN "BROKEN"
#define SETNAME "SETNAME"
#define TIEMPOCHEQUEODEADLOCK "TiempoChequeoDeadlock"
#define TURNOS "turnos"
#define INTERVALO "intervalo"

void analize_response(char *response, t_scheduler_queue *scheduler_queue,
		personaje_planificador *personaje, int *breakIt);

void planificar(t_scheduler_queue *scheduler_queue) {

	if (queue_size(scheduler_queue->character_queue) > 0) {

		char *response = (char *) malloc(MAXDATASIZE); //CHECK LENGTH

		int turno = 0;

		int *breakIt;

		breakIt = FALSE;

		int turnoActual = scheduler_queue->orquestador_config->turnos;

		int sleepTime = scheduler_queue->orquestador_config->intervalo;

		turno = 0;

		breakIt = FALSE;

		printf("Entro al while\n");

		personaje_planificador *personaje = (personaje_planificador*) queue_pop(
				scheduler_queue->character_queue);

		while (turno < turnoActual && breakIt == FALSE) {

			printf("Realizo el pop\n");

			response = sendMessage(personaje->fd, "Paso por el planificador\n");

			if (string_starts_with(response, BROKEN)) {

				int j = 0;

				for (j = 0; j < list_size(scheduler_queue->pjList); j++) {
					if (string_equals_ignore_case(personaje->nombre,
							((personaje_planificador*) list_get(
									scheduler_queue->pjList, j))->nombre)) {
						list_remove(scheduler_queue->pjList, j);
					}
				}

				break;
			}

			printf("Mando mensaje\n");

			response = recieveMessage(personaje->fd);

			printf(response);

			if (string_starts_with(response, BROKEN)) {
				int j = 0;
				if (string_equals_ignore_case(personaje->nombre,
						((personaje_planificador*) list_get(
								scheduler_queue->pjList, j))->nombre)) {
					list_remove(scheduler_queue->pjList, j);
				}
				break;
			}

			printf("Respondio mensaje\n");

			analize_response(response, scheduler_queue, personaje, &breakIt);

			sleep(sleepTime);
			free(response);
			turno++;
		}
		if (string_starts_with(response, SIGNAL_OK) && breakIt == 0) {
			queue_push(scheduler_queue->character_queue, personaje);
		}
	}
}

void planificador(t_scheduler_queue *scheduler_queue) {

	int j, rc = 1;
	int max_sd, new_sd;
	int desc_ready, end_server = FALSE;
	struct timeval timeout;
	fd_set master_set;
	fd_set working_set;

	//Declaro las estructuras nesesarias para el inotify

	inotify_struct *datos = (inotify_struct*) malloc(sizeof(inotify_struct));
	inotify_list_struct* data = (inotify_list_struct*) malloc(
			sizeof(inotify_list_struct));
	inotify_list_struct* data2 = (inotify_list_struct*) malloc(
			sizeof(inotify_list_struct));
	datos->path = scheduler_queue->path;
	datos->lista = list_create();

	data->nombre = TURNOS;
	data->valor = &(scheduler_queue->orquestador_config->turnos);
	list_add(datos->lista, data);

	data2->nombre = INTERVALO;
	data2->valor = &(scheduler_queue->orquestador_config->intervalo);
	list_add(datos->lista, data2);

	pthread_t *thread_inot = (pthread_t*) malloc(sizeof(pthread_t));
	pthread_create(thread_inot, NULL, (void *) inotify,
			(inotify_struct *) datos);

	scheduler_queue->master_set = &master_set;
	/*************************************************************/
	/* Set the listen back log                                   */
	/*************************************************************/
	rc = listen(scheduler_queue->listen_sd, 32);
	if (rc < 0) {
		perror("listen() failed");
		close(scheduler_queue->listen_sd);
		exit(-1);
	}

	/*************************************************************/
	/* Initialize the master fd_set                              */
	/*************************************************************/
	FD_ZERO(&master_set);
	max_sd = scheduler_queue->listen_sd;
	FD_SET(scheduler_queue->listen_sd, &master_set);

	do {
		/**********************************************************/
		/* Copy the master fd_set over to the working fd_set.     */
		/**********************************************************/
		memcpy(&working_set, &master_set, sizeof(master_set));

		/**********************************************************/
		/* Call select() and wait 5 minutes for it to complete.   */
		/**********************************************************/
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;

		rc = select(FD_SETSIZE, &working_set, NULL, NULL, &timeout);

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
		if (rc == 0 && queue_size(scheduler_queue->character_queue) > 0) {
			planificar(scheduler_queue);
		} else if (rc > 0) {
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
					if (j == scheduler_queue->listen_sd) {
						printf("  Listening socket is readable\n");
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
							new_sd = accept(scheduler_queue->listen_sd, NULL,
									NULL );
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
							printf("  New incoming connection - %d\n", new_sd);
							personaje_planificador *personaje =
									(personaje_planificador*) malloc(
											sizeof(personaje_planificador));
							personaje->fd = new_sd;
							personaje->respondio = 1;
							personaje->nombre = recieveMessage(new_sd);
							list_add(scheduler_queue->pjList, personaje);
							queue_push(scheduler_queue->character_queue,
									personaje);
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
				}
			} /* End of if (FD_ISSET(i, &working_set)) */
		} /* End of loop through selectable descriptors */

	} while (end_server == FALSE);

}

void analize_response(char *response, t_scheduler_queue *scheduler_queue,
		personaje_planificador *personaje, int *breakIt) {

	if (string_starts_with(response, SIGNAL_BLOCKED)) {
		response = string_substring_from(response, sizeof(SIGNAL_BLOCKED));
		blocked_character *blockedCharacter = (blocked_character*) malloc(
				sizeof(blocked_character));
		blockedCharacter->personaje = personaje;
		blockedCharacter->recurso = response[0];
		*breakIt = TRUE;
		queue_push(scheduler_queue->blocked_queue, blockedCharacter);
	} else if (string_equals_ignore_case(response, TERMINE_NIVEL)) {
		close(personaje->fd);
	} else if (string_starts_with(response, PEDIR)) {
		*breakIt = TRUE;
		queue_push(scheduler_queue->character_queue, personaje);
	}
}
