#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/string.h>
#include <nivel-gui/tad_items.h>
#include <curses.h>
#include <commons/collections/dictionary.h>
#include<uncommons/SocketsBasic.h>
#include<uncommons/fileStructures.h>
#define DOSPUNTOS ":"
#define COMA ","
#define MAXSIZE 1024
#define POSRECURSO "Posicion del recurso"
#define MOVER "Me muevo"
#define RECURSO "Dame recurso"
#define CONFIRMACION "Confirmacion"
#define RECHAZO "Rechazo"

typedef struct mensaje {
	char *nombre;
	char caracter;
	t_posicion * pos;
} mensaje_t;

mensaje_t* interpretarMensaje(char* mensaje) {

	mensaje_t* mensj = (mensaje_t*)malloc(sizeof(mensaje_t));
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
	sendMessage(&sockfd, mensaje);
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

int evaluarPosicion(t_posicion* posicion, ITEM_NIVEL * item) {

	if (item->posx == posicion->posX && item->posy == posicion->posY) {
		return 1;
	} else
	{
		return 0;
	}
}

void restarRecursos(t_posicion* posicion, ITEM_NIVEL* ListaItems, int* sockfd) {

	ITEM_NIVEL* temp = ListaItems;
	char* msjMovimiento = (char*) malloc(MAXSIZE);
	msjMovimiento = "";
	while (temp != NULL && temp->item_type == RECURSO_ITEM_TYPE) {
		if (evaluarPosicion) {
			if (temp->quantity > 0) {
				restarRecurso(ListaItems, &temp->id);
				msjMovimiento= string_from_format("%s",CONFIRMACION);

			} else {
				msjMovimiento= string_from_format("%s",RECHAZO);

			}
		}
		temp = temp->next;
		}


	if (msjMovimiento == "") {

		msjMovimiento = string_from_format("%d,%d", posicion->posX,
						posicion->posY);
		}

		sendMessage(&sockfd, msjMovimiento);
		free(msjMovimiento);
}

//Funcion Principal
void movimientoPersonaje(ITEM_NIVEL* ListaItems) {
//void main(){

	//char* mensaje= "Me muevo:20,30";

	//ITEM_NIVEL* ListaItems = (ITEM_NIVEL*) malloc(sizeof(ITEM_NIVEL));

	//CrearCaja(&ListaItems, 'H', 20, 40, 5);
	//CrearCaja(&ListaItems, 'M', 15, 8, 3);
	//CrearCaja(&ListaItems, 'F', 9, 19, 2);
	//CrearPersonaje(&ListaItems, '#', 1, 1);

	int rows = 0;
	int cols = 0;
	char simbolo;

	t_posicion* posicion=(t_posicion*)malloc(sizeof(t_posicion));
	posicion->posX = 1;
	posicion->posY = 1;
	nivel_gui_inicializar();

	nivel_gui_get_area_nivel(&rows, &cols);

	char * mensaje;
	int * sockfd = 47;

	while (1) {
		/*Voy a escuchar*/
		mensaje = recieveMessage(sockfd);
		mensaje_t * mens = interpretarMensaje(mensaje);

		if (string_equals_ignore_case(mens->nombre, POSRECURSO)) {
			mandarPosRecurso(mens->caracter, ListaItems, sockfd);
		}
		if (string_equals_ignore_case(mens->nombre, MOVER)) {
			int valor = validarPos(mens->pos, posicion, rows, cols, ListaItems,
					simbolo);
			char* msjMovimiento = (char*) malloc(MAXSIZE);

			if (valor) {

				msjMovimiento = string_from_format(CONFIRMACION);
				sendMessage(&sockfd, msjMovimiento);
				free(msjMovimiento);
			} else {

				msjMovimiento = string_from_format("%d,%d", posicion->posX,
						posicion->posY);
				sendMessage(&sockfd, msjMovimiento);
				free(msjMovimiento);
			}
		}

		if (string_equals_ignore_case(mens->nombre, SIMBOLO)) {
			simbolo = mens->caracter;
			CrearPersonaje(&ListaItems, simbolo, 1, 1);
			posicion->posX = 1;
			posicion->posY = 1;
		}
		if (string_equals_ignore_case(mens->nombre, RECURSO)) {
			restarRecursos(posicion, ListaItems, sockfd);
		}
		nivel_gui_dibujar(ListaItems);
	}
	nivel_gui_terminar();
}
