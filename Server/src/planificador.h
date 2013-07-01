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
	int *timeToWait;

};

typedef struct{
	int* fd;
	char recurso;
	pthread_mutex_t *readlock;
}blocked_character;

void planificar(t_scheduler_queue *scheduler_queue) ;


void planificador(t_scheduler_queue *scheduler_queue);
