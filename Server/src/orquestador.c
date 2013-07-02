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

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <uncommons/select.h>

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

void accionar(int sock, int *connectlist, int *highsock, fd_set *socks, t_dictionary *levelsMap, t_dictionary *levels_queues);
void executeResponse(char* response, t_dictionary *levelsMap, int *fd, t_dictionary *levels_queues);
void giveResource(t_scheduler_queue *queues, int recurso, blocked_character *blockedCharacter);
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

		t_scheduler_queue *scheduler_queue = (t_scheduler_queue*) malloc(sizeof(t_scheduler_queue));

		scheduler_queue->blocked_queue = queue_create();
		scheduler_queue->character_queue = queue_create();
		scheduler_queue->port = port[1];

		dictionary_put(levels_queues, levelName, scheduler_queue);

		pthread_create(t, NULL, (void *) planificador, (t_scheduler_queue*) scheduler_queue);

	}

	free(levelName);
	free(addresses);
	free(levelsList);


	int sock; /* fd del listener*/
	int connectlist[MAXQUEUE]; /* array de sockets conectados */
	fd_set socks; /* lista de fds */
	int highsock;

	int readsocks; /* Number of sockets ready for reading */

	/* Obtain a file descriptor for our "listening" socket */
	sock = socketServer(PORT);

	listen(sock, MAXQUEUE);

	highsock = sock;
	memset((char *) &connectlist, 0, sizeof(connectlist));

	while (1) {
		build_select_list(sock, connectlist, highsock, &socks);

		readsocks = select(FD_SETSIZE, &socks, (fd_set *) 0, (fd_set *) 0, NULL);

		if (readsocks < 0) {
			perror("select");
			exit(1);
		} else
			printf("paso por accionar\n");
			accionar(sock, &connectlist, &highsock, &socks, levelsMap, levels_queues);
	}



}


void accionar(int sock, int *connectlist, int *highsock, fd_set *socks, t_dictionary *levelsMap, t_dictionary *levels_queues) {
	int listnum;

// Devuelvo el valor correspondiente al fd listener para primero gestionar conexiones nuevas.
	if (FD_ISSET(sock,socks)){
		int fd = handle_new_connection(sock, connectlist, highsock, socks);
		FD_SET(fd, socks);
		sendMessage(fd, CONNECTED);
	}
	for (listnum = 0; listnum < MAXQUEUE; listnum++) {
		if (FD_ISSET(connectlist[listnum],socks))
			orquestador(levelsMap, connectlist[listnum], levels_queues, socks);
			connectlist[listnum] = 0;
	}
}

void orquestador(t_dictionary *levelsMap, int fd, t_dictionary *levels_queues, fd_set *socks) {

	char *response = (char *) malloc(MAXSIZE);

		response = recieveMessage(fd);

		if(!string_starts_with(response, BROKEN)){
			executeResponse(response, levelsMap, fd, levels_queues);
		}
		FD_CLR(fd, socks);
}

void executeResponse(char* response, t_dictionary *levelsMap, int *fd, t_dictionary *levels_queues) {

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
		free(socketsToGo);
		free(response);
		free(level);
		close(fd);
	} else if (string_starts_with(response, FREERESC)) {

		response = string_substring_from(response, sizeof(FREERESC));

		char** data = (char*) malloc(MAXSIZE);

		data = string_split(response, COMA);

		t_scheduler_queue *queues = dictionary_get(levels_queues, data[0]);

		if(queue_size(queues->blocked_queue) == 0){
			sendMessage(fd, OKEY);
			close(fd);
		}
		else{

			sendMessage(fd, NOTOK);

			int hongos = (int*) malloc(sizeof(int));
			int monedas = (int*) malloc(sizeof(int));
			int flores = (int*) malloc(sizeof(int));

			flores = atoi(data[1]);
			monedas = atoi(data[2]);
			hongos = atoi(data[3]);

			int i = 0;

			for(i = 0 ; i < queue_size(queues->blocked_queue) ; i++){

				blocked_character *blockedCharacter = queue_pop(queues->blocked_queue);

				if(blockedCharacter->recurso == 'F'){
					giveResource(queues, flores, blockedCharacter);
				}
				else if (blockedCharacter->recurso == 'H'){
					giveResource(queues, hongos, blockedCharacter);
				}
				else if(blockedCharacter->recurso == 'M'){
					giveResource(queues, monedas, blockedCharacter);
				}

				t_level_address *addresses = (t_level_address*) dictionary_get(levelsMap, data[0]);

				char **levelSocket = string_split(addresses->nivel, DOSPUNTOS);

				int fdNivel = openSocketClient(levelSocket[1], levelSocket[0]);

				sendMessage(fdNivel, string_from_format("RSC,%d,%d,%d", flores, monedas, hongos));

				free(data);
			}

		}
	} else if (string_starts_with(response, DEADLOCK)) {

		response = string_substring_from(response, sizeof(DEADLOCK));
		char** deadlockPointer = string_split(response, COMA);

		int i = 0;
		int size = atoi(deadlockPointer[0]) + 1;
		int selected = atoi(deadlockPointer[1]);

		for(i = 2 ; i<size ; i++){
			if(atoi(deadlockPointer[i]) < selected){
				selected = atoi(deadlockPointer[i]);
			}
		}

		sendMessage(fd, string_from_format("%d,",selected));

	}

}

void giveResource(t_scheduler_queue *queues, int recurso, blocked_character *blockedCharacter){
	if(recurso > 0){
		recurso--;
		queue_push(queues->character_queue, blockedCharacter->fd);
	}
}
