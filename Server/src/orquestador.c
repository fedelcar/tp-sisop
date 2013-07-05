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

#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <uncommons/select.h>

#include <sys/ioctl.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>

#define DOSPUNTOS ":"
#define MAXSIZE 1024
#define PORT "9930"
#define LEVEL "LVL"
#define PIPE "|"
#define FREERESC "FREERESC"
#define DEADLOCK "DEADLOCK"
#define COMA ","
#define OKEY "ok"
#define DAR_RECURSO "DAR_RECURSO"
#define NOTOK "NOTOK"
#define BROKEN "BROKEN"
#define CONNECTED "CONNECTED"
#define TRUE             1
#define FALSE            0

//void accionar(int sock, int *connectlist, int *highsock, fd_set *socks, t_dictionary *levelsMap, t_dictionary *levels_queues);
void executeResponse(char* response, t_dictionary *levelsMap, int fd,
		t_dictionary *levels_queues, fd_set *socks);
void giveResource(t_scheduler_queue *queues, int recurso,
		blocked_character *blockedCharacter);
//void orquestador(t_dictionary *levelsMap, int fd, t_dictionary *levels_queues, fd_set *socks);
void orquestador(t_dictionary *levelsMap, int fd, t_dictionary *levels_queues, fd_set *socks);

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

		t_scheduler_queue *scheduler_queue = (t_scheduler_queue*) malloc(
				sizeof(t_scheduler_queue));

		scheduler_queue->blocked_queue = queue_create();
		scheduler_queue->character_queue = queue_create();
		scheduler_queue->portInt = atoi(port[1]);

		dictionary_put(levels_queues, levelName, scheduler_queue);

		pthread_create(t, NULL, (void *) planificador,
				(t_scheduler_queue*) scheduler_queue);

	}

	free(levelName);
	free(levelsList);

	int j, len, rc, on = 1;
	int listen_sd, max_sd, new_sd;
	int desc_ready, end_server = FALSE;
	int close_conn;
	char buffer[MAXSIZE];
	struct sockaddr_in addr;
	struct timeval timeout;
	fd_set master_set;
	fd_set working_set;

	listen_sd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sd < 0) {
		perror("socket() failed");
		exit(-1);
	}

	rc = setsockopt(listen_sd, SOL_SOCKET, SO_REUSEADDR, (char *) &on,
			sizeof(on));
	if (rc < 0) {
		perror("setsockopt() failed");
		close(listen_sd);
		exit(-1);
	}

	rc = ioctl(listen_sd, FIONBIO, (char *) &on);
	if (rc < 0) {
		perror("ioctl() failed");
		close(listen_sd);
		exit(-1);
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY );
	addr.sin_port = htons(9930);
	rc = bind(listen_sd, (struct sockaddr *) &addr, sizeof(addr));
	if (rc < 0) {
		perror("bind() failed");
		close(listen_sd);
		exit(-1);
	}

	rc = listen(listen_sd, 32);
	if (rc < 0) {
		perror("listen() failed");
		close(listen_sd);
		exit(-1);
	}

	FD_ZERO(&master_set);
	max_sd = listen_sd;
	FD_SET(listen_sd, &master_set);

	do {

		memcpy(&working_set, &master_set, sizeof(master_set));

		printf("Waiting on select()...\n");
		rc = select(FD_SETSIZE, &working_set, NULL, NULL, NULL );

		if (rc < 0) {
			perror("  select() failed");
			break;
		}

		if (rc == 0) {
			printf("  select() timed out.  End program.\n");
			break;
		}

		desc_ready = rc;
		for (j = 0; j <= max_sd && desc_ready > 0; ++j) {

			if (FD_ISSET(j, &working_set)) {

				desc_ready -= 1;

				if (j == listen_sd) {
					printf("  Listening socket is readable\n");

					do {
						new_sd = accept(listen_sd, NULL, NULL );
						if (new_sd < 0) {
							if (errno != EWOULDBLOCK) {
								perror("  accept() failed");
								end_server = TRUE;
							}
							break;
						}

						printf("  New incoming connection - %d\n", new_sd);
						FD_SET(new_sd, &master_set);
						if (new_sd > max_sd)
							max_sd = new_sd;

					} while (new_sd != -1);
				}

				else {
					printf("  Descriptor %d is readable\n", j);
					close_conn = FALSE;

					orquestador(levelsMap, j, levels_queues, &master_set);

					if (close_conn) {
						close(j);
						FD_CLR(j, &master_set);
						if (j == max_sd) {
							while (FD_ISSET(max_sd, &master_set) == FALSE)
								max_sd -= 1;
						}
					}
				}
			}
		}

	} while (end_server == FALSE);

}

void orquestador(t_dictionary *levelsMap, int fd, t_dictionary *levels_queues, fd_set *socks) {

	char *response = (char *) malloc(MAXSIZE);

	response = recieveMessage(fd);

	if (!string_starts_with(response, BROKEN)) {
		executeResponse(response, levelsMap, fd, levels_queues, socks);
	}

}

void executeResponse(char* response, t_dictionary *levelsMap, int fd,
		t_dictionary *levels_queues, fd_set *socks) {

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
		FD_CLR(fd, socks);
		free(socketsToGo);
		free(response);
		free(level);
		close(fd);
	} else if (string_starts_with(response, FREERESC)) {

		response = string_substring_from(response, sizeof(FREERESC));

		char** data = (char*) malloc(MAXSIZE);

		data = string_split(response, COMA);

		t_scheduler_queue *queues = dictionary_get(levels_queues, data[0]);

		if (queue_size(queues->blocked_queue) == 0) {
			sendMessage(fd, OKEY);
			close(fd);
		} else {

			sendMessage(fd, NOTOK);

			int hongos = (int*) malloc(sizeof(int));
			int monedas = (int*) malloc(sizeof(int));
			int flores = (int*) malloc(sizeof(int));

			flores = atoi(data[1]);
			monedas = atoi(data[2]);
			hongos = atoi(data[3]);

			int i = 0;

			for (i = 0; i < queue_size(queues->blocked_queue); i++) {

				blocked_character *blockedCharacter = queue_pop(
						queues->blocked_queue);

				if (blockedCharacter->recurso == 'F') {
					giveResource(queues, flores, blockedCharacter);
				} else if (blockedCharacter->recurso == 'H') {
					giveResource(queues, hongos, blockedCharacter);
				} else if (blockedCharacter->recurso == 'M') {
					giveResource(queues, monedas, blockedCharacter);
				}

				t_level_address *addresses = (t_level_address*) dictionary_get(
						levelsMap, data[0]);

				char **levelSocket = string_split(addresses->nivel, DOSPUNTOS);

				int fdNivel = openSocketClient(levelSocket[1], levelSocket[0]);

				sendMessage(fdNivel,
						string_from_format("RSC,%d,%d,%d", flores, monedas,
								hongos));

				free(data);
			}

		}
	} else if (string_starts_with(response, DEADLOCK)) {

		response = string_substring_from(response, sizeof(DEADLOCK));
		char** deadlockPointer = string_split(response, COMA);

		int i = 0;
		int size = atoi(deadlockPointer[0]) + 1;
		int selected = atoi(deadlockPointer[1]);

		for (i = 2; i < size; i++) {
			if (atoi(deadlockPointer[i]) < selected) {
				selected = atoi(deadlockPointer[i]);
			}
		}

		sendMessage(fd, string_from_format("%d,", selected));

	}

	FD_CLR(fd, socks);

}

void giveResource(t_scheduler_queue *queues, int recurso,
		blocked_character *blockedCharacter) {
	if (recurso > 0) {
		recurso--;
		queue_push(queues->character_queue, blockedCharacter->fd);
	}
}
