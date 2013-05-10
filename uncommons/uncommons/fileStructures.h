/*
 * fileStructures.c
 *
 *  Created on: May 1, 2013
 *      Author: lucas
 */

#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>

#define NOMBRE "Nombre"
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
	char *caja1;
	char *caja2;
	char *caja3;
	char *orquestador;
	char *tiempoChequeoDeadlock;
	char *recovery;
} level;

typedef struct{
	char *nivel;
	char *planificador;

}level_address;

typedef struct{
	char *nombre;
	char *simbolo;
	t_list *planDeNiveles;
	t_dictionary *obj;
	char *vidas;
	char *orquestador;
} character;

typedef struct{
	char *turnos;
	char *sleep;
}level_attributes;


t_dictionary* getCharacters();

t_dictionary* getLevelsMap();

t_list* getLevelsInfo();

level* getLevel(char *finalPath);

t_list *getLevelsList();
