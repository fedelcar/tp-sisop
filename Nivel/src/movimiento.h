
/*
 * movimiento.h
 *
 *  Created on: Jun 12, 2013
 *      Author: lucas
 */

#include <commons/log.h>

void movimientoPersonaje(resource_struct* resources, int rows, int cols, char* mensaje, fd_set *master_set, int fileDescriptorPj, int socketOrquestador, t_list *listaSimbolos, t_log* logEntrante);

