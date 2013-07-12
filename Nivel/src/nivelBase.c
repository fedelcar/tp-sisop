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
#include "uncommons/inotify.h"
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

#define TIEMPOCHEQUEODEADLOCK "TiempoChequeoDeadlock"

//void openListener(char* argv, queue_n_locks *queue);
void agregarRecursos(t_dictionary *recursosAt, ITEM_NIVEL *listaItems, t_list *listasSimbolos);
void saveList(resource_struct *resourceStruct, t_list *threads);
void deteccionInterbloqueo(deadlock_struct *deadlockStruct);
void analize_response(int fd, t_list *threads, t_level_config *nivel,
		ITEM_NIVEL *listaItems, t_dictionary *listaPersonajes, int rows,
		int cols, fd_set *master_set, int socketOrquestador, t_list *listaSimbolos);
resource_struct *getLevelStructure(t_level_config *level_config, int *fd);
void agregarRecursosOrquestador(char *bufferSocket, ITEM_NIVEL *listaItems, t_list *listaSimbolos);

char* endingStringBroken(t_dictionary *recursosAt, char* nivel, t_list *listaSimbolos, int fd);

ITEM_NIVEL* cambiarEstructura(t_level_config* levelConfig, t_list *listaSimbolos);

int id;
t_log* log;

int main(int argc, char **argv) {

	id = 0;

	t_list *listaSimbolos = list_create();

	t_level_config *nivel = (t_level_config*) malloc(sizeof(t_level_config));

	nivel = getLevel(argv[1], listaSimbolos); //argv[0]

	char* pathLog = (char*) malloc(MAXSIZE);
	memset(pathLog, 0, sizeof(pathLog));
	string_append(&pathLog, "/home/tp/config/logs/");
	string_append(&pathLog, nivel->nombre);
	string_append(&pathLog, ".txt");

	log = log_create(pathLog, "Nivel", 1, LOG_LEVEL_DEBUG);


	//Creo el thread nesesario para el inotify

	inotify_struct *datos = (inotify_struct*)malloc(sizeof(inotify_struct));
	inotify_list_struct* data = (inotify_list_struct*)malloc(sizeof(inotify_list_struct));
	datos->path = argv[1];
	datos->lista = list_create();
	datos->log = log;

	data->nombre = TIEMPOCHEQUEODEADLOCK;
	data->valor = &(nivel->tiempoChequeoDeadlock);
	list_add(datos->lista, data);


	pthread_t *thread_inot = (pthread_t*) malloc(sizeof(pthread_t));
	pthread_create(thread_inot, NULL, (void *) inotify,
			(inotify_struct *) datos);



	ITEM_NIVEL *listaItems = cambiarEstructura(nivel, listaSimbolos);

	t_dictionary *listaPersonajes = dictionary_create();

	char **ipPuerto = string_split(nivel->orquestador, DOSPUNTOS);

	int socketOrquestador = openSocketClient(ipPuerto[1], ipPuerto[0]);
	log_debug(log, string_from_format(
			"Me conecto con el Orquestador. IP:%s Puerto:%s",
			ipPuerto[0], ipPuerto[1]));

	t_list *threads = list_create();

	int *id = (int*) malloc(sizeof(int));

	id = 0;

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
		log_error(log, "listen() failed");
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

	char *simbolos = (char*) malloc(MAXSIZE);

	int k = 0;

	simbolos = string_from_format("%d:", list_size(listaSimbolos));

	for(k = 0 ; k < list_size(listaSimbolos) ; k++){
		string_append(&simbolos, string_from_format("%s:", list_get(listaSimbolos, k)));

	}


	sendMessage(socketOrquestador,
			string_from_format("NEWLVL,%d,%s,%s,%s", ntohs(s->sin_port),
					nivel->nombre, simbolos, nivel->localIp));
	log_info(log, "Hand shake con Orquestador");
	free(simbolos);

	pthread_t detectionThread;

	deadlock_struct *deadlockStruct = (deadlock_struct*) malloc(
			sizeof(deadlock_struct));

	deadlockStruct->items = listaItems;
	deadlockStruct->list = threads;
	deadlockStruct->socket = socketOrquestador;
	deadlockStruct->puerto = portForLevel;
	deadlockStruct->recovery = nivel->recovery;
	deadlockStruct->checkDeadlock = &(nivel->tiempoChequeoDeadlock);
	deadlockStruct->path = argv[1];

	pthread_create(&detectionThread, NULL, (void*) deteccionInterbloqueo,
			(deadlock_struct*) deadlockStruct);

	/*************************************************************/
	/* Initialize the master fd_set                              */
	/*************************************************************/
	FD_ZERO(&master_set);
	max_sd = listen_sd;
	FD_SET(listen_sd, &master_set);

	do {

		memcpy(&working_set, &master_set, sizeof(master_set));

		rc = select(FD_SETSIZE, &working_set, NULL, NULL, NULL );


		if (rc < 0) {
			log_error(log, "select() failed");
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
								log_error(log, "accept() failed");
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

					analize_response(j, threads, nivel, listaItems,
							listaPersonajes, rows, cols, &master_set,
							socketOrquestador, listaSimbolos);
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

void analize_response(int fd, t_list *threads, t_level_config *nivel,
		ITEM_NIVEL *listaItems, t_dictionary *listaPersonajes, int rows,
		int cols, fd_set *master_set, int socketOrquestador, t_list *listaSimbolos) {

	char* bufferSocket = (char*) malloc(MAXSIZE);
	memset(bufferSocket, 0, sizeof(bufferSocket));
	bufferSocket = recieveMessage(fd);
	if (string_starts_with(bufferSocket, "BROKEN")) {
		if (dictionary_has_key(listaPersonajes, string_from_format("%d", fd))) {
			resource_struct *personajeABorrar = dictionary_get(listaPersonajes,
					string_from_format("%d", fd));
			log_info(log, string_from_format("Desconexión repentina de: %s."), personajeABorrar->nombre);
			sleep(1);
			sendMessage(socketOrquestador ,endingStringBroken(personajeABorrar->recursosAt, nivel->nombre, listaSimbolos, fd));
			log_info(log, "Peticion de liberación de recursos");
			nivel_gui_dibujar(listaItems);
			dictionary_remove(listaPersonajes, string_from_format("%d", fd));
			int i = 0;
			for(i = 0 ; i < list_size(threads) ; i++){
				if(fd == *(((datos_personaje*) list_get(threads, i))->fd)){
					list_remove(threads, i);
				}
			free(personajeABorrar);
		}
		}
		FD_CLR(fd, master_set);
		return;
	}

	if (string_starts_with(bufferSocket, START)) {
		bufferSocket = string_substring_from(bufferSocket, sizeof(START));
		bufferSocket = string_substring_from(bufferSocket, sizeof(SIMBOLO));
		char** split = string_split(bufferSocket, PIPE);
		t_dictionary *recursosAt = dictionary_create();

		int i = 0;

		for(i = 0 ; i < list_size(listaSimbolos) ; i++){
			uint32_t *zero = (uint32_t*) malloc(sizeof(int));
			*zero = 0;
			dictionary_put(recursosAt, list_get(listaSimbolos, i),zero);
		}

		resource_struct *resourceStruct = getLevelStructure(nivel, fd);

		resourceStruct->recursosAt = recursosAt;

		resourceStruct->recursoBloqueado = "0";

		resourceStruct->simbolo = split[0][0];

		resourceStruct->posicion = (t_posicion*) malloc(sizeof(t_posicion));

		resourceStruct->posicion->posX = 1;
		resourceStruct->posicion->posY = 1;

		resourceStruct->nombre = split[1];
		log_info(log, string_from_format("Conexión del personaje %s", resourceStruct->nombre));
		CrearPersonaje(&listaItems, resourceStruct->simbolo, 1, 1);

		resourceStruct->listaItems = listaItems;

		saveList(resourceStruct, threads);

		dictionary_put(listaPersonajes, string_from_format("%d", fd),
				resourceStruct);

		sendMessage(fd, "ok");

		id++;
	} else if (string_starts_with(bufferSocket, RESOURCES)) {
		char **split = string_split(bufferSocket, COMA);
		int i = 0;
		log_info(log, "Liberando recursos indicados por el Orquestador");
		agregarRecursosOrquestador(bufferSocket, listaItems, listaSimbolos);
		nivel_gui_dibujar(listaItems);
		for(i = 0 ; i < list_size(threads) ; i++){
			if(atoi(split[1]) == *(((datos_personaje*) list_get(threads, i))->fd) ){
				list_remove(threads, i);
			}
			FD_CLR(atoi(split[1]), master_set);
		}
	} else if (string_starts_with(bufferSocket, MOVIMIENTO)) {
		bufferSocket = string_substring_from(bufferSocket, sizeof(MOVIMIENTO));
		resource_struct *personaje = (resource_struct*) dictionary_get(
				listaPersonajes, string_from_format("%d", fd));
		log_info(log, string_from_format("Petición del personaje: %s", personaje->nombre));
		movimientoPersonaje(personaje, rows, cols, bufferSocket, master_set, fd,
				socketOrquestador, listaSimbolos, log);
	} else if (string_starts_with(bufferSocket, DEATH)){
		bufferSocket = string_substring_from(bufferSocket, sizeof(DEATH));
		char** split = string_split(bufferSocket, COMA);
		sendMessage(atoi(split[0]), DEATH);
	}
	free(bufferSocket);
}

//TODO PENDIENTE BORRAR ITEMS

void agregarRecursos(t_dictionary *recursosAt, ITEM_NIVEL *listaItems, t_list *listaSimbolos) {

	ITEM_NIVEL* temp;

	int i = 0;

	for(i = 0 ; i < list_size(listaSimbolos) ; i++){
		temp = buscarRecurso(listaItems, ((char*) list_get(listaSimbolos, i))[0]);
		temp->quantity = temp->quantity + (int) dictionary_get(recursosAt, list_get(listaSimbolos, i));
	}

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

ITEM_NIVEL* cambiarEstructura(t_level_config* levelConfig, t_list *listaSimbolos) {


	ITEM_NIVEL* listaItems = (ITEM_NIVEL*) malloc(sizeof(ITEM_NIVEL));
	listaItems = NULL;

	int i = 0;

	for(i = 0 ; i < list_size(listaSimbolos) ; i++){

		if (dictionary_has_key(levelConfig->cajas, list_get(listaSimbolos, i))) {
			t_caja *caja = dictionary_get(levelConfig->cajas, list_get(listaSimbolos, i));
			CrearCaja(&listaItems, ((char*) list_get(listaSimbolos, i))[0], (int) caja->posX, (int) caja->posY,
					(int) caja->instancias);
		}

	}

	return listaItems;
}

void deteccionInterbloqueo(deadlock_struct *deadlockStruct) {


	int fdNivel = openSocketClient(string_from_format("%d", deadlockStruct->puerto), "localhost");

	char *bufferDeadlock = (char*) malloc(MAXSIZE);

	int j = 0;

	char *deadlockMessage = (char*) malloc(MAXSIZE);

	t_list *deadlockList;
	while (1) {

		sleep(*(deadlockStruct->checkDeadlock)); //levantar de archivo de configuracion, inotify

		deadlockList = detectionAlgorithm(deadlockStruct->items,
				deadlockStruct->list);
		int x;
		for (x=0;x<list_size(deadlockStruct);x++){
			datos_personaje* pers = list_get(deadlockStruct,x);
			log_debug(log,"Personaje involucrado en deadlock: %s", pers->fd);
		}

		log_debug(log, "Recovery: %d", deadlockStruct->recovery);

		if (list_size(deadlockList) > 1
				&& deadlockStruct->recovery == 1) {


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

			for (j = 0; j < list_size(deadlockStruct->list); j++) {

				datos = (datos_personaje*) list_get(deadlockStruct->list, j);

				if (datos->id == atoi(response[0])) {
					sendMessage(fdNivel, string_from_format("DEATH,%d,", *(datos->fd)));
					log_debug(log, "Victima: %s" , datos->nombre);
				}

			}

		}
	}

	free(bufferDeadlock);
	free(deadlockMessage);
}

void saveList(resource_struct *resourceStruct, t_list *threads) {

	datos_personaje *datos = (datos_personaje*) malloc(sizeof(datos_personaje));

	datos->recursosAt = resourceStruct->recursosAt;
	datos->recurso = &(resourceStruct->recursoBloqueado);
	datos->id = id;
	datos->fd = &resourceStruct->fd;
	datos->nombre = resourceStruct->nombre;

	list_add(threads, datos);

}

void agregarRecursosOrquestador(char *bufferSocket, ITEM_NIVEL *listaItems, t_list *listaSimbolos){

	bufferSocket = string_substring_from(bufferSocket, sizeof(RESOURCES));

	char** data = (char*) malloc(MAXSIZE);

	data = string_split(bufferSocket, COMA);

	char** simbolos;

	int j = 0;

	t_dictionary *recursosDisponibles = dictionary_create();

	for (j = 1; j < list_size(listaSimbolos) + 1; j++) {
		simbolos = string_split(data[j], DOSPUNTOS);
		dictionary_put(recursosDisponibles, simbolos[0], atoi(simbolos[1]));
	}

	agregarRecursos(recursosDisponibles, listaItems, listaSimbolos);

	free(recursosDisponibles);

}

char* endingStringBroken(t_dictionary *recursosAt, char* nivel, t_list *listaSimbolos, int fd){

	char* lastString = (char*) malloc(MAXSIZE);

	char* nivelLocal = (char*) malloc(MAXSIZE);

	strcpy(nivelLocal, nivel);

	string_append(&lastString, "FREERESC,");

	string_append(&lastString, string_from_format("%s,%d," ,nivel, fd));

//	char *recursos = (char*) malloc(MAXSIZE);

	int k = 0;

	for(k = 0 ; k < list_size(listaSimbolos) ; k++){

		string_append(&lastString, string_from_format("%s:%d,",list_get(listaSimbolos, k), *((uint32_t *) dictionary_get(recursosAt, list_get(listaSimbolos, k))) ));

	}

	return lastString;
}

