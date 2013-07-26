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
char* posicionToString(t_posicion* miPos);
t_posicion setPosicion(int x, int y);
bool sonPosicionesIguales(t_posicion* pos1, t_posicion* pos2);
void pedirPosicionRecurso(t_Nivel* nivel, t_posicion* posRec,
		t_link_element* pRecursoActual);
int conectarseAlNivelActual(t_Nivel* nivel, int sockfdOrquestador,
		int* sockfdPlanif, t_character* personaje, char* ipOrquestador);
void inicializarNivel(t_Nivel* nivel, t_posicion* miPos, t_posicion* posRec,
		t_link_element** pRecursoActual, t_character* personaje);
void muerePersonaje(t_Nivel* nivel, t_link_element** pRecursoActual,
		t_character* personaje);
void term(int signum);
void sum(int signum);

int main(int argc, char **argv) {

//---------- Inicializar Punteros ------------
	t_Nivel* nivel = (t_Nivel*) malloc(sizeof(t_Nivel));
	t_character *personaje = (t_character *) malloc(sizeof(t_character));
	int sockfdOrquestador;
	int sockfdPlanif;
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

	personaje = getCharacter(argv[1]);

	char* pathLog = (char*) malloc(MAXSIZE);
	memset(pathLog, 0, sizeof(pathLog));
	string_append(&pathLog, "/home/utnso/git/tp-20131c-tp-so-1c2013/tp/logs/");
	string_append(&pathLog, personaje->nombre);
	string_append(&pathLog, ".txt");

	log = log_create(pathLog, "Personaje", 1, LOG_LEVEL_DEBUG);

	ipOrquestador = extraerIp(personaje->orquestador);
	puertoOrquestador = extraerPuerto(personaje->orquestador);

//	Comienzo

	nivel->pNivelActual = ((personaje->planDeNiveles)->head);
	vidas = personaje->vidas;
	log_debug(log, personaje->nombre);
	log_debug(log, string_from_format("Cantidad de vidas:%d", vidas));

//	Configuro select
	int rc = 1;
	int listen_sd;

	fd_set master_set;
	fd_set working_set;

	do { //		Comienzo Plan de Niveles

		log_debug(log, "Esperando conexión");
		sockfdOrquestador = openSocketClient(puertoOrquestador, ipOrquestador);
		mensaje = string_from_format(
				"Me conecto con el Orquestador. IP:%s Puerto:%s", ipOrquestador,
				puertoOrquestador);
		log_debug(log, mensaje);

		if (!(conectarseAlNivelActual(nivel, sockfdOrquestador, &sockfdPlanif,
				personaje, ipOrquestador))) {
			break;
		}

		close(sockfdOrquestador);
		log_debug(log, "Me desconecto con el Orquestador");

		inicializarNivel(nivel, miPos, posRec, &pRecursoActual, personaje);

		vivo = TRUE;

		while (vivo) { //Comienzo Nivel

			//Quedar a la espera del turno
			buff = recieveMessage(sockfdPlanif);

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
				*miPos = stringToPosicion(buff);
				pedirPosicionRecurso(nivel, posRec, pRecursoActual);
			}

			//Analizar si llegue al recurso
			if (sonPosicionesIguales(miPos, posRec)) {
				//Pedir recurso al nivel
				mensaje = string_from_format("MOVIMIENTO:Dame recurso:%s|",
						pRecursoActual->data);
				sendMessage(nivel->sockfdNivel, mensaje);

				log_debug(log, mensaje);

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

					/**********************************************************/
					/* Copy the master fd_set over to the working fd_set.     */
					/**********************************************************/
					FD_ZERO(&master_set);
					FD_SET(nivel->sockfdNivel, &master_set);
					FD_SET(sockfdPlanif, &master_set);
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
						recieveMessage(nivel->sockfdNivel);
						log_debug(log, "Mori por DEADLOCK");
						vivo = FALSE;
						break;
					}

				} else {

					//					Informo al planificador que pedi recurso
					sendMessage(sockfdPlanif, "PEDIRRECURSO");
					log_debug(log, "PEDIRRECURSO");
				}

				*posRec = setPosicion(-1, -1);
				pRecursoActual = pRecursoActual->next;

				if (pRecursoActual == NULL ) {
					sendMessage(sockfdPlanif, TERMINE_NIVEL);
					break;
				}

			} else {

				//				Informo al planificador que termine mi turno
				if (pRecursoActual == NULL ) {
					break;
				}

				sendMessage(sockfdPlanif, OK);
				log_debug(log, "Termine mi turno");
			}

//			Analizar si termine el Nivel
			if (pRecursoActual == NULL ) {
				break;
			}

		} //termina WHILE Nivel

//		Informo que termine el nivel al Nivel y al Orquestador
		sendMessage(nivel->sockfdNivel, LIBERAR_RECURSOS);

		sockfdOrquestador = openSocketClient(puertoOrquestador, ipOrquestador);
		mensaje = string_from_format(
				"Me conecto con el Orquestador. IP:%s Puerto:%s", ipOrquestador,
				puertoOrquestador);
		log_debug(log, mensaje);

		mensaje = string_from_format("TNIVEL,%s,%s,", personaje->nombre,
				nivel->pNivelActual->data);
		sendMessage(sockfdOrquestador, mensaje);
		log_debug(log, mensaje);

		close(sockfdOrquestador);
		log_debug(log, "Me desconecto con el Orquestador");

		close(nivel->sockfdNivel);
		log_debug(log, "Me desconecto con el Nivel");

		close(sockfdPlanif);
		log_debug(log, "Me desconecto con el Planificador");

//		Analizo si sali porque mori o porque termine el nivel
		if (!vivo) {

			muerePersonaje(nivel, &pRecursoActual, personaje);

		} else {

			//			Analizo si termine el Plan de Niveles
			if ((nivel->pNivelActual->next) == NULL ) {

				sockfdOrquestador = openSocketClient(puertoOrquestador,
						ipOrquestador);
				mensaje = string_from_format(
						"Me conecto con el Orquestador. IP:%s Puerto:%s",
						ipOrquestador, puertoOrquestador);
				log_debug(log, mensaje);

				sendMessage(sockfdOrquestador, TERMINE_TODO);
				log_debug(log, TERMINE_TODO);

				close(sockfdOrquestador);
				log_debug(log, "Me desconecto con el Orquestador");

				break;

			} else {

				log_debug(log, TERMINE_NIVEL);
				nivel->pNivelActual = (nivel->pNivelActual->next);
			}
		}

	} while (1); //termina Plan de Niveles

//Libero Punteros
	free(nivel);
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

	vidas--;
	log_debug(log, string_from_format("Cantidad de vidas:%d", vidas));
	if (vidas == 0) {
		nivel->pNivelActual = ((personaje->planDeNiveles)->head);
		vidas = personaje->vidas;
		log_debug(log, "Reinicio Plan de Niveles");
	} else {
		log_debug(log, REINICIAR);
	}
}

int conectarseAlNivelActual(t_Nivel* nivel, int sockfdOrquestador,
		int* sockfdPlanif, t_character* personaje, char* ipOrquestador) {
	char* ipPlanificador = (char*) malloc(MAXSIZE);
	char* ipNivel = (char*) malloc(MAXSIZE);
	char* puertoPlanificador = (char*) malloc(MAXSIZE);
	char* puertoNivel = (char*) malloc(MAXSIZE);
	char* mensaje2 = (char*) malloc(MAXSIZE);
	char* buff2 = (char*) malloc(MAXSIZE);
	int sockNivel;

	//Pedir direccion de planificador y nivel
	mensaje2 = string_from_format("LVL,%s", nivel->pNivelActual->data);
	string_append(&mensaje2, ENDSTRING);
	sendMessage(sockfdOrquestador, mensaje2);
	log_debug(log, mensaje2);

	buff2 = recieveMessage(sockfdOrquestador);
	log_debug(log, buff2);

	//Si no encuentra el Nivel ERROR
	if (string_starts_with(buff2, "No existe el nivel pedido")) {
		mensaje2 = string_from_format("No se encontro el %s",
				nivel->pNivelActual->data);
		log_debug(log, mensaje2);

		return 0;
	} else {

		//Conectarse al planificador y nivel
		ipPlanificador = extraerIpPlanificador(buff2);
		ipNivel = extraerIpNivel(buff2);
		puertoPlanificador = extraerPuertoPlanificador(buff2);
		puertoNivel = extraerPuertoNivel(buff2);

		nivel->sockfdNivel = openSocketClient(puertoNivel, ipNivel);

		mensaje2 = string_from_format(
				"Me conecto con el Nivel. IP:%s Puerto:%s", ipNivel,
				puertoNivel);
		log_debug(log, mensaje2);

		*sockfdPlanif = openSocketClient(puertoPlanificador, ipOrquestador);
		mensaje2 = string_from_format(
				"Me conecto con el Planificador. IP:%s Puerto:%s",
				ipOrquestador, puertoPlanificador);
		log_debug(log, mensaje2);

		//Envio mi simbolo al nivel
		mensaje2 = string_from_format("START:Simbolo:%s|%s|",
				personaje->simbolo, personaje->nombre);
		if (string_equals_ignore_case(sendMessage(nivel->sockfdNivel, mensaje2),
				"BROKEN")) {
			log_debug(log, "BROKEN");
			return 0;
		}
		log_debug(log, mensaje2);

		recieveMessage(nivel->sockfdNivel);

//		Le envio mi nombre al Planificador
		sendMessage(*sockfdPlanif, string_from_format("%s,",personaje->nombre));
		log_debug(log, personaje->nombre);

	}

	free(ipPlanificador);
	free(ipNivel);
	free(puertoPlanificador);
	free(puertoNivel);
	free(mensaje2);
	free(buff2);

	return 1;
}

void inicializarNivel(t_Nivel* nivel, t_posicion* miPos, t_posicion* posRec,
		t_link_element** pRecursoActual, t_character* personaje) {

	nivel->recursosNivel = (t_list*) dictionary_get(personaje->obj,
			nivel->pNivelActual->data);
	*miPos = setPosicion(1, 1);
	*posRec = setPosicion(-1, -1);
	*pRecursoActual = nivel->recursosNivel->head;
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
	log_debug(log, buff2);

	char **split = string_split(buff2, PIPE);
	buff2 = split[0];
	*posRec = stringToPosicion(buff2);

}

void term(int signum) {
	log_debug(log, "Me mori por SIGTERM");
	vivo = FALSE;
}

void sum(int signum) {
	log_debug(log, "Gracias por la vida!");
	vidas++;
	log_debug(log, string_from_format("Cantidad de vidas:%d", vidas));
}
