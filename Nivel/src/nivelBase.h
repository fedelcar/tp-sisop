/*
 * nivel.h
 *
 *  Created on: May 24, 2013
 *      Author: lucas
 */

#include "uncommons/fileStructures.h"
#include <nivel-gui/tad_items.h>
#include <pthread.h>

typedef struct mensaje {
	char *nombre;
	char caracter;
	t_posicion * pos;
} mensaje_t;

typedef struct {
	int F;
	int H;
	int M;
} recursos_otorgados;

typedef struct {
	int id;
	pthread_mutex_t *readLock;
	pthread_mutex_t *writeLock;
	t_level_config *level_config;
	ITEM_NIVEL * listaItems;
	recursos_otorgados *recursosAt;
	char recursoBloqueado;
	int *fd;
} resource_struct;


typedef struct {
	int id;
	pthread_t *thread;
	int *F;
	int *H;
	int *M;
	char *recurso;
} datos_personaje;

typedef struct {
	t_list *list;
	ITEM_NIVEL *items;
	char *socket;
	int *recovery;
}deadlock_struct;
