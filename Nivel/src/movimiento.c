#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/string.h>
#include <curses.h>
#include <commons/collections/dictionary.h>
#include <uncommons/SocketsBasic.h>
#include "nivelBase.h"

#define DOSPUNTOS ":"
#define COMA ","
#define MAXSIZE 1024
#define POSRECURSO "Posicion del recurso"
#define MOVER "Me muevo"
#define RECURSO "Dame recurso"
#define CONFIRMACION "Confirmacion"
#define RECHAZO "Rechazo"
#define EMPTYSTRING "EMPTY"

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

	if (Npos->posX > cols) {
	} else {
		if (Npos->posY > rows) {
		} else {
			Apos->posX = Npos->posX;
			Apos->posY = Npos->posY;
			MoverPersonaje(ListaItems, simbolo, Npos->posX, Npos->posY);
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
		char recurso, recursos_otorgados* recursos) {

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
void movimientoPersonaje(resource_struct* resources) {
//void main(){

//char* mensaje= "Me muevo:20,30";

	recursos_otorgados * recursosAt=(recursos_otorgados*)malloc(sizeof(recursos_otorgados));
	recursosAt->F = 0;
	recursosAt->H = 0;
	recursosAt->M = 0;

	ITEM_NIVEL* listaItems = (ITEM_NIVEL*) malloc(sizeof(ITEM_NIVEL));

	listaItems = resources->listaItems;

//CrearCaja(&ListaItems, 'H', 20, 40, 5);
//CrearCaja(&ListaItems, 'M', 15, 8, 3);
//CrearCaja(&ListaItems, 'F', 9, 19, 2);
//CrearPersonaje(&ListaItems, '#', 1, 1);

	int *rows = (int*) malloc(sizeof(int));
	int *cols = (int*) malloc(sizeof(int));
	rows = (int*) 10;
	cols = (int*) 10;
	char simbolo;

	t_posicion* posicion = (t_posicion*) malloc(sizeof(t_posicion));
	posicion->posX = 0;
	posicion->posY = 0;

	//nivel_gui_get_area_nivel(&rows, &cols);

	char * mensaje = (char*) malloc(MAXSIZE);
	int *sockfd = resources->fd;

	while (1) {
		/*Voy a escuchar*/
		mensaje = recieveMessage(sockfd);

		if(string_equals_ignore_case(mensaje, "Termine nivel")){
			restaurarRecursos(recursosAt, listaItems);
			break;
		}

		mensaje_t * mens = interpretarMensaje(mensaje);

		if (string_equals_ignore_case(mens->nombre, POSRECURSO)) {
			mandarPosRecurso(mens->caracter, listaItems, sockfd);
		}
		if (string_equals_ignore_case(mens->nombre, MOVER)) {

			int valor = validarPos(mens->pos, posicion, rows, cols, listaItems,
					simbolo);
			char* msjMovimiento = (char*) malloc(MAXSIZE);

			if (valor == 1) {

				msjMovimiento = string_from_format(CONFIRMACION);
				sendMessage(sockfd, msjMovimiento);
				free(msjMovimiento);
			} else {

				msjMovimiento = string_from_format("%d,%d", posicion->posX,
						posicion->posY);
				sendMessage(sockfd, msjMovimiento);
				free(msjMovimiento);
			}
		}

		if (string_equals_ignore_case(mens->nombre, SIMBOLO)) {
			simbolo = mens->caracter;
			CrearPersonaje(&listaItems, simbolo, 1, 1);
			posicion->posX = 1;
			posicion->posY = 1;
		}
		if (string_equals_ignore_case(mens->nombre, RECURSO)) {
			restarRecursos(posicion, listaItems, sockfd, mens->caracter, recursosAt);
		}
	}

	BorrarItem(&listaItems, simbolo);

}
