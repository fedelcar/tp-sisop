/*
 * fileStructures.c
 *
 *  Created on: May 1, 2013
 *      Author: lucas
 */

#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>

#define NOMBRE "nombre"
#define CAJA1 "Caja1"
#define CAJA2 "Caja2"
#define CAJA3 "Caja3"
#define ORQUESTADOR "orquestador"
#define TIEMPOCHEQUEODEADLOCK "TiempoChequeoDeadlock"
#define RECOVERY "Recovery"
#define SIMBOLO "simbolo"
#define PLANDENIVELES "planDeNiveles"
#define OBJ "obj"

#define VIDAS "vidas"

typedef struct {
	char *nombre;
	t_dictionary *cajas;
	char *orquestador;
	char *tiempoChequeoDeadlock;
	char *recovery;
} t_level;

typedef struct{
	char *nivel;
	char *planificador;

}t_level_address;

typedef struct{
	char *nombre;
	char *simbolo;
	t_list *planDeNiveles;
	t_dictionary *obj;
	char *vidas;
	char *orquestador;
} t_character;

typedef struct{
	char *turnos;
	char *sleep;
}t_level_attributes;


t_dictionary* getCharacters();

t_dictionary* getLevelsMap();

t_list* getLevelsInfo();

t_level* getLevel(char *finalPath);

t_list *getLevelsList();

t_level_attributes *getLevelAttributes();
