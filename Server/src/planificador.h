/*
 * planificador.h
 *
 *  Created on: Apr 28, 2013
 *      Author: lucas
 */

#include <uncommons/fileStructures.h>
#define MAXQUEUE 100

struct scheduler_struct{

	char *port;
	long *timeToWait;

};

typedef struct{
	long fd;
	char* nombre;
	long *respondio;
}personaje_planificador;

typedef struct{
	personaje_planificador *personaje;
	char recurso;
	pthread_mutex_t *readlock;
}blocked_character;


void planificar(t_scheduler_queue *scheduler_queue) ;


void planificador(t_scheduler_queue *scheduler_queue);
