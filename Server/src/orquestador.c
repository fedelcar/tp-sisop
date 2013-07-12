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
#include "uncommons/inotify.h"
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
#define TIEMPOCHEQUEODEADLOCK "TiempoChequeoDeadlock"
#define TURNOS "turnos"
#define INTERVALO "intervalo"

char* stringRecursos(t_list *simbolos, t_dictionary *recursosDisponibles, int fd);
void executeResponse(char* response, t_dictionary *levelsMap, int fd,
		t_dictionary *levels_queues, fd_set *socks,
		t_orquestador *orquestador_config, char* path, t_list *niveles);
int giveResource(t_scheduler_queue *queues, int *recurso,
		blocked_character *blockedCharacter);
//void orquestador(t_dictionary *levelsMap, int fd, t_dictionary *levels_queues, fd_set *socks);
void orquestador(t_dictionary *levelsMap, int fd, t_dictionary *levels_queues,
		fd_set *socks, t_orquestador *orquestador_config, char* path,
		t_list *niveles);
int *generateSocket(int* portInt, int *scheduler_port);
void executeKoopa(t_list *niveles, t_dictionary* levels_queues, t_orquestador *orquestador_config);

int flagTerminoUnPersonaje;
t_log *log;

int main(int argc, char **argv) {

	flagTerminoUnPersonaje = FALSE;


	/**
	 * Lista de niveles
	 */
	t_orquestador *orquestador_config = getOrquestador(
			"/home/tp/config/orquestador/orquestador.config"); //argv[0]


	char* pathLog = (char*) malloc(MAXSIZE);
	memset(pathLog, 0, sizeof(pathLog));
	string_append(&pathLog, "/home/tp/config/logs/orquestador.txt");


	log = log_create(pathLog, "Plataforma", 1, LOG_LEVEL_DEBUG);

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
		log_error(log, "socket() failed");
		exit(-1);
	}

	rc = setsockopt(listen_sd, SOL_SOCKET, SO_REUSEADDR, (char *) &on,
			sizeof(on));
	if (rc < 0) {
		log_error(log, "setsockopt() failed");
		close(listen_sd);
		exit(-1);
	}

	rc = ioctl(listen_sd, FIONBIO, (char *) &on);
	if (rc < 0) {
		log_error(log, "ioctl() failed");
		close(listen_sd);
		exit(-1);
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY );
	addr.sin_port = htons(orquestador_config->puerto);
	rc = bind(listen_sd, (struct sockaddr *) &addr, sizeof(addr));
	if (rc < 0) {
		log_error(log, "bind() failed");
		close(listen_sd);
		exit(-1);
	}

	rc = listen(listen_sd, 32);
	if (rc < 0) {
		log_error(log, "listen() failed");
		close(listen_sd);
		exit(-1);
	}

	FD_ZERO(&master_set);
	max_sd = listen_sd;
	FD_SET(listen_sd, &master_set);

	do {

		memcpy(&working_set, &master_set, sizeof(master_set));


		rc = select(FD_SETSIZE, &working_set, NULL, NULL, NULL );

		if (rc < 0) {
			log_error(log, "  select() failed");
			break;
		}

		desc_ready = rc;
		for (j = 0; j <= max_sd && desc_ready > 0; ++j) {

			if (FD_ISSET(j, &working_set)) {

				desc_ready -= 1;

				if (j == listen_sd) {

					do {
						new_sd = accept(listen_sd, NULL, NULL );
						if (new_sd < 0) {
							if (errno != EWOULDBLOCK) {
								log_error(log, "  accept() failed");
								end_server = TRUE;
							}
							break;
						}

						FD_SET(new_sd, &master_set);
						if (new_sd > max_sd)
							max_sd = new_sd;

					} while (new_sd != -1);
				}

				else {
					close_conn = FALSE;

					orquestador(levelsMap, j, levels_queues, &master_set,
							orquestador_config,
							"/home/tp/config/orquestador/orquestador.config",
							niveles); //argv[0]

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
		fd_set *socks, t_orquestador *orquestador_config, char* path,
		t_list *niveles) {

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
		if (!dictionary_has_key(levelsMap, (string_split(response, PIPE))[0])) {
			sendMessage(fd, "No existe el nivel pedido");
		} else {
			string_append(&socketsToGo,
					((t_level_address*) (dictionary_get(levelsMap,
							(string_split(response, PIPE))[0])))->planificador);
			string_append(&socketsToGo, COMA);
			string_append(&socketsToGo,
					((t_level_address*) (dictionary_get(levelsMap,
							(string_split(response, PIPE))[0])))->nivel);
			string_append(&socketsToGo, COMA);
			sendMessage(fd, socketsToGo);
		}
		log_info(log, "Conexión de personaje recibida, petición de IP y Puerto de nivel.");
		FD_CLR(fd, socks);
		free(socketsToGo);
		free(response);
		free(level);
		close(fd);
	} else if (string_starts_with(response, NEWLVL)) {

		response = string_substring_from(response, sizeof(NEWLVL));
		char** split = string_split(response, COMA);

//		len = sizeof addr;
//		getpeername(fd, (struct sockaddr*) &addr, &len);
//
//		struct sockaddr_in *s = (struct sockaddr_in *) &addr;
//
//		inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);

		pthread_t *t = (pthread_t*) malloc(sizeof(pthread_t));

		t_scheduler_queue *scheduler_queue = (t_scheduler_queue*) malloc(
				sizeof(t_scheduler_queue));

		int *scheduler_port = (int*) malloc(sizeof(int));

		scheduler_queue->blocked_queue = queue_create();
		scheduler_queue->character_queue = queue_create();
		scheduler_queue->listen_sd = generateSocket(DEFAULTPORT,
				scheduler_port);
		scheduler_queue->orquestador_config = orquestador_config;
		scheduler_queue->path = path; //argv[0]
		scheduler_queue->pjList = list_create();
		scheduler_queue->simbolos = list_create();
		scheduler_queue->log = log;


		inotify_struct *datos = (inotify_struct*) malloc(sizeof(inotify_struct));
		inotify_list_struct* data = (inotify_list_struct*) malloc(
				sizeof(inotify_list_struct));
		inotify_list_struct* data2 = (inotify_list_struct*) malloc(
				sizeof(inotify_list_struct));

		datos->path = path;
		datos->lista = list_create();

		data->nombre = TURNOS;
		data->valor = &(orquestador_config->turnos);
		list_add(datos->lista, data);

		data2->nombre = INTERVALO;
		data2->valor = &(orquestador_config->intervalo);
		list_add(datos->lista, data2);

		pthread_t *thread_inot = (pthread_t*) malloc(sizeof(pthread_t));

		pthread_create(thread_inot, NULL, (void *) inotify,
				(inotify_struct *) datos);



		char** simbolos = string_split(split[2], DOSPUNTOS);

		int p = 0;

		for (p = 1; p < atoi(simbolos[0]) + 1; p++) {
			list_add(scheduler_queue->simbolos, simbolos[p]);
		}

		t_level_address *level = (t_level_address *) malloc(
				sizeof(t_level_address));

		level->nivel = string_from_format("%s:%s", split[3], split[0]);
		level->planificador = string_from_format("%s:%d", "127.0.0.1",
				*scheduler_port);

		list_add(niveles, split[1]);

		dictionary_put(levelsMap, split[1], level);
		dictionary_put(levels_queues, split[1], scheduler_queue);

		log_info(log, "Conexión de nuevo nivel: %s", split[1]);

		pthread_create(t, NULL, (void *) planificador,
				(t_scheduler_queue*) scheduler_queue);

	} else if (string_starts_with(response, FREERESC)) {

		response = string_substring_from(response, sizeof(FREERESC));

		char** data = (char*) malloc(MAXSIZE);

		data = string_split(response, COMA);

		t_scheduler_queue *queues = dictionary_get(levels_queues, data[0]);

		char** simbolos;

		int j = 2;

		t_dictionary *recursosDisponibles = dictionary_create();

		for (j = 2; j < list_size(queues->simbolos) + 2; j++) {
			simbolos = string_split(data[j], DOSPUNTOS);
			dictionary_put(recursosDisponibles, simbolos[0], atoi(simbolos[1]));
		}

		if (queue_size(queues->blocked_queue) > 0) {

			int i = 0;

			int k = 0;

			for (i = 0; i < queue_size(queues->blocked_queue); i++) {

				blocked_character *blockedCharacter = queue_pop(
						queues->blocked_queue);

				for (k = 0; k < list_size(queues->simbolos); k++) {
					if (blockedCharacter->recurso
							== ((char*) list_get(queues->simbolos, k))[0]) {

						if (giveResource(queues,
								dictionary_get(recursosDisponibles,
										list_get(queues->simbolos, k)),
								blockedCharacter) == 1) {
							queue_push(queues->character_queue,
									blockedCharacter->personaje);
						} else {
							queue_push(queues->blocked_queue,
									blockedCharacter->personaje);
						}
					}
				}
				k = 0;
			}
		}
		t_level_address *addresses = (t_level_address*) dictionary_get(
				levelsMap, data[0]);

		char **levelSocket = string_split(addresses->nivel, DOSPUNTOS);

		int fdNivel = openSocketClient(levelSocket[1], levelSocket[0]);

		sendMessage(fdNivel,
				stringRecursos(queues->simbolos, recursosDisponibles, atoi(data[1])));

		free(data);

		log_info(log, "Liberar recursos.");
		if(flagTerminoUnPersonaje == TRUE){
			executeKoopa(niveles, levels_queues, orquestador_config);
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
		log_info(log, "Resolviendo deadlock");
		sendMessage(fd, string_from_format("%d,", selected));

	} else if (string_starts_with(response, "TNIVEL")) {
		response = string_substring_from(response, sizeof("TNIVEL"));
		char** split = string_split(response, COMA);

		t_scheduler_queue *scheduler_queue = dictionary_get(levels_queues,
				split[1]);
		int i = 0;
		for (i = 0; i < queue_size(scheduler_queue->blocked_queue); i++) {
			blocked_character *blockedCharacter = queue_pop(
					scheduler_queue->blocked_queue);
			if (!string_equals_ignore_case(split[0],
					blockedCharacter->personaje->nombre)) {
				queue_push(scheduler_queue->blocked_queue, blockedCharacter);
			}
		}

		i = 0;

		for (i = 0; i < list_size(scheduler_queue->pjList); i++) {
			if (string_equals_ignore_case(split[0],
					((personaje_planificador*) list_get(scheduler_queue->pjList,
							i))->nombre)) {
				list_remove(scheduler_queue->pjList, i);
			}
		}
		log_info(log, "Un personaje finalizo su plan de niveles");
	} else if (string_starts_with(response, "Termine todo")) {

		flagTerminoUnPersonaje = TRUE;

		executeKoopa(niveles, levels_queues, orquestador_config);

	}

}

int giveResource(t_scheduler_queue *queues, int *recurso,
		blocked_character *blockedCharacter) {
	if (recurso > 0) {
		int value = recurso;
		recurso = value - 1;
		return 1;
	} else {
		return 0;
	}
}

int *generateSocket(int* portInt, int *scheduler_port) {

	int rc, on = 1;
	int *listen_sd = (int*) malloc(sizeof(int));
	struct sockaddr_in addr;

	listen_sd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sd < 0) {
		log_error(log, "socket() failed");
		exit(-1);
	}

	/*************************************************************/
	/* Allow socket descriptor to be reuseable                   */
	/*************************************************************/
	rc = setsockopt(listen_sd, SOL_SOCKET, SO_REUSEADDR, (char *) &on,
			sizeof(on));
	if (rc < 0) {
		log_error(log, "setsockopt() failed");
		close(listen_sd);
		exit(-1);
	}

	rc = ioctl(listen_sd, FIONBIO, (char *) &on);
	if (rc < 0) {
		log_error(log, "ioctl() failed");
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

char* stringRecursos(t_list *simbolos, t_dictionary *recursosDisponibles, int fd) {

	char* stringRecursos = (char*) malloc(MAXSIZE);

	string_append(&stringRecursos, string_from_format("RSC,%d,", fd));

	int k = 0;

	int a = 0;
	char* b;

	for (k = 0; k < list_size(simbolos); k++) {
		string_append(&stringRecursos,
				string_from_format("%s:%d,", list_get(simbolos, k),
						dictionary_get(recursosDisponibles,
								list_get(simbolos, k))));
	}

	return stringRecursos;
}

void executeKoopa(t_list *niveles, t_dictionary* levels_queues, t_orquestador *orquestador_config){

	int i = 0;
	char* nivel;
	t_scheduler_queue *scheduler;
	int final = TRUE;
	for (i = 0; i < list_size(niveles); i++) {
		nivel = (char*) list_get(niveles, i);
		scheduler = dictionary_get(levels_queues, nivel);
		if (list_size(scheduler->pjList) > 0) {
			final = FALSE;
		}
	}
	if (final == TRUE) {
		log_info(log, "Ejecutando Koopa");
			char * arg1 = orquestador_config->argumento1;
			char * arg2[] = { "koopa", orquestador_config->argumento2, NULL };
			char * arg3[] = { orquestador_config->argumento3, "TERM=xterm", NULL };
			int ejecKoopa = execve(arg1, arg2, arg3);
	}

}
