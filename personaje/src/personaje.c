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
#include <x86_64-linux-gnu/sys/types.h>
#include <x86_64-linux-gnu/sys/socket.h>


#define PATHPERSONAJE "home/.."
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

int main(char* character) {

	t_character *personaje;
	personaje = getCharacters(character);

	char* ipOrquestador = extraerIp(personaje->orquestador);
	char* puertoOrquestador = extraerPuerto(personaje->orquestador);
	t_link_element * pNivelActual = ((personaje->planDeNiveles)->head);


do {

	char* nivelActual = (char*)pNivelActual->data;
	int sockfdOrquestador = openSocketClient(puertoOrquestador, ipOrquestador);
	char* buff;
	char** recursosNivel = (char**) dictionary_get(personaje->obj, nivelActual);
	//Pedir direccion de planificador y nivel
	char* msjPedirNivel = string_from_format("LBL,%s", nivelActual);
	sendMessage(&sockfdOrquestador, msjPedirNivel);
	buff = recieveMessage(&sockfdOrquestador);

	close (sockfdOrquestador);

	//Conectarse al planificador y nivel
	char* ipPlanificador = extraerIpPlanificador(buff);
	char* ipNivel = extraerIpNivel(buff);
	char* puertoPlanificador = extraerPuertoPlanificador(buff);
	char* puertoNivel = extraerPuertoNivel(buff);

	int sockfdNivel = openSocketClient(puertoNivel, ipNivel);
	int sockfdPlanif = openSocketClient(puertoPlanificador, ipPlanificador);

	int cantRecAcum;
	t_posicion* miPos;
	t_posicion* posRec;
	char* recursoActual = *recursosNivel;
	//inicializarVariables (&miPos, &posRec, &recAcum);

do {

	//Quedar a la espera del turno
	buff = recieveMessage(&sockfdPlanif);

	if ((string_equals_ignore_case(buff,"Tu turno")) == 0) {
		//Que pasa si no es mi turno???
	}

	//si no se donde esta mi prox recurso se le pregunto al nivel
	if (posRec == NULL) {
		char* msjPedirRecurso = string_from_format("Posicion del recurso: %s", recursoActual);
		sendMessage(&sockfdNivel, msjPedirRecurso);
		buff = recieveMessage(&sockfdNivel);
		posRec = stringToPosicion(buff);
	}

	//moverme hacia el recurso
	miPos = calcularMovimiento (miPos, posRec);
	char* msjMovimiento = string_from_format("Me muevo: %d",miPos);
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
		posRec = NULL;
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

}while (1);
//termina recursos

if ((pNivelActual->next) == NULL) {
	sendMessage(&sockfdOrquestador, "Termine todo");
	break;
}

pNivelActual = (pNivelActual->next);

} while (1);
//termina nivel
return 0;
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
char* temp;
temp= string_split(buffer,COMA);
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
int x,y;
x = miPos->posX;
y = miPos->posY;
char* sX,sY;
sprintf(sX,"% i", x);
sprintf(sY,"% d", y);
string_append(sX,",");
string_append(sX,sY);
return sX;
}
/*
 t_log *log;
 log = log_create("/home/tp/log.txt","personaje.c",1,LOG_LEVEL_DEBUG);

 log_debug(log,"Prueba de log de Debug");

 t_dictionary *personaje = getCharacters();



 t_config *config_Personaje = config_create("/home/tp/config/mario.txt");
 char *nombre_Personaje = config_get_string_value(config_Personaje, "Nombre");
 char *simbolo_Personaje = config_get_string_value(config_Personaje, "Simbolo");
 char **niveles_Personaje = config_get_array_value(config_Personaje,"planDeNiveles");
 log_debug(log,"%s",niveles_Personaje[0]);

 int i =0;
 int cantNiveles = 0;
 do {
 cantNiveles ++;
 i++;
 } while (niveles_Personaje[i] != NULL);
 log_debug(log,"%d",cantNiveles);

 t_list *lista_Objetivos = list_create();

 for (i = 0; i < cantNiveles; i++) {
 printf("obj[%s]\n",niveles_Personaje[i]);
 //char* nivel = ("obj[%s]",niveles_Personaje[i]);
 //log_debug(logDebug,nivel);
 //char** objetivos_nivel = config_get_array_value(config_Personaje,nivel);
 //list_add(lista_Objetivos, objetivos_nivel);
 }


 log_debug(log, "Se finalizo el programa");
 return EXIT_SUCCESS;
 }


 //t_log *initiate_log (int Level)
 //{
 //	t_log *logTemp;
 //	//logTemp = log_create("uncommons/log.txt","personaje.c",1,Level);
 //	logTemp = log_create("/home/tp/log.txt","personaje.c",1,Level);
 //	return logTemp;
 //}
 /*/
