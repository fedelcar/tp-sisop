/*
 * inotify.h
 *
 *  Created on: 27/06/2013
 *      Author: utnso
 */
#include<commons/collections/list.h>

typedef struct {
	char* path;
	t_list* lista;
} inotify_struct;

typedef struct{
	char* nombre;
	int valor;
}inotify_list_struct;

int inotify(inotify_struct *datos);
