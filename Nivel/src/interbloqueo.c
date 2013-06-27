#include <stdio.h>
#include <stdlib.h>
#include "commons/collections/queue.h"
#include "commons/collections/list.h"
#include "nivelBase.h"


//Se considera que 1 es true y 0 es false

int algunRecursoVacio(ITEM_NIVEL *temp, int *he, int *fe, int *me) {
    int bol = 0;
//Me fijo si algun recurso esta en 0
    while (temp != NULL ) {
        if (temp->item_type == RECURSO_ITEM_TYPE){

        if (temp->quantity == 0) {
            bol = 1;

            if(temp->id == 'H'){

                *he = 0;

            }
            else if(temp->id == 'F'){

                *fe = 0;

            }
            else if(temp->id == 'M'){

               *me = 0;

            }
        }
        }
        temp = temp->next;
    }
    return bol;
}

int miPersonajeNesecita(datos_personaje* datos, int hf, int ff, int mf) {
    int bol2 = 0;

    if (hf == 0) {
        if (datos->recurso[0] == 'H') {
            bol2 = 1;
        }
    }
     if (ff == 0) {
        if (datos->recurso[0] == 'F') {
            bol2 = 1;

        }
     }
     if (mf==0){

        if (datos->recurso[0] == 'M') {
            bol2 = 1;
        }
    }

    return bol2;
}

int tieneRecurso(datos_personaje*pers2, datos_personaje*pers) {
    int bol3 = 0;

    switch (pers->recurso[0]) {
    case 'H':
        if (pers2->H > 0) {
            bol3 = 1;
        }
        break;
    case 'F':
        if (pers2->F > 0) {
            bol3 = 1;
        }
        break;
    case 'M':
        if (pers2->M > 0) {
            bol3 = 1;
        }
        break;
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
    int* bol4 =(int*)malloc(sizeof(int));
    bol4 = 1;
    int y = 0;
    for (y = 0; y < list_size(listaPersonajes); y++) {
        datos_personaje * pers2 = list_get(listaPersonajes, y);
        if (perss->id != pers2->id) {
            if (tieneRecurso(pers2, perss) == 1) {
            	if (estaBloqueado(pers2)==1){
            		//devuelve true

            	}
            	else
            	{
            		//devuelve false
            		bol4 = 0;
            	}
            }
        }
    }
    return bol4;
}



t_list* detectionAlgorithm(ITEM_NIVEL* listaItems,t_list* listaPersonajes){

    int h = 2;
    int f = 2;
    int m = 2;
    t_list * listaResultados = list_create();

    if (list_size(listaPersonajes) > 0 && algunRecursoVacio(listaItems, &h, &f, &m) == 1) {


        int* valor = (int*)malloc(sizeof(int));
        int i = 0;

        for (i = 0; i < list_size(listaPersonajes); i++) {
            //hacer otro for para que cada personaje recorra toda la lista de personajes

            datos_personaje * pers = list_get(listaPersonajes, i);

            if (miPersonajeNesecita(pers, h, f, m) == 1) {

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
