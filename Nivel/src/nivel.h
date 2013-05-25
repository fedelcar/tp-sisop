/*
 * nivel.h
 *
 *  Created on: May 24, 2013
 *      Author: lucas
 */

#include "uncommons/fileStructures.h"

typedef struct{
  pthread_mutex_t *readLock;
	pthread_mutex_t *writeLock;
	t_level_config *level_config;
	int *fd;
}resource_struct;
