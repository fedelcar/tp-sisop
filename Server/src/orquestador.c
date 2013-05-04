/*
 * orquestador.c
 *
 *  Created on: May 1, 2013
 *      Author: lucas
 */
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <commons/collections/queue.h>
#include "uncommons/fileStructures.h"
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>


#define MAXSIZE 1024


void orquestador(){

	t_queue *blocked_characters = queue_create();

	t_list *levelsList = list_create();

	char *levelName = (char *) malloc(MAXSIZE);

	level_address *addresses = (level_address *) malloc(sizeof(level_address));

	t_dictionary *levelsMap = dictionary_create();

	levelsMap = getLevelsMap();

	int i;

	for( i = 0 ; i < list_size(levelsList) ; i++){

		levelName = (char *) list_get(levelsList,i);

		addresses = (level_address*) dictionary_get(levelsMap, levelName);

		//TODO THREAD LEVEL SCHEDULER

	}

	free(levelName);
	free(addresses);
	free(levelsList);
}
