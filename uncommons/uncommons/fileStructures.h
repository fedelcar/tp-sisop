
/*
 * fileStructures.c
 *
 *  Created on: May 1, 2013
 *      Author: lucas
 */

#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/queue.h>
#include <commons/log.h>
#include <stdint.h>

#define NOMBRE "nombre"
#define CAJA "Caja"
#define ORQUESTADORBASE "orquestador"
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
	long tiempoChequeoDeadlock;
	long recovery;
	char* localIp;
} t_level_config;

typedef struct{
	char *nivel;
	char *planificador;

}t_level_address;


typedef struct{
	long posX;
	long posY;
}t_posicion;

typedef struct{
	char *nombre;
	char *simbolo;
	t_list *planDeNiveles;
	t_dictionary *obj;
	long vidas;
	char *orquestador;
} t_character;

typedef struct{
	char *turnos;
	char *sleep;
}t_level_attributes;

typedef struct{
	char *nombre;
	char *simbolo;
	long *instancias;
	long *posX;
	long *posY;
}t_caja;

typedef struct{
	long intervalo;
	long puerto;
	long turnos;
	char *argumento1;
	char *argumento2;
	char *argumento3;
}t_orquestador;

typedef struct{
	t_queue *character_queue;
	t_queue *blocked_queue;
	char *port;
	long portInt;
	t_orquestador *orquestador_config;
	char *path;
	long listen_sd;
	fd_set *master_set;
	t_list *pjList;
	t_list *simbolos;
	t_log *log;
	void *personajeCorriendo;
	pthread_mutex_t *mutex;
}t_scheduler_queue;

t_dictionary* getCharacters();

t_dictionary* getLevelsMap();

t_dictionary* getLevels();

t_list* getLevelsInfo();

t_list *getLevelsList();

t_level_attributes *getLevelAttributes();

t_orquestador* getOrquestador(char* finalPath);

t_character* getCharacter(char* finalPath);

t_level_config* getLevel(char* finalPath, t_list *listaSimbolos);


