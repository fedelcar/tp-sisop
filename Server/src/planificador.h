/*
 * planificador.h
 *
 *  Created on: Apr 28, 2013
 *      Author: lucas
 */

#include "commons/collections/queue.h"

struct scheduler_struct{

	char *port;
	int *timeToWait;

};

void planificador(char* port);



