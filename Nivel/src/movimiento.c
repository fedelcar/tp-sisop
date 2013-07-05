#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <commons/string.h>
#include <curses.h>
#include <commons/collections/dictionary.h>
#include <uncommons/SocketsBasic.h>
#include "nivelBase.h"
#include <uncommons/SocketsCliente.h>

#define BROKEN "BROKEN"
#define DOSPUNTOS ":"
#define COMA ","
#define MAXSIZE 1024
#define POSRECURSO "Posicion del recurso"
#define MOVER "Me muevo"
#define RECURSO "Dame recurso"
#define CONFIRMACION "Confirmacion"
#define RECHAZO "Rechazo"
#define EMPTYSTRING "EMPTY"
#define OKEY "ok"
#define FREERESC "FREERESC,"
#define PIPE "|"

char* endingString(recursos_otorgados * recursosAt, char* nivel);

mensaje_t* interpretarMensaje(char* mensaje) {

	mensaje_t* mensj = (mensaje_t*) malloc(sizeof(mensaje_t));
	mensj->pos = (t_posicion*) malloc(sizeof(t_posicion));
	char** temp;
	temp = string_split(mensaje, DOSPUNTOS);
	mensj->nombre = temp[0];
	char* P = temp[1];
	char** temp2;
	temp2 = string_split(temp[1], COMA);

	if (temp2[1] != NULL ) {
		mensj->pos->posX = atoi(temp2[0]);
		mensj->pos->posY = atoi(temp2[1]);
	} else {
		mensj->caracter = P[0];
	}
	return mensj;
}

void mandarPosRecurso(char recurso, ITEM_NIVEL* temp, int* sockfd) {

	while (temp != NULL && temp->id != recurso) {
		temp = temp->next;
	}

	char *mensaje = string_from_format("%d,%d", temp->posx, temp->posy);
	sendMessage(sockfd, mensaje);
}

int validarPos(t_posicion * Npos, t_posicion * Apos, int rows, int cols,
		ITEM_NIVEL* ListaItems, char simbolo) {
	printf("entro a la funcion");
	if (Npos->posX > cols) {
	} else {
		if (Npos->posY > rows) {
		} else {
			Apos->posX = Npos->posX;
			Apos->posY = Npos->posY;
			ITEM_NIVEL *personaje = (ITEM_NIVEL*) malloc(sizeof(ITEM_NIVEL));
			personaje->id = simbolo;
			personaje->posx = Npos->posX;
			personaje->posy = Npos->posY;
			personaje->item_type = PERSONAJE_ITEM_TYPE;
			MoverPersonaje(ListaItems, simbolo, Npos->posX, Npos->posY);
			free(personaje);
			return 1;
		}
	}
	return 0;
}

int evaluarPosicion(t_posicion* posicion, ITEM_NIVEL *item) {

	if (item->posx == posicion->posX && item->posy == posicion->posY) {
		return 1;
	} else {
		return 0;
	}
}

void pasarLista(recursos_otorgados*recursos, char recurso){
	switch (recurso){
	case 'H':
		recursos->H++;
		break;
	case 'F':
		recursos->F++;
		break;
	case 'M':
		recursos->M++;
		break;
	}
}

void restarRecursos(t_posicion* posicion, ITEM_NIVEL* listaItems, int* sockfd,
		char recurso, recursos_otorgados* recursos, resource_struct* resources) {

	char* msjMovimiento = (char*) malloc(MAXSIZE);
	msjMovimiento = EMPTYSTRING;
	while (listaItems != NULL && recurso != listaItems->id) {
		listaItems = listaItems->next;
	}

	if (evaluarPosicion(posicion, listaItems) == 1) {
		if (listaItems->quantity > 0) {
			restarRecurso(listaItems, listaItems->id);
			pasarLista(recursos, recurso);
			msjMovimiento = string_from_format("%s", CONFIRMACION);

		} else {
			msjMovimiento = string_from_format("%s", RECHAZO);
			resources->recursoBloqueado = recurso;
		}
	}

	if (string_equals_ignore_case(msjMovimiento, "EMPTY")) {

		msjMovimiento = string_from_format("%d,%d", posicion->posX,
				posicion->posY);
	}

	sendMessage(sockfd, msjMovimiento);
	free(msjMovimiento);
}

ITEM_NIVEL* buscarRecurso(ITEM_NIVEL* listaItems, char recurso){
	while (listaItems!=NULL && listaItems->id !=recurso){
		listaItems =listaItems->next;
	}
	return listaItems;
}

void restaurarRecursos(recursos_otorgados* recursos, ITEM_NIVEL* listaItems){
	ITEM_NIVEL* temp;

	temp = buscarRecurso(listaItems, 'H');
	temp->quantity= temp->quantity + recursos->H;

	temp = buscarRecurso(listaItems, 'F');
	temp->quantity= temp->quantity + recursos->F;

	temp = buscarRecurso(listaItems, 'M');
	temp->quantity= temp->quantity + recursos->M;

}


//Funcion Principal
void movimientoPersonaje(resource_struct* resources, int rows, int cols, char* mensaje, fd_set *master_set, int fileDescriptorPj, int socketOrquestador) {

	ITEM_NIVEL* listaItems = resources->listaItems;

	int *sockfd = resources->fd;

	char** splitMessage = (char*) malloc(MAXSIZE);

		/*Voy a escuchar*/

		if(string_starts_with(mensaje, BROKEN)){
			return;
		}

		splitMessage = string_split(mensaje, PIPE);

		mensaje = splitMessage[0];

		resources->recursoBloqueado = '0';

		if(string_starts_with(mensaje, "Liberar recursos")){
			int fileDescriptor;
			char** socket = (char*) malloc(MAXSIZE);
			socket = string_split(resources->level_config->orquestador, DOSPUNTOS);
			fileDescriptor = openSocketClient(socket[1], socket[0]);
			char* mensaje = endingString(resources->recursosAt, resources->level_config->nombre);
			sendMessage(fileDescriptor, mensaje);
			mensaje = recieveMessage(fileDescriptor);
			if (string_equals_ignore_case(mensaje, OKEY)) {
				restaurarRecursos(resources->recursosAt, listaItems);
				nivel_gui_dibujar(listaItems);
			}
			BorrarItem(&listaItems, resources->simbolo);
			FD_CLR(fileDescriptorPj, master_set);
			free(mensaje);
			close(fileDescriptor);
			free(socket);
			return;
		}

		mensaje_t * mens = interpretarMensaje(mensaje);

		if (string_equals_ignore_case(mens->nombre, POSRECURSO)) {
			mandarPosRecurso(mens->caracter, listaItems, sockfd);
		}
		if (string_equals_ignore_case(mens->nombre, MOVER)) {
			printf("validar pos");
			int valor = validarPos(mens->pos, resources->posicion, rows, cols, listaItems,
					resources->simbolo);
			char* msjMovimiento = (char*) malloc(MAXSIZE);

			if (valor == 1) {

				msjMovimiento = string_from_format(CONFIRMACION);
				sendMessage(sockfd, msjMovimiento);
				free(msjMovimiento);
			} else {

				msjMovimiento = string_from_format("%d,%d", resources->posicion->posX,
						resources->posicion->posY);
				sendMessage(sockfd, msjMovimiento);
				free(msjMovimiento);
			}
		}

		if (string_equals_ignore_case(mens->nombre, RECURSO)) {
			restarRecursos(resources->posicion, listaItems, sockfd, mens->caracter, resources->recursosAt, resources);
		}
		nivel_gui_dibujar(listaItems);

		free(splitMessage);

}


char* endingString(recursos_otorgados *recursosAt, char* nivel){

	char* lastString = (char*) malloc(MAXSIZE);

	char* nivelLocal = (char*) malloc(MAXSIZE);

	strcpy(nivelLocal, nivel);

	string_append(&lastString, FREERESC);

	string_append(&lastString, nivel);

	string_append(&lastString, string_from_format(",%d,%d,%d,", recursosAt->F, recursosAt->M, recursosAt->H));

	return lastString;
}
