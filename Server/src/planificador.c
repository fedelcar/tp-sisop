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

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>

//TODO CHANGE SIGNALS VALUES AND RESPONSE LENGTH
#define SIGNAL_OK "OK"
#define SIGNAL_BLOCKED "BLOCKED"
#define MAXDATASIZE 1024
#define TERMINE_NIVEL "Termine nivel"
#define PEDIR "PEDIRRECURSO"
#define TRUE 1
#define FALSE 0
#define BROKEN "BROKEN"

void analize_response(char *response, t_scheduler_queue *scheduler_queue, int fd, int *breakIt);
void nuevo_fd(int sock, int connectlist[], int highsock, fd_set socks,
		t_scheduler_queue *scheduler_queue);

void planificar(t_scheduler_queue *scheduler_queue) {

	if(queue_size(scheduler_queue->character_queue )> 0){

	char *response = (char *) malloc(MAXDATASIZE); //CHECK LENGTH

	int *fd;

	t_level_attributes *level;

	int turno = 0;

	int *breakIt;

	breakIt = FALSE;

	level = getLevelAttributes();

	int turnoActual = atoi(level->turnos);

	int sleepTime = atoi(level->sleep);

	turno = 0;

	breakIt = FALSE;

	printf("Entro al while\n");

	fd = (int *) queue_pop(scheduler_queue->character_queue);

	while (turno < turnoActual && breakIt == FALSE) {

		printf("Realizo el pop\n");

		sendMessage(fd, "Paso por el planificador\n");

		printf("Mando mensaje\n");

		response = recieveMessage(fd);

		if (string_starts_with(response, BROKEN)) {
			break;
		}

		printf("Respondio mensaje\n");

		analize_response(response, scheduler_queue, fd, &breakIt);

		sleep(sleepTime);

		turno++;
	}
	if (string_equals_ignore_case(response, SIGNAL_OK) && breakIt == 0) {
		queue_push(scheduler_queue->character_queue, fd);
	}
	}
}

void planificador(t_scheduler_queue *scheduler_queue) {

	int sock; /* fd del listener*/
	int connectlist[MAXQUEUE]; /* array de sockets conectados */
	fd_set socks; /* lista de fds */
	int highsock; /* Highest #'d file descriptor, needed for select() */

	int readsocks; /* Number of sockets ready for reading */

	/* Obtain a file descriptor for our "listening" socket */
	sock = socketServer(scheduler_queue->port);

	/* Set up queue for incoming connections. */
	listen(sock, MAXQUEUE);
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	highsock = sock;
	memset((char *) &connectlist, 0, sizeof(connectlist));

	while (1) { /* Main server loop - forever */
		build_select_list(sock, connectlist, highsock, &socks);

		readsocks = select(FD_SETSIZE, &socks, (fd_set *) 0, (fd_set *) 0, &tv);

		if (readsocks < 0) {
			perror("select");
			exit(1);
		}
		if (readsocks == 0) {
			planificar(scheduler_queue);
		} else
			nuevo_fd(sock, connectlist, highsock, socks, scheduler_queue);
	}
}

void nuevo_fd(int sock, int connectlist[], int highsock, fd_set socks,
		t_scheduler_queue *scheduler_queue) {
	int listnum;

// Devuelvo el valor correspondiente al fd listener para primero gestionar conexiones nuevas.
	if (FD_ISSET(sock,&socks)) {
		queue_push(scheduler_queue->character_queue, handle_new_connection_scheduler(sock, connectlist, highsock, socks));
	}

}

void analize_response(char *response, t_scheduler_queue *scheduler_queue, int fd, int *breakIt) {

	if (string_starts_with(response, SIGNAL_BLOCKED)) {
		response = string_substring_from(response, sizeof(SIGNAL_BLOCKED));
		blocked_character *blockedCharacter = (blocked_character*) malloc(
				sizeof(blocked_character));
		blockedCharacter->fd = fd;
		blockedCharacter->recurso = response[0];
		*breakIt = TRUE;
		queue_push(scheduler_queue->blocked_queue, blockedCharacter);
	} else if (string_equals_ignore_case(response, TERMINE_NIVEL)) {
		close(fd);
	} else if (string_equals_ignore_case(response, PEDIR)) {
		*breakIt = TRUE;
		queue_push(scheduler_queue->character_queue, fd);
	}
}
