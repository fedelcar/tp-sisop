#include <stdlib.h>
#include "memoria.h"
#include <stdio.h>

t_list* listaParticiones;
char* Memoria;
int tamanioMax;


t_memoria crear_memoria(int tamanio) {

	//Se usa la barra  '/' para determinar espacio libre, con el mismo ID
	listaParticiones = list_create();
	tamanioMax=tamanio;
	t_particion* part = malloc(sizeof(t_particion));
	part->dato="";
	part->id='/';
	part->inicio=0;
	part->tamanio=tamanio;
	part->libre=1;
	list_add(listaParticiones, part);
	t_memoria Memoria = malloc(tamanio+1);
	int i =0;
	for (i; i <= tamanio; ++i) {
		Memoria[i]='/';
	}
	Memoria[tamanio]='\0';
	return Memoria;

}

int almacenar_particion(t_memoria segmento, char id, int tamanio, char* contenido) {

//Chequeo que no exista el id
	t_link_element* nodo2 = listaParticiones->head;
	do {
		t_particion* ptemp = nodo2->data;
		if(ptemp->id == id)
			return -1;
		else
			nodo2=nodo2->next;
	} while (nodo2!=NULL);
	//free(nodo2);

//Chequeo que la particion sea menor que el tamaño maximo de la memoria creada.
	if (tamanioMax<tamanio)
		return -1;

//Creo particion a almacenar en la lista de particiones.
	t_particion* part=malloc(sizeof(t_particion));
	part->id=id;
	part->tamanio=tamanio;
	part->dato=contenido;
	part->libre=0;

	t_link_element* nodo = listaParticiones->head;

	do {
		t_particion* ptemp = nodo->data;
		if ((ptemp->libre == 1) && (ptemp->tamanio >= tamanio))
		{
			//Actualizo lista de particiones y almaceno el contenido en memoria
			part->inicio=ptemp->inicio;
			ptemp->inicio=ptemp->inicio+tamanio;
			ptemp->tamanio=ptemp->tamanio-tamanio;
			list_add(listaParticiones,part);
			int p=part->inicio;
			int o=0;
			for (; o<tamanio; ++p, ++o) {
				segmento[p]=contenido[o];
			}
			return 1;
		}
		else
			nodo2=nodo2->next;

	} while (nodo2->next!=NULL);
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
			int i =ptemp->inicio;
			for (; i < ptemp->tamanio+ptemp->inicio; i++) {
				segmento[i]='/';
			}

//			free(ptemp);
	//		free(nodo);
			t_link_element* b = list_remove(listaParticiones,a);
			return 1;
		}
		nodo=nodo->next;
		++a;
	} while (nodo!=NULL);

	return 0;
}

void liberar_memoria(t_memoria segmento) {
	free(segmento);
}

t_list* particiones(t_memoria segmento) {
	return listaParticiones;
}
