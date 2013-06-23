/*
 * planificador.h
 *
 *  Created on: Apr 28, 2013
 *      Author: lucas
 */

#include <uncommons/fileStructures.h>

struct scheduler_struct{

	char *port;
	int *timeToWait;

};

void planificador(t_scheduler_queue *scheduler_queue);



