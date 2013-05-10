/*
 * main.c
 *
 *  Created on: Apr 14, 2013
 *      Author: lucas
 */
#include <pthread.h>
#include "planificador.h"
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <commons/collections/queue.h>
#include <commons/collections/dictionary.h>
#include <commons/config.h>
#include <string.h>
#include <commons/string.h>
#include "uncommons/fileStructures.h"


int mainn(){

	pthread_t t;
	void *status;
	struct scheduler_struct *schedulerStruct = (struct scheduler_struct *) malloc(sizeof(struct scheduler_struct));
	schedulerStruct->port = "9930";
	schedulerStruct->timeToWait = 1;

	pthread_create(&t, NULL, (void *) planificador, (struct scheduler_struct *) schedulerStruct);

	pthread_join(t, &status);

	return 0;
}
