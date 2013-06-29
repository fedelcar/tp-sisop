#include <stdlib.h>
#include "memoria.h"
#include <stdio.h>

t_list* listaParticiones;
char* Pparticiones;
int tamanioMax;

int main(){

char* asd = crear_memoria(1024);

}

t_memoria crear_memoria(int tamanio) {

	//Se usa la barra  '/' para determinar espacio libre, con el mismo ID
	listaParticiones = list_create();
	tamanioMax=tamanio;
	t_particion* part;
	part->dato="/";
	part->id="/";
	part->inicio=0;
	part->tamanio=tamanio;
	part->libre=1;
	list_add(listaParticiones, part);
	free(part);
	t_memoria Pparticiones = malloc(tamanio * sizeof(t_particion));
	int i =0;
	for (i; i < tamanio; ++i) {
		Pparticiones[i]="/";
	}
	return Pparticiones;

}

int almacenar_particion(t_memoria segmento, char id, int tamanio, char* contenido) {

//Chequeo que no exista el id
	t_link_element* nodo2 = listaParticiones->head;
	do {
		t_particion* ptemp = nodo2->data;
		if(ptemp->id == id)
			return -1;
	} while (nodo2->next!=NULL);
	free(nodo2);

//Chequeo que la particion sea menor que el tamaño maximo de la memoria creada.
	if (tamanioMax<tamanio)
		return -1;

//Creo particion a almacenar en la lista de particiones.
	t_particion part;
	part.id=id;
	part.tamanio=tamanio;
	part.dato=contenido;
	part.libre=0;
	t_link_element* nodo = listaParticiones->head;

	do {
		t_particion* partTemp = nodo;
		if (partTemp->libre && partTemp->tamanio >= tamanio)
		{
			//Actualizo lista de particiones y almaceno el contenido en memoria
			part.inicio=partTemp->inicio;
			partTemp->inicio=partTemp->inicio+tamanio;
			list_add(listaParticiones,&part);
			Pparticiones[part.inicio]=contenido;
			return 1;
		}
		else
			nodo=nodo->next;

	} while (nodo->next!=NULL);
	free(nodo);

	//No existe una particion de espacio suficiente.
	return 0;
}

int eliminar_particion(t_memoria segmento, char id) {
	t_link_element* nodo = listaParticiones->head;
	t_link_element* nodoAnt = listaParticiones->head;
	do {
		t_particion* ptemp = nodo;
		if (ptemp->id == id)
		{
			int i =0;
			for (i; i < ptemp->tamanio; i++) {
				segmento[ptemp->inicio+i]="";
			}
			nodoAnt->next=nodo->next;
			free(ptemp);
			free(nodo);
			return 1;
		}
		nodoAnt=nodoAnt->next;
	} while (nodo->next!=NULL);

	return 0;
}

void liberar_memoria(t_memoria segmento) {
	free(Pparticiones);
	free(listaParticiones);
}

t_list* particiones(t_memoria segmento) {
	return listaParticiones;
}
