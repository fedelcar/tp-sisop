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

#define DEFAULTPORT 9930
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
#define NEWLVL "NEWLVL"

//void accionar(int sock, int *connectlist, int *highsock, fd_set *socks, t_dictionary *levelsMap, t_dictionary *levels_queues);
void executeResponse(char* response, t_dictionary *levelsMap, int fd,
		t_dictionary *levels_queues, fd_set *socks,
		t_orquestador *orquestador_config, char* path, t_list *niveles);
void giveResource(t_scheduler_queue *queues, int *recurso,
		blocked_character *blockedCharacter);
//void orquestador(t_dictionary *levelsMap, int fd, t_dictionary *levels_queues, fd_set *socks);
void orquestador(t_dictionary *levelsMap, int fd, t_dictionary *levels_queues,
		fd_set *socks, t_orquestador *orquestador_config, char* path, t_list *niveles);
int *generateSocket(int* portInt, int *scheduler_port);

int main(int argc, char **argv) {

	/**
	 * Lista de niveles
	 */
	t_orquestador *orquestador_config = getOrquestador(
			"/home/tp/config/orquestador/orquestador.config"); //argv[0]


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

//	levelsMap = getLevelsMap();

	char** port = (char*) malloc(MAXSIZE);

	int i;

	t_dictionary *levels_queues = dictionary_create();

	t_list *niveles = list_create();

	free(levelName);

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
	addr.sin_port = htons(orquestador_config->puerto);
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

					orquestador(levelsMap, j, levels_queues, &master_set,
							orquestador_config,
							"/home/tp/config/orquestador/orquestador.config", niveles); //argv[0]

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

void orquestador(t_dictionary *levelsMap, int fd, t_dictionary *levels_queues,
		fd_set *socks, t_orquestador *orquestador_config, char* path, t_list *niveles) {

	char *response = (char *) malloc(MAXSIZE);

	response = recieveMessage(fd);

	if (string_starts_with(response, BROKEN)) {
		FD_CLR(fd, socks);
	} else {
		executeResponse(response, levelsMap, fd, levels_queues, socks,
				orquestador_config, path, niveles);
	}

}

void executeResponse(char* response, t_dictionary *levelsMap, int fd,
		t_dictionary *levels_queues, fd_set *socks,
		t_orquestador *orquestador_config, char* path, t_list *niveles) {

	if (string_starts_with(response, LEVEL)) {
		response = string_substring_from(response, sizeof(LEVEL));
		t_level_address *level = (t_level_address *) malloc(
				sizeof(t_level_address));
		char *socketsToGo = (char*) malloc(MAXSIZE);
		memset(socketsToGo, 0, sizeof(socketsToGo));
		if(!dictionary_has_key(levelsMap, (string_split(response, PIPE))[0])){
			sendMessage(fd, "No existe el nivel pedido");
		}
		else{
			string_append(&socketsToGo,
					((t_level_address*) (dictionary_get(levelsMap,
							(string_split(response, PIPE))[0])))->planificador);
			string_append(&socketsToGo, COMA);
			string_append(&socketsToGo,
					((t_level_address*) (dictionary_get(levelsMap,
							(string_split(response, PIPE))[0])))->nivel);
			sendMessage(fd, socketsToGo);
		}
		FD_CLR(fd, socks);
		free(socketsToGo);
		free(response);
		free(level);
		close(fd);
	} else if (string_starts_with(response, NEWLVL)) {

		response = string_substring_from(response, sizeof(NEWLVL));
		char** split = string_split(response, COMA);

		socklen_t len;
		struct sockaddr_storage addr;
		char ipstr[INET_ADDRSTRLEN];

		len = sizeof addr;
		getpeername(fd, (struct sockaddr*) &addr, &len);

		struct sockaddr_in *s = (struct sockaddr_in *) &addr;

		inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);

		pthread_t *t = (pthread_t*) malloc(sizeof(pthread_t));

		t_scheduler_queue *scheduler_queue = (t_scheduler_queue*) malloc(
				sizeof(t_scheduler_queue));

		int *scheduler_port = (int*) malloc(sizeof(int));

		scheduler_queue->blocked_queue = queue_create();
		scheduler_queue->character_queue = queue_create();
		scheduler_queue->listen_sd = generateSocket(DEFAULTPORT, scheduler_port);
		scheduler_queue->orquestador_config = orquestador_config;
		scheduler_queue->path = path; //argv[0]
		scheduler_queue->pjList = list_create();

		t_level_address *level = (t_level_address *) malloc(
				sizeof(t_level_address));

		level->nivel = string_from_format("%s:%s", ipstr, split[0]);
		level->planificador = string_from_format("%s:%d", ipstr, *scheduler_port);

		list_add(niveles, split[1]);

		dictionary_put(levelsMap, split[1], level);
		dictionary_put(levels_queues, split[1], scheduler_queue);

		pthread_create(t, NULL, (void *) planificador,
				(t_scheduler_queue*) scheduler_queue);

	} else if (string_starts_with(response, FREERESC)) {

		response = string_substring_from(response, sizeof(FREERESC));

		char** data = (char*) malloc(MAXSIZE);

		data = string_split(response, COMA);

		t_scheduler_queue *queues = dictionary_get(levels_queues, data[0]);


		int flores = atoi(data[1]);
		int monedas = atoi(data[2]);
		int hongos = atoi(data[3]);

		if (queue_size(queues->blocked_queue) > 0) {

//			int hongos = (int*) malloc(sizeof(int));
//			int monedas = (int*) malloc(sizeof(int));
//			int flores = (int*) malloc(sizeof(int));

			int i = 0;

			for (i = 0; i < queue_size(queues->blocked_queue); i++) {

				blocked_character *blockedCharacter = queue_pop(
						queues->blocked_queue);

				if (blockedCharacter->recurso == 'F') {
					giveResource(queues, &flores, blockedCharacter);
				} else if (blockedCharacter->recurso == 'H') {
					giveResource(queues, &hongos, blockedCharacter);
				} else if (blockedCharacter->recurso == 'M') {
					giveResource(queues, &monedas, blockedCharacter);
				}
			}
		}
		t_level_address *addresses = (t_level_address*) dictionary_get(
				levelsMap, data[0]);

		char **levelSocket = string_split(addresses->nivel, DOSPUNTOS);

		int fdNivel = openSocketClient(levelSocket[1], levelSocket[0]);

		sendMessage(fdNivel,
				string_from_format("RSC,%d,%d,%d", flores, monedas,
						hongos));

		free(data);

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
	else if(string_starts_with(response, "TNIVEL")){
		response = string_substring_from(response, sizeof("TNIVEL"));
		char** split = string_split(response, COMA);
		char* ressssponse = split[1];
		t_scheduler_queue *scheduler_queue = dictionary_get(levels_queues, split[1]);
		int i = 0;
		for(i = 0 ; i < list_size(scheduler_queue->pjList) ; i++){
			if(string_equals_ignore_case(split[0], ( (personaje_planificador*)list_get(scheduler_queue->pjList, i))->nombre)){
				list_remove(scheduler_queue->pjList, i);
			}
		}

	}
	else if(string_starts_with(response, "Termine todo")){
		int i = 0;
		char* nivel;
		t_scheduler_queue *scheduler;
		int final = TRUE;
		for(i = 0 ; i < list_size(niveles) ; i++){
			nivel = (char*) list_get(niveles, i);
			scheduler = dictionary_get(levels_queues, nivel);
			if(list_size(scheduler->pjList) > 0){
				final = FALSE;
			}
		}
		if(final == TRUE){
			char * arg1   = orquestador_config->argumento1;
			char * arg2[] = {orquestador_config->argumento2, NULL};
			char * arg3[] = {orquestador_config->argumento3, "TERM=xterm", NULL};
			int ejecKoopa = execve(arg1, arg2, arg3);
		}
	}

//	FD_CLR(fd, socks);

}

void giveResource(t_scheduler_queue *queues, int *recurso,
		blocked_character *blockedCharacter) {
	if (*recurso > 0) {
		int value = *recurso;
		*recurso = value - 1;
		queue_push(queues->character_queue, blockedCharacter->personaje);
	}
}

int *generateSocket(int* portInt, int *scheduler_port) {

	int rc, on = 1;
	int *listen_sd = (int*) malloc(sizeof(int));
	struct sockaddr_in addr;

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

	while (1) {
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_ANY );
		addr.sin_port = htons(portInt);
		rc = bind(listen_sd, (struct sockaddr *) &addr, sizeof(addr));
		if (rc < 0) {
			portInt++;
			continue;
		}
		break;
	}

	*scheduler_port = portInt;

	return listen_sd;
}
