/*
 ============================================================================
 Name        : personaje.c
 Author      :
 Version     :
 Copyright   : Your copyright notice
 Description : Proceso Personaje del TP de SO
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <commons/temporal.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/collections/node.h>
#include <uncommons/fileStructures.h>
#include <uncommons/SocketsCliente.h>
#include <uncommons/SocketsBasic.h>
#include <sys/types.h>
#include <sys/socket.h>
//#include "sisdeps.h"
#include <unistd.h>
//#include <signal.h>
#include <string.h>

#define MAXSIZE 1024
//#define COMA ","
//#define TUTURNO "Tu turno"
#define ENDSTRING "|\0"

char* extraerIpPlanificador(char* mensaje);
char* extraerIpNivel(char* mensaje);
char* extraerPuertoPlanificador(char* mensaje);
char* extraerPuertoNivel(char* mensaje);
char* extraerIp(char* direccion);
char* extraerPuerto(char* direccion);
t_posicion stringToPosicion(char* buffer);
t_posicion calcularMovimiento(t_posicion* miPos, t_posicion* posRec);
//bool termineNivel(t_link_element* recursosNivel, int cantRecAcum);
char* posicionToString(t_posicion* miPos);
t_posicion setPosicion(int x, int y);

static const char* TERMINE_NIVEL = "Termine nivel";
static const char* RECHAZO = "Rechazo";
static const char* BLOCKED = "BLOCKED";
static const char* OK = "ok";
static const char* DAME_RECURSO = "Dame recurso";
static const char* TERMINE_TODO = "Termine todo";
static const char* TU_TURNO = "Tu Turno";

bool sonPosicionesIguales(t_posicion* pos1, t_posicion* pos2);
int vidas;
//void term(int signum);


int main(char* character) {

	t_log* log = log_create("/home/lucas/log.txt", "Personaje", 1,
			LOG_LEVEL_DEBUG);

//---------- Inicializar Punteros ------------
	char* nivelActual = (char*) malloc(MAXSIZE);
	t_list* recursosNivel = (t_list*) malloc(sizeof(t_list));
	t_posicion* miPos = (t_posicion*) malloc(sizeof(t_posicion));
	t_posicion* posRec = (t_posicion*) malloc(sizeof(t_posicion));
	char* recursoActual = (char*) malloc(MAXSIZE);
	char* ipPlanificador = (char*) malloc(MAXSIZE);
	char* ipNivel = (char*) malloc(MAXSIZE);
	char* puertoPlanificador = (char*) malloc(MAXSIZE);
	char* puertoNivel = (char*) malloc(MAXSIZE);
	char* msjPedirRecurso = (char*) malloc(MAXSIZE);
	char* msjMovimiento = (char*) malloc(MAXSIZE);
	char* msjSimbolo = (char*) malloc(MAXSIZE);
	t_character *personaje = (t_character *) malloc(sizeof(t_character));
	char* ipOrquestador = (char*) malloc(MAXSIZE);
	char* puertoOrquestador = (char*) malloc(MAXSIZE);
	t_link_element* pNivelActual = (t_link_element*) malloc(
			sizeof(t_link_element));
	t_link_element* pRecursoActual = (t_link_element*) malloc(
			sizeof(t_link_element));
	t_dictionary* personajesTodos = (t_dictionary*) malloc(
			sizeof(t_dictionary));
	int *sockfdOrquestador = (int*) malloc(sizeof(int));
	int *sockfdNivel = (int*) malloc(sizeof(int));
	int *sockfdPlanif = (int*) malloc(sizeof(int));
	//-----------------------------------------------

	personajesTodos = getCharacters();

	personaje = (t_character*) dictionary_get(personajesTodos, "Mario");
	ipOrquestador = extraerIp(personaje->orquestador);
	puertoOrquestador = extraerPuerto(personaje->orquestador);

	// Señales ------------------------------------
//	struct sigaction action;
//	memset(&action, 0, sizeof(struct sigaction));
//	action.sa_handler = term;
//	sigaction(SIGTERM, &action, NULL);
	//---------------------------------------------

comienzoPlanDeNiveles:

	pNivelActual = ((personaje->planDeNiveles)->head);
	vidas = personaje->vidas;

	do { //Nivel

comienzoNivel:

		//Inicializar Nivel
		nivelActual = (char*) pNivelActual->data;
		recursosNivel = (t_list*) dictionary_get(personaje->obj, nivelActual);
		int cantRecAcum = 0;
		*miPos = setPosicion(1, 1);
		*posRec = setPosicion(-1, -1);
		pRecursoActual = recursosNivel->head;

		char* buff = (char*) malloc(MAXSIZE);
		char* msjPedirNivel = (char*) malloc(MAXSIZE);
		memset(msjPedirNivel, 0, sizeof(msjPedirNivel));
		memset(buff, 0, sizeof(buff));

		log_debug(log, "Esperando conexión");
		sockfdOrquestador = openSocketClient(puertoOrquestador, ipOrquestador);

		//Pedir direccion de planificador y nivel
		msjPedirNivel = string_from_format("LVL,%s", nivelActual);
		string_append(&msjPedirNivel, ENDSTRING);
		sendMessage(sockfdOrquestador, msjPedirNivel);
		log_debug(log, msjPedirNivel);
		buff = recieveMessage(sockfdOrquestador);
		log_debug(log,buff);


		close(sockfdOrquestador);

		//Conectarse al planificador y nivel
		ipPlanificador = extraerIpPlanificador(buff);
		ipNivel = extraerIpNivel(buff);
		puertoPlanificador = extraerPuertoPlanificador(buff);
		puertoNivel = extraerPuertoNivel(buff);

		sockfdNivel = openSocketClient(puertoNivel, ipNivel);
		sockfdPlanif = openSocketClient(puertoPlanificador, ipPlanificador);

		//Envio mi simbolo al nivel
		msjSimbolo = string_from_format("Simbolo:%s",personaje->simbolo);
		sendMessage(sockfdNivel, msjSimbolo);
		recieveMessage(sockfdNivel);
		log_debug(log, msjSimbolo);

		do { //Recurso
			recursoActual = string_from_format("%s", pRecursoActual->data);

			//Quedar a la espera del turno
			buff = recieveMessage(sockfdPlanif);

			if (string_equals_ignore_case(buff, TU_TURNO)) {
				//Que pasa si no es mi turno???
//				log_debug(log, "No es mi turno :(");
			}

			//si no se donde esta mi prox recurso se le pregunto al nivel
			if (posRec->posX == -1) {
				msjPedirRecurso = string_from_format("Posicion del recurso:%s",
						recursoActual);
				log_debug(log, msjPedirRecurso);

				sendMessage(sockfdNivel, msjPedirRecurso);
				buff = recieveMessage(sockfdNivel);
				*posRec = stringToPosicion(buff);

			}

			//moverme hacia el recurso
			*miPos = calcularMovimiento(miPos, posRec);
			msjMovimiento = string_from_format("Me muevo:%d,%d", miPos->posX,
					miPos->posY);
			log_debug(log, msjMovimiento);

			sendMessage(sockfdNivel, msjMovimiento);
			buff = recieveMessage(sockfdNivel);

			//Analizar respuesta
			if ((string_equals_ignore_case(buff, OK)) == 0) {
//				log_debug(log, "Error al moverme");
				//Me manda miPos => Saber que pos tengo mal
			}

			//Analizar si llegue al recurso
			if (sonPosicionesIguales(miPos, posRec)) {
				//Pedir recurso al nivel
				msjPedirRecurso = string_from_format("Dame recurso:%s",
										recursoActual);
				sendMessage(sockfdNivel, msjPedirRecurso);
				log_debug(log, DAME_RECURSO);

				*posRec = setPosicion(-1, -1);
				cantRecAcum = cantRecAcum + 1;
				pRecursoActual = pRecursoActual->next;
				buff = recieveMessage(sockfdNivel);

				if (string_equals_ignore_case(buff, RECHAZO)) {
					sendMessage(sockfdPlanif, BLOCKED);
					recieveMessage(sockfdNivel);
				}
				//Analizar si termine el nivel
				if (pRecursoActual == NULL) {
					sendMessage(sockfdNivel, TERMINE_NIVEL);
					sendMessage(sockfdPlanif, TERMINE_NIVEL);
					close(sockfdNivel);
					close(sockfdPlanif);

					log_debug(log, TERMINE_NIVEL);

					break;
				}

			}

			//Informo al planificador que termine mi turno
			sendMessage(sockfdPlanif, OK);
			log_debug(log, OK);

		} while (1); //termina recursos

		if ((pNivelActual->next) == NULL) {

			sockfdOrquestador = openSocketClient(puertoOrquestador,
					ipOrquestador);
			sendMessage(sockfdOrquestador, TERMINE_TODO);
			log_debug(log, TERMINE_TODO);

			close(sockfdOrquestador);

			break;
		}
		free(buff);
		free(msjPedirNivel);
//		pNivelActual = (pNivelActual->next);
		break;
		close(sockfdNivel);
		close(sockfdOrquestador);
		close(sockfdPlanif);

	} while (1); //termina nivel

//Libero Punteros
	free(nivelActual);
	free(recursosNivel);
	free(miPos);
	free(posRec);
	free(recursoActual);
	free(ipPlanificador);
	free(ipNivel);
	free(puertoPlanificador);
	free(puertoNivel);
	free(msjPedirRecurso);
	free(msjMovimiento);
	free(personaje);
	free(ipOrquestador);
	free(puertoOrquestador);
	free(pNivelActual);
	free(sockfdOrquestador);
	free(sockfdNivel);
	free(sockfdPlanif);
	free(msjSimbolo);

	return 0;
}

t_posicion setPosicion(int x, int y) {
	t_posicion posicionAux;
	posicionAux.posX = x;
	posicionAux.posY = y;
	return posicionAux;
}

char* extraerIpPlanificador(char* mensaje) {
	char** listaCompleta = string_split(mensaje, ",");
	char* ipPlanif = extraerIp(*listaCompleta);
	return ipPlanif;
}

char* extraerPuertoPlanificador(char* mensaje) {
	char** listaCompleta = string_split(mensaje, ",");
	char* puertoPlanif = extraerPuerto(*listaCompleta);
	return puertoPlanif;
}

char* extraerIpNivel(char* mensaje) {
	char** listaCompleta = string_split(mensaje, ",");
	char* ipNivel = extraerIp(*(listaCompleta + 1));
	return ipNivel;
}

char* extraerPuertoNivel(char* mensaje) {
	char** listaCompleta = string_split(mensaje, ",");
	char* puertoNivel = extraerPuerto(*(listaCompleta + 1));
	return puertoNivel;
}

char* extraerIp(char* direccion) {
	char** completo = string_split(direccion, ":");
	char* ip = string_from_format(*completo);
	return ip;
}

char* extraerPuerto(char* direccion) {
	char** completo = string_split(direccion, ":");
	char* puerto = string_from_format(*(completo + 1));
	return puerto;
}

t_posicion calcularMovimiento(t_posicion* miPos, t_posicion* posRec) { //Recibe la posicion actual y la del recurso buscado y devuelve la nueva posicion.
	t_posicion temp;
	temp.posX = miPos->posX;
	temp.posY = miPos->posY;
	if (miPos->posX < posRec->posX) { //estoy a la IZQUIERDA del recurso
		temp.posX++;
		return temp;
	} else { //NO estoy a la IZQUIERDA del recurso
		if (miPos->posX > posRec->posX) { //estoy a la DERECHA del recurso
			temp.posX--;
			return temp;
		} else { //estoy en el X del recurso
			if (miPos->posY < posRec->posY) { //estoy ARRIBA del recurso
				temp.posY++;
				return temp;
			} else { //NO estoy ARRIBA del recurso
				if (miPos->posY > posRec->posY) { //estoy ABAJO del recurso
					temp.posY--;
					return temp;
				}
			}
		}
	}
	return temp;
}

t_posicion stringToPosicion(char* buffer) { //Recibe el buffer en formato string y lo devuelve en t_posicion
	char** temp;
	temp = string_split(buffer, ",");
	int X = atoi(*temp);
	int Y = atoi(*(temp + 1));
	t_posicion temp2;
	temp2.posX = X;
	temp2.posY = Y;
	return temp2;
}

//bool termineNivel(t_link_element* recursosNivel) { //Recibe el array con los recursos buscados del nivel y la cantidad acumualda hasta ahora
//	int i = 0;
//	t_link_element* recursosNivelaux = recursosNivel;
//	do {
//		//cantRecAcum--;
//		i++;
//
//	} while (recursosNivelaux->head != NULL );
//	return cantRecAcum == 0;
//}

char* posicionToString(t_posicion* miPos) { //Recibe un t_posicion y devuelve un string con el formato "X,Y"
	int x;
	int y;
	x = miPos->posX;
	y = miPos->posY;
	char* mensj = string_from_format("%i,%i", x, y);
	return mensj;
}

bool sonPosicionesIguales(t_posicion* pos1, t_posicion* pos2) {
	return (pos1->posX == pos2->posX && pos1->posY == pos2->posY);
}


//void term(int signum)
//{
//	sendMessage(sockfdNivel, "Mori");
//	log_debug(log, "Me mori por SIGTERM");
//	vidas--;
//	if (vidas < 0) {
//		goto comienzoPlanDeNiveles;
//
//	}else {
//		goto comienzoNivel;
//	}
//
//}

