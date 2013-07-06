#include <stdlib.h>
#include "memoria.h"
#include <stdio.h>
#include<string.h>

t_list* listaParticiones;

int tamanioMax;


t_memoria crear_memoria(int tamanio) {

	//Se usa la barra  '/' para determinar espacio libre, con el mismo ID
	listaParticiones = list_create();
	t_memoria Memoria = (t_memoria)malloc(tamanio+1);
	tamanioMax=tamanio;
	t_particion* part = (t_particion*)malloc(sizeof(t_particion));
	part->dato=&Memoria[0];
	part->id='/';
	part->inicio=0;
	part->tamanio=tamanio;
	part->libre=1;
	list_add(listaParticiones, part);

	return Memoria;
}

int almacenar_particion(t_memoria segmento, char id, int tamanio, char* contenido) {

//Chequeo que no exista el id


	t_link_element* nodo2 = listaParticiones->head;
	do {
		t_particion* ptemp2 = nodo2->data;
		if(ptemp2->id == id)
			return -1;
		else
			nodo2=nodo2->next;
	} while (nodo2!=NULL);
	//free(nodo2);

//Chequeo que la particion sea menor que el tamaño maximo de la memoria creada.
	if (tamanioMax<tamanio)
	{
		return -1;
	}

//Creo particion a almacenar en la lista de particiones.
	t_particion* part=malloc(sizeof(t_particion));
	part->id=id;
	part->tamanio=tamanio;
	part->libre=0;

	int index = 0;

	t_link_element* nodo = listaParticiones->head;

	do {
		t_particion* ptemp = nodo->data;
		if ((ptemp->libre == 1) && (ptemp->tamanio >= tamanio))
		{
			//Actualizo lista de particiones y almaceno el contenido en memoria
			part->inicio=ptemp->inicio;
			ptemp->inicio=ptemp->inicio+tamanio;
			ptemp->tamanio=ptemp->tamanio-tamanio;

			list_remove(listaParticiones, index);

			list_add_in_index(listaParticiones, index, part);

			if (ptemp->tamanio>0){
				ptemp->dato = &segmento[ptemp->inicio];
			list_add_in_index(listaParticiones, index+1, ptemp);
			}

			int p=part->inicio;
			int o;
			for (o = 0; o<tamanio; p++, o++) {
				segmento[p]=contenido[o];
			}
			int i = part->inicio;

			part->dato=&segmento[i];

			return 1;
		}
		else
			nodo=nodo->next;
			index++;

	} while (nodo!=NULL);
	free(nodo);
	free(nodo2);
	//No existe una particion de espacio suficiente.
	return 0;
}

int eliminar_particion(t_memoria segmento, char id) {
	t_link_element* nodo = listaParticiones->head;
	int a =0;
	do {
		t_particion* ptemp = nodo->data;
		if (ptemp->id == id)
		{

	//		free(nodo);
			ptemp->id= '/';
			ptemp->libre = 1;

			return 1;
		}
		nodo=nodo->next;
		++a;
	} while (nodo!=NULL);

	return 0;
}

void liberar_memoria(t_memoria segmento) {
	list_clean(listaParticiones);
	list_destroy(listaParticiones);
	free(segmento);
}

t_list* particiones(t_memoria segmento) {

	t_list* copiaLista = list_create();

	list_add_all(copiaLista, listaParticiones);

	return copiaLista;
}
