#include <stdio.h>
#include <stdlib.h>
#include "commons/collections/queue.h"
#include "commons/collections/list.h"
#include "commons/collections/dictionary.h"
#include "nivelBase.h"

//Se considera que 1 es true y 0 es false

int algunRecursoVacio(ITEM_NIVEL *temp) {
	int bol = 0;
//Me fijo si algun recurso esta en 0
	while (temp != NULL ) {
		if (temp->item_type == RECURSO_ITEM_TYPE) {

			if (temp->quantity == 0) {
				bol = 1;
			}
		}
		temp = temp->next;
	}
	return bol;
}

int miPersonajeNesecita(datos_personaje* datos, t_dictionary * recursos) {
	int bol2 = 0;

	if (dictionary_has_key(recursos, datos->recurso)) {

		if (dictionary_get(recursos, datos->recurso) == 0) {
			bol2 = 1;
		}
	}
	return bol2;
}

int tieneRecurso(datos_personaje*pers2, datos_personaje*pers) {
	int bol3 = 0;

	if (dictionary_get(pers2->recursosAt, pers->recurso) > 0) {
		bol3 = 1;
	}

	return bol3;
}

int estaBloqueado(datos_personaje*perso) {
	int bol5 = 0;
	if (perso->recurso[0] != '0') {
		bol5 = 1;
	}
	return bol5;
}

int* loTieneOtro(t_list *listaPersonajes, datos_personaje*perss) {
	int* bol4 = (int*) malloc(sizeof(int));
	bol4 = 1;
	int y = 0;
	for (y = 0; y < list_size(listaPersonajes); y++) {
		datos_personaje * pers2 = list_get(listaPersonajes, y);
		if (perss->id != pers2->id) {
			if (tieneRecurso(pers2, perss) == 1) {
				if (estaBloqueado(pers2) == 1) {
					//devuelve true

				} else {
					//devuelve false
					bol4 = 0;
				}
			}
		}
	}
	return bol4;
}

t_list* detectionAlgorithm(ITEM_NIVEL* listaItems, t_list* listaPersonajes) {

	t_dictionary* recursos = dictionary_create();
	ITEM_NIVEL* aux;
	aux = listaItems;
	while (aux != NULL ) {
		if (aux->item_type == RECURSO_ITEM_TYPE) {
			dictionary_put(recursos, &(aux->id), aux->quantity);
		}
		aux = aux->next;
	}

	t_list * listaResultados = list_create();

	if (list_size(listaPersonajes) > 1 && algunRecursoVacio(listaItems) == 1) {

		int valor;
		int i = 0;

		for (i = 0; i < list_size(listaPersonajes); i++) {
			//hacer otro for para que cada personaje recorra toda la lista de personajes

			datos_personaje * pers = list_get(listaPersonajes, i);

			if (estaBloqueado(pers)==1) {

				valor = loTieneOtro(listaPersonajes, pers);

				if (valor == 1) {

					list_add(listaResultados, pers);
				} else {

					//list_add(listaResultados, valor);
				}
			}
		}
	}

	return listaResultados;
}
