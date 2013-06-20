/*
 * nivel.h
 *
 *  Created on: May 24, 2013
 *      Author: lucas
 */

#include "uncommons/fileStructures.h"
#include<nivel-gui/tad_items.h>

typedef struct {
	pthread_mutex_t *readLock;
	pthread_mutex_t *writeLock;
	t_level_config *level_config;
	ITEM_NIVEL * listaItems;
	int *fd;
} resource_struct;

typedef struct mensaje {
	char *nombre;
	char caracter;
	t_posicion * pos;
} mensaje_t;

typedef struct {
	int F ;
	int H ;
	int M ;
	}recursos_otorgados;
