/*
 ============================================================================
 Name        : personaje.c
 Author      :
 Version     :
 Copyright   : Your copyright notice
 Description : Proceso Personaje del TP de SO
 ============================================================================
 */

#include "personaje.h"
#include <stdio.h>
#include <stdlib.h>
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


#define MAXSIZE 1024
#define COMA ","

char* extraerIpPlanificador(char* mensaje);
char* extraerIpNivel(char* mensaje);
char* extraerPuertoPlanificador(char* mensaje);
char* extraerPuertoNivel(char* mensaje);
char* extraerIp(char* direccion);
char* extraerPuerto(char* direccion);
t_posicion* stringToPosicion(char* buffer);
t_posicion* calcularMovimiento(t_posicion* miPos, t_posicion* posRec);
int termineNivel(char** recursosNivel, int cantRecAcum);
char* posicionToString (t_posicion* miPos);
t_posicion* setPosicion(int x, int y);

int main(char* character) {

	//---------- Inicializar Punteros ------------
	char* nivelActual = (char*) malloc(MAXSIZE);
	char** recursosNivel = (char**) malloc(MAXSIZE);
	t_posicion* miPos = (t_posicion*) malloc(sizeof(t_posicion));
	t_posicion* posRec = (t_posicion*) malloc(sizeof(t_posicion));
	char* recursoActual = (char*) malloc(MAXSIZE);
	char* buff = (char*) malloc(MAXSIZE);
	char* msjPedirNivel = (char*) malloc(MAXSIZE);
	char* ipPlanificador = (char*) malloc(MAXSIZE);
	char* ipNivel = (char*) malloc(MAXSIZE);
	char* puertoPlanificador = (char*) malloc(MAXSIZE);
	char* puertoNivel = (char*) malloc(MAXSIZE);
	char* msjPedirRecurso = (char*) malloc(MAXSIZE);
	char* msjMovimiento = (char*) malloc(MAXSIZE);
	t_character *personaje = (t_character *) malloc(sizeof(t_character));
	char* ipOrquestador = (char*) malloc(MAXSIZE);
	char* puertoOrquestador = (char*) malloc(MAXSIZE);
	t_link_element* pNivelActual = (t_link_element*) malloc(sizeof(t_link_element));;
	//-----------------------------------------------

	personaje = getCharacters(character);

	ipOrquestador = extraerIp(personaje->orquestador);
	puertoOrquestador = extraerPuerto(personaje->orquestador);
	pNivelActual = ((personaje->planDeNiveles)->head);



do { //Nivel

	//Inicializar Nivel
	nivelActual = (char*)pNivelActual->data;
	recursosNivel = (char**) dictionary_get(personaje->obj, nivelActual);
	int cantRecAcum = 0;
	miPos = setPosicion(0,0);
	posRec = setPosicion(-1,-1);
	recursoActual = *recursosNivel;


	int sockfdOrquestador = openSocketClient(puertoOrquestador, ipOrquestador);

	//Pedir direccion de planificador y nivel
	msjPedirNivel = string_from_format("LBL,%s", nivelActual);
	sendMessage(&sockfdOrquestador, msjPedirNivel);
	buff = recieveMessage(&sockfdOrquestador);

	close (sockfdOrquestador);

	//Conectarse al planificador y nivel
	ipPlanificador = extraerIpPlanificador(buff);
	ipNivel = extraerIpNivel(buff);
	puertoPlanificador = extraerPuertoPlanificador(buff);
	puertoNivel = extraerPuertoNivel(buff);

	int sockfdNivel = openSocketClient(puertoNivel, ipNivel);
	int sockfdPlanif = openSocketClient(puertoPlanificador, ipPlanificador);


do {//Recurso

	//Quedar a la espera del turno
	buff = recieveMessage(&sockfdPlanif);

	if ((string_equals_ignore_case(buff,"Tu turno")) == 0) {
		//Que pasa si no es mi turno???
	}

	//si no se donde esta mi prox recurso se le pregunto al nivel
	if (posRec->posX == -1) {
		msjPedirRecurso = string_from_format("Posicion del recurso: %s", recursoActual);
		sendMessage(&sockfdNivel, msjPedirRecurso);
		buff = recieveMessage(&sockfdNivel);
		posRec = stringToPosicion(buff);
	}

	//moverme hacia el recurso
	miPos = calcularMovimiento (miPos, posRec);
	msjMovimiento = string_from_format("Me muevo: %d",miPos);
	sendMessage(&sockfdNivel, msjMovimiento);
	buff = recieveMessage(&sockfdNivel);
	//Analizar respuesta
	if ((string_equals_ignore_case(buff,"ok")) == 0) {
		printf("Error al moverme.");
	}

	//Analizar si llegue al recurso
	if (miPos == posRec) {
		//Pedir recurso al nivel
		sendMessage(&sockfdNivel, "Dame recurso");
		posRec = setPosicion(-1,-1);
		cantRecAcum = cantRecAcum + 1;
		recursoActual = *(recursosNivel+cantRecAcum);
		buff = recieveMessage(&sockfdNivel);
		if (string_equals_ignore_case(buff,"Error")) {
			sendMessage(&sockfdPlanif, "Me bloquearon");
			buff = recieveMessage(&sockfdPlanif);

		}
		//Analizar si termine el nivel
		if (termineNivel (recursosNivel, cantRecAcum)) {
			sendMessage(&sockfdNivel, "Termine nivel");
			sendMessage(&sockfdPlanif, "Termine nivel");
			close (sockfdNivel);
			close (sockfdPlanif);
			break;
		}

	}

	//Informo al planificador que termine mi turno
	sendMessage(&sockfdPlanif, "Ok");

}while (1); //termina recursos

if ((pNivelActual->next) == NULL) {
	int sockfdOrquestador = openSocketClient(puertoOrquestador, ipOrquestador);
	sendMessage(&sockfdOrquestador, "Termine todo");
	close (sockfdOrquestador);
	break;
}

pNivelActual = (pNivelActual->next);

} while (1); //termina nivel

//Libero Punteros
free(nivelActual);
free(recursosNivel);
free(miPos);
free(posRec);
free(recursoActual);
free(buff);
free(msjPedirNivel);
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


return 0;
}

t_posicion* setPosicion(int x, int y) {
	t_posicion* posicionAux;
	posicionAux->posX = x;
	posicionAux->posY = y;
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
char* puerto = string_from_format(*(completo+1));
return puerto;
}

t_posicion* calcularMovimiento(t_posicion* miPos, t_posicion* posRec){ //Recibe la posicion actual y la del recurso buscado y devuelve la nueva posicion.
t_posicion* temp;
temp->posX = miPos->posX;
temp->posY = miPos->posY;
if (miPos->posX < posRec -> posX) { //estoy a la IZQUIERDA del recurso
temp->posX ++;
} else { //NO estoy a la IZQUIERDA del recurso
if (miPos->posX > posRec ->posX) { //estoy a la DERECHA del recurso
temp->posX --;
} else { //estoy en el X del recurso
if (miPos->posY < posRec->posY) { //estoy ARRIBA del recurso
temp->posY ++;
} else { //NO estoy ARRIBA del recurso
if (miPos->posY > posRec ->posY){ //estoy ABAJO del recurso
temp->posY--;
}
}
}
}
return temp;
}

t_posicion* stringToPosicion(char* buffer){ //Recibe el buffer en formato string y lo devuelve en t_posicion
char** temp;
temp= string_split(buffer,",");
int X = atoi(temp[0]);
int Y = atoi(temp[1]);
t_posicion* temp2;
temp2->posX=X;
temp2->posY=Y;
return temp2;
}

int termineNivel(char** recursosNivel, int cantRecAcum){ //Recibe el array con los recursos buscados del nivel y la cantidad acumualda hasta ahora
int i =0;
do {
cantRecAcum --;
i++;
} while (recursosNivel[i]!= NULL);
return cantRecAcum == 0;
}

char* posicionToString (t_posicion* miPos){ //Recibe un t_posicion y devuelve un string con el formato "X,Y"
int x;
int y;
x = miPos->posX;
y = miPos->posY;
char* mensj = string_from_format("%i,%i",x,y);
//char* sX,sY;
//sprintf(sX,"% i", x);
//sprintf(sY,"% d", y);
//string_append(sX,",");
//string_append(sX,sY);
return mensj;
}
