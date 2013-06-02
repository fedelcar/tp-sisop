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

typedef struct mensaje{
	char *nombre;
	char *caracter;
	t_posicion * pos;
} mensaje_t;

mensaje_t* interpretarMensaje(char* mensaje){

	char* temp;
	temp= strtok(mensaje,DOSPUNTOS);
	char* nombre = temp[0];
	char Y = temp[1];
	mensaje_t* temp2;
	temp2->nombre=nombre;
	if (strlen(Y)>1)
	{
		temp2->pos= Y;
	}
	else
	{
	temp2->caracter=Y;
	}
	return temp2;
	}

void mandarPosRecurso(char* recurso, ITEM_NIVEL* ListaItems, int* sockfd){
	//Aca deberia ser algo asi
	  ITEM_NIVEL* temp = ListaItems;

	  while (temp != NULL && temp->id!=recurso) {
	  temp = temp->next;
	  }

	  char *mensaje = string_from_format("%d,%d",temp->posx, temp->posy);
	  sendMessage(&sockfd, mensaje);
}

void validarPos(t_posicion * Npos,t_posicion * Apos, int rows, int cols , ITEM_NIVEL* ListaItems, char simbolo){

	if (Npos->posX >cols){
	}
	else
	{
		if (Npos->posY>rows){
		}
		else
		{
			Apos->posX = Npos->posX;
			Apos->posY = Npos->posY;
			MoverPersonaje(ListaItems, simbolo, Npos->posX,Npos->posY);
		}
	}
}

void restarRecursos(t_posicion* posicion,ITEM_NIVEL* ListaItems, int* sockfd){
	  ITEM_NIVEL* temp = ListaItems;
		  while (temp != NULL) {
		  if (temp->item_type== RECURSO_ITEM_TYPE){
			  if (temp->posx == posicion->posX){
				  if (temp->posy == posicion->posY){
					  if (temp->quantity>0){
						  restarRecurso(ListaItems, &temp->id);
						  sendMessage(&sockfd, "Confirmacion");
					  }
					  sendMessage(&sockfd, "Rechazo");
				  }
			  }
		  }
		  temp = temp->next;
}
}

//Funcion Principal
void movimientoPersonaje() {

	ITEM_NIVEL* ListaItems = NULL;

	int rows, cols;
	char simbolo;


	t_posicion* posicion;
	posicion->posX = 1;
	posicion->posY = 1;



	nivel_gui_get_area_nivel(&rows, &cols);

	char * mensaje;
	int * sockfd = 47;

	while (1){
		/*Voy a escuchar*/
		mensaje = recieveMessage(sockfd);
		mensaje_t * mens = interpretarMensaje(mensaje);

		if (mens->nombre=="Posicion del recurso"){
			mandarPosRecurso(mens->caracter, ListaItems, sockfd);
		}
		if (mens->nombre =="Me muevo" ){
			validarPos(mens->pos, posicion, rows, cols, ListaItems, simbolo);
		}
		if (mens->nombre=="Simbolo"){
			simbolo = mens->caracter;
			CrearPersonaje(&ListaItems, simbolo, 1, 1);
			posicion->posX= 1;
			posicion->posY = 1;
		}
		if (mens->nombre == "Dame recurso"){
			restarRecursos(posicion, ListaItems, sockfd);
		}
	nivel_gui_dibujar(ListaItems);
	}

	nivel_gui_terminar();
}

