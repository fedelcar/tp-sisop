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
#include <string.h>
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
#include <unistd.h>
#include <signal.h>

#define MAXSIZE 1024
#define COMA ","
#define TUTURNO "Tu turno"
#define ENDSTRING "|\0"
#define START "START"
#define TERMINE_NIVEL "Termine nivel"
#define RECHAZO "Rechazo"
#define BLOCKED "BLOCKED"
#define OK "ok"
#define DAME_RECURSO "MOVIMIENTO:Dame recurso"
#define TERMINE_TODO "Termine todo"
#define TU_TURNO "Tu Turno"
#define LIBERAR_RECURSOS "MOVIMIENTO:Liberar recursos"
#define REINICIAR "Reinicio nivel"
#define FALSE 0
#define TRUE 1
#define PIPE "|"

typedef struct {
	int sockfdNivel;
	t_link_element* pNivelActual;
	t_list* recursosNivel;
} t_Nivel;

int vivo;
int vidas;
t_log* log;

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
bool sonPosicionesIguales(t_posicion* pos1, t_posicion* pos2);
void conectarseAlNivelActual(t_Nivel* nivel, int* sockfdOrquestador,
		int** sockfdPlanif, t_character* personaje);
void inicializarNivel(t_Nivel* nivel, t_posicion* miPos, t_posicion* posRec,
		t_link_element** pRecursoActual, int* bloqueado, t_character* personaje);
void muerePersonaje(t_Nivel* nivel, t_link_element** pRecursoActual,
		t_character* personaje);
void term(int signum);
void sum(int signum);

int main(char* character) {

	log = log_create("/home/lucas/log.txt", "Personaje", 1, LOG_LEVEL_DEBUG);
//---------- Inicializar Punteros ------------
	t_Nivel* nivel = (t_Nivel*) malloc(sizeof(t_Nivel));

//	int *sockfdNivel = (int*) malloc(sizeof(int));
//	t_link_element* pNivelActual = (t_link_element*) malloc(
//			sizeof(t_link_element));
//	t_list* recursosNivel = (t_list*) malloc(sizeof(t_list));
//	char* nivelActual = (char*) malloc(MAXSIZE);

	t_dictionary* personajesTodos = (t_dictionary*) malloc(
			sizeof(t_dictionary));
	t_character *personaje = (t_character *) malloc(sizeof(t_character));
	int *sockfdOrquestador = (int*) malloc(sizeof(int));
	int sockfdPlanif = (int*) malloc(sizeof(int));
	t_link_element* pRecursoActual = (t_link_element*) malloc(
			sizeof(t_link_element));
	t_posicion* miPos = (t_posicion*) malloc(sizeof(t_posicion));
	t_posicion* posRec = (t_posicion*) malloc(sizeof(t_posicion));
	char* ipOrquestador = (char*) malloc(MAXSIZE);
	char* puertoOrquestador = (char*) malloc(MAXSIZE);
	char* buff = (char*) malloc(MAXSIZE);
	char* mensaje = (char*) malloc(MAXSIZE);
	memset(mensaje, 0, sizeof(mensaje));
	memset(buff, 0, sizeof(buff));
	int bloqueado;
	vidas = personaje->vidas;
//-----------------------------------------------

//	 Señales ------------------------------------
//	struct sigaction action;
//	memset(&action, 0, sizeof(struct sigaction));
//	action.sa_handler = term;
//	sigaction(SIGTERM, &action, NULL);
//
//	struct sigaction action2;
//	memset(&action2, 0, sizeof(struct sigaction));
//	action.sa_handler = sum;
//	sigaction(SIGUSR1, &action, NULL);

	signal(SIGTERM, term);
	signal(SIGUSR1, sum);
//	---------------------------------------------

//	cargo archivo de config
	personaje = getCharacter("/home/tp/config/personajes/personaje1.config");
	ipOrquestador = extraerIp(personaje->orquestador);
	puertoOrquestador = extraerPuerto(personaje->orquestador);

//	comienzoPlanDeNiveles
	nivel->pNivelActual = ((personaje->planDeNiveles)->head);
	vidas = personaje->vidas;

//	Configuro select
	int rc = 1;
	int listen_sd;

	fd_set master_set;
	fd_set working_set;

	do { //		Comienzo Plan de Niveles

		sockfdOrquestador = openSocketClient(puertoOrquestador, ipOrquestador);
		log_debug(log, "Esperando conexión");

		conectarseAlNivelActual(nivel, sockfdOrquestador, &sockfdPlanif,
				personaje);

		close(sockfdOrquestador);

		inicializarNivel(nivel, miPos, posRec, &pRecursoActual, &bloqueado,
				personaje);

		FD_ZERO(&master_set);
		FD_SET(nivel->sockfdNivel, &master_set);
		FD_SET(sockfdPlanif, &master_set);

////////////////////////////////////////////////////////////////////////////////////////////////
		vivo = TRUE;
		while (vivo) { //Comienzo Nivel

//			Analizo si estuve bloqueado (si lo estuve no espero el turno)
			if (bloqueado) {
				bloqueado = FALSE;
			} else {
				//Quedar a la espera del turno
				buff = recieveMessage(sockfdPlanif);
			}

			//si no se donde esta mi prox recurso se le pregunto al nivel
			if (posRec->posX == -1) {

				pedirPosicionRecurso(nivel, posRec, pRecursoActual);

			}

			//moverme hacia el recurso
			*miPos = calcularMovimiento(miPos, posRec);
			mensaje = string_from_format("MOVIMIENTO:Me muevo:%d,%d|",
					miPos->posX, miPos->posY);

			sendMessage(nivel->sockfdNivel, mensaje);
			log_debug(log, mensaje);

			buff = recieveMessage(nivel->sockfdNivel);

			//Analizar respuesta
			if (!(string_starts_with(buff, "Confirmacion"))) {
				log_debug(log, "Error al moverme");
//				Me manda miPos => Saber que pos tengo mal
			}

			//Analizar si llegue al recurso
			if (sonPosicionesIguales(miPos, posRec)) {
				//Pedir recurso al nivel
				mensaje = string_from_format("MOVIMIENTO:Dame recurso:%s|",
						pRecursoActual->data);
				sendMessage(nivel->sockfdNivel, mensaje);

				log_debug(log, mensaje);

				*posRec = setPosicion(-1, -1);
				pRecursoActual = pRecursoActual->next;

				buff = recieveMessage(nivel->sockfdNivel);
				log_debug(log, buff);

//				Analizar si quedo bloqueado
				if (string_starts_with(buff, RECHAZO)) {
					char* mensajeBloqueado = (char*) malloc(MAXSIZE);
					mensajeBloqueado = string_from_format("BLOCKED,%s",
							pRecursoActual->data);
					sendMessage(sockfdPlanif, mensajeBloqueado);
					log_debug(log, mensajeBloqueado);
					free(mensajeBloqueado);

					bloqueado = TRUE;

					/**********************************************************/
					/* Copy the master fd_set over to the working fd_set.     */
					/**********************************************************/
					memcpy(&working_set, &master_set, sizeof(master_set));

					rc = select(FD_SETSIZE, &working_set, NULL, NULL, NULL );

					/**********************************************************/
					/* Check to see if the select call failed.                */
					/**********************************************************/
					if (rc < 0) {
						perror("select() failed");
						vivo = FALSE;
						break;
					}

					if (FD_ISSET(nivel->sockfdNivel, &working_set)) {
						log_debug(log, "Mori por DEADLOCK");
						vivo = FALSE;
					}

				} else {

					//					Informo al planificador que pedi recurso
					sendMessage(sockfdPlanif, "PEDIRRECURSO");
					log_debug(log, "PEDIRRECURSO");
				}

			} else {

				//				Informo al planificador que termine mi turno
				sendMessage(sockfdPlanif, OK);
				log_debug(log, "Termine mi turno");
			}

//			Analizar si termine el Nivel
			if (pRecursoActual == NULL ) {
				break;
			}

		} //termina WHILE recursos

//		Informo que termine el nivel al Planif
		sendMessage(nivel->sockfdNivel, LIBERAR_RECURSOS);
		close(nivel->sockfdNivel);

		sendMessage(sockfdPlanif, TERMINE_NIVEL);
		close(sockfdPlanif);

//		Analizo si sali porque mori o porque termine el nivel
		if (!vivo) {

			muerePersonaje(nivel, &pRecursoActual, personaje);

		} else {

			//			Analizo si termine el Plan de Niveles
			if ((nivel->pNivelActual->next) == NULL ) {

				sockfdOrquestador = openSocketClient(puertoOrquestador,
						ipOrquestador);
				sendMessage(sockfdOrquestador, TERMINE_TODO);
				close(sockfdOrquestador);

				log_debug(log, TERMINE_TODO);
				break;

			} else {

				log_debug(log, TERMINE_NIVEL);
				nivel->pNivelActual = (nivel->pNivelActual->next);
			}
		}

	} while (1); //termina nivel

//Libero Punteros
	free(nivel);
//	free(sockfdNivel);
//	free(pNivelActual);
//	free(recursosNivel);
//	free(nivelActual);
	free(personajesTodos);
	free(personaje);
	free(pRecursoActual);
	free(miPos);
	free(posRec);
	free(ipOrquestador);
	free(puertoOrquestador);
	free(buff);
	free(mensaje);
	free(log);

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
			return temp; //	nivelActual = (char*) pNivelActual->data;
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

void muerePersonaje(t_Nivel* nivel, t_link_element** pRecursoActual,
		t_character* personaje) {
	if ((vidas--) == 0) {
		nivel->pNivelActual = ((personaje->planDeNiveles)->head);
		vidas = personaje->vidas;
		log_debug(log, "Reinicio Plan de Niveles");
	} else {
		log_debug(log, REINICIAR);
	}
}

void conectarseAlNivelActual(t_Nivel* nivel, int* sockfdOrquestador,
		int** sockfdPlanif, t_character* personaje) {
	char* ipPlanificador = (char*) malloc(MAXSIZE);
	char* ipNivel = (char*) malloc(MAXSIZE);
	char* puertoPlanificador = (char*) malloc(MAXSIZE);
	char* puertoNivel = (char*) malloc(MAXSIZE);
	char* mensaje2 = (char*) malloc(MAXSIZE);
	char* buff2 = (char*) malloc(MAXSIZE);

	//Pedir direccion de planificador y nivel
	mensaje2 = string_from_format("LVL,%s", nivel->pNivelActual->data);
	string_append(&mensaje2, ENDSTRING);
	sendMessage(sockfdOrquestador, mensaje2);
	log_debug(log, mensaje2);

	buff2 = recieveMessage(sockfdOrquestador);
	log_debug(log, buff2);

	//Conectarse al planificador y nivel
	ipPlanificador = extraerIpPlanificador(buff2);
	ipNivel = extraerIpNivel(buff2);
	puertoPlanificador = extraerPuertoPlanificador(buff2);
	puertoNivel = extraerPuertoNivel(buff2);

	nivel->sockfdNivel = openSocketClient(puertoNivel, ipNivel);
	*sockfdPlanif = openSocketClient(puertoPlanificador, ipPlanificador);
	recieveMessage(sockfdPlanif);
	sendMessage(sockfdPlanif, personaje->nombre);
	log_debug(log, personaje->nombre);

	//Envio mi simbolo al nivel
	mensaje2 = string_from_format("START:Simbolo:%s|%s|", personaje->simbolo, personaje->nombre);
	sendMessage(nivel->sockfdNivel, mensaje2);
	log_debug(log, mensaje2);

	recieveMessage(nivel->sockfdNivel);

	free(ipPlanificador);
	free(ipNivel);
	free(puertoPlanificador);
	free(puertoNivel);
	free(mensaje2);
	free(buff2);
}

void inicializarNivel(t_Nivel* nivel, t_posicion* miPos, t_posicion* posRec,
		t_link_element** pRecursoActual, int* bloqueado, t_character* personaje) {
	char* nivelActual = (char*) malloc(MAXSIZE);

	nivelActual = (char*) nivel->pNivelActual->data;
	nivel->recursosNivel = (t_list*) dictionary_get(personaje->obj,
			nivelActual);
	*miPos = setPosicion(1, 1);
	*posRec = setPosicion(-1, -1);
	*pRecursoActual = nivel->recursosNivel->head;
	*bloqueado = FALSE;

	free(nivelActual);
}

void pedirPosicionRecurso(t_Nivel* nivel, t_posicion* posRec,
		t_link_element* pRecursoActual) {
	char* recursoActual = (char*) malloc(MAXSIZE);
	char* mensaje2 = (char*) malloc(MAXSIZE);
	char* buff2 = (char*) malloc(MAXSIZE);

	recursoActual = string_from_format("%s", pRecursoActual->data);
	mensaje2 = string_from_format("MOVIMIENTO:Posicion del recurso:%s|",
			recursoActual);

	sendMessage(nivel->sockfdNivel, mensaje2);
	log_debug(log, mensaje2);

	buff2 = recieveMessage(nivel->sockfdNivel);
	char **split = string_split(buff2, PIPE);
	buff2 = split[0];
	*posRec = stringToPosicion(buff2);

}

void term(int signum) {
	log_debug(log, "Me mori por SIGTERM");
	vivo = FALSE;
}

void sum(int signum) {
	vidas++;
}
