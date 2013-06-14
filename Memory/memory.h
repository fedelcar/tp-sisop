/*
 * memory.h
 *
 *  Created on: Jun 13, 2013
 *      Author: federico
 */

#ifndef MEMORY_H_
#define MEMORY_H_
#include <commons/collections/list.h>

typedef char* t_memoria;

typedef struct t_particion {
	char id;
	int inicio;
	int tamanio;
	t_memoria dato;
	bool libre;
} particion;
//estructura que define una partición

t_memoria crear_memoria(int tamanio);
//Crea el segmento de memoria a particionar

int almacenar_particion(t_memoria segmento, char id, int tamanio, t_memoria contenido);
/* Crea una particion dentro del segmento de memoria del tamano, identificador
  y contenido especificado. Devuelve el valor numerico -1 en caso de error (ej: tamano de
  la particion mayor que el tamano total del segemento, id duplicado, etc), 1 en caso de
  exito y 0 en caso de no encontrar una particion libre lo suficientemente grande
  para almacenar la particion */

int eliminar_particion(t_memoria segmento, char id);
/*Esta funcion elimina la particion dentro del segmento de memoria correspondiente
 al identificador enviado como paramtreo. Devuelve el valor numerico 1 en caso de exito
 y 0 en caso de no encontrar una particion con dicho identificador.*/

void liberar_memoria(t_memoria segmento);
// Esta funcion libera los recursos tomados en crear_memoria()

t_list* particiones(t_memoria segmento);
/*Esta funcion devuelve una lista en el formato t_list de las commons-library con
la siguiente descripcion por cada particion */


#endif /* MEMORY_H_ */
