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

void analize_response(char *response, t_scheduler_queue *scheduler_queue,
		personaje_planificador *personaje, int *breakIt);
void showLog(t_scheduler_queue *scheduler_queue,
		personaje_planificador *personaje);

t_log *log;

void planificar(t_scheduler_queue *scheduler_queue) {

	if (queue_size(scheduler_queue->character_queue) > 0) {

		char *response = (char *) malloc(MAXDATASIZE);
		int turno = 0;

		int *breakIt;

		breakIt = FALSE;

		int turnoActual = scheduler_queue->orquestador_config->turnos;

		int sleepTime = scheduler_queue->orquestador_config->intervalo;

		turno = 0;

		breakIt = FALSE;

		personaje_planificador *personaje = (personaje_planificador*) queue_pop(
				scheduler_queue->character_queue);

		showLog(scheduler_queue, personaje);


		while (turno < turnoActual && breakIt == FALSE) {

			printf("Realizo el pop\n");

			response = sendMessage(personaje->fd, "Tu turno");

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

				for (j = 0; j < list_size(scheduler_queue->pjList); j++) {
					if (string_equals_ignore_case(personaje->nombre,
							((personaje_planificador*) list_get(
									scheduler_queue->pjList, j))->nombre)) {
						list_remove(scheduler_queue->pjList, j);
					}
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

	log = scheduler_queue->log;
	int j, rc = 1;
	int max_sd, new_sd;
	int desc_ready, end_server = FALSE;
	struct timeval timeout;
	fd_set master_set;
	fd_set working_set;

	//Declaro las estructuras nesesarias para el inotify

	scheduler_queue->master_set = &master_set;

	rc = listen(scheduler_queue->listen_sd, 32);
	if (rc < 0) {
		perror("listen() failed");
		close(scheduler_queue->listen_sd);
		exit(-1);
	}

	FD_ZERO(&master_set);
	max_sd = scheduler_queue->listen_sd;
	FD_SET(scheduler_queue->listen_sd, &master_set);

	do {

		memcpy(&working_set, &master_set, sizeof(master_set));


		timeout.tv_sec = 0;
		timeout.tv_usec = 0;

		rc = select(FD_SETSIZE, &working_set, NULL, NULL, &timeout);



		if (rc < 0) {
			perror("  select() failed");
			break;
		}

		if (rc == 0 && queue_size(scheduler_queue->character_queue) > 0) {
			planificar(scheduler_queue);
		} else if (rc > 0) {
			desc_ready = rc;
			for (j = 0; j <= max_sd && desc_ready > 0; ++j) {

				if (FD_ISSET(j, &working_set)) {

					desc_ready -= 1;


					if (j == scheduler_queue->listen_sd) {
						printf("  Listening socket is readable\n");

						do {

							new_sd = accept(scheduler_queue->listen_sd, NULL,
									NULL );
							if (new_sd < 0) {
								if (errno != EWOULDBLOCK) {
									perror("  accept() failed");
									end_server = TRUE;
								}
								break;
							}


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

						} while (new_sd != -1);
					}


				}
			}
		}

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

void showLog(t_scheduler_queue *scheduler_queue,
		personaje_planificador *personaje) {

	char* myLog = (char*) malloc(MAXSIZE * 3);

	string_append(&myLog, "Listos:");

	int i = 0;

	for (i = 0; i < queue_size(scheduler_queue->character_queue); i++) {
		personaje_planificador *personaje_temporal = queue_pop(
				scheduler_queue->character_queue);
		string_append(&myLog,
				string_from_format("<-%s", personaje_temporal->nombre));
		queue_push(scheduler_queue->character_queue, personaje_temporal);
	}

	string_append(&myLog, ";Bloqueados:");
	if (queue_size(scheduler_queue->blocked_queue) > 0) {
		for (i = 0; i < queue_size(scheduler_queue->blocked_queue); i++) {
			blocked_character *personaje_temporal = queue_pop(
					scheduler_queue->blocked_queue);
			string_append(&myLog,
					string_from_format("<-%s",
							personaje_temporal->personaje->nombre));
			queue_push(scheduler_queue->blocked_queue, personaje_temporal);
		}
	}

	string_append(&myLog,
			string_from_format(";Ejecutando: %s", personaje->nombre));

	log_info(log, myLog);
}
