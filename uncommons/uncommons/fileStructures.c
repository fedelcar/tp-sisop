/*
 * fileStructures.c
 *
 *  Created on: May 1, 2013
 *      Author: lucas
 */

#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <commons/collections/queue.h>
#include <commons/collections/dictionary.h>
#include <commons/config.h>
#include <string.h>
#include <commons/string.h>
#include "fileStructures.h"

#define NIVELES "niveles"
#define COMA ","
#define OPENBRACKET "obj["
#define CLOSEBRACKET "]"
#define EXTENSIONLENGTH 7
#define NEXTPORT 2
#define MAXSIZE 1024
#define START 0
#define IP "127.0.0.1"
#define STARTINGPORT 9931
#define fileExtension ".config"
#define directoryPathLevels "/home/lucas/config/niveles"
#define directoryPathCharacters "/home/lucas/config/personajes"
#define directoryPathConfig "/home/lucas/config/orquestador"
#define ORQUESTADOR "orquestador.config"
#define TURNOS "turnos"
#define SLEEP "sleep"

char* getFullKey(char *nivel, char *key);

t_dictionary* getCharacters(){

	t_dictionary *character_dictionary = dictionary_create();
	t_config *configFile;

	 /* Variables */
	 DIR *dirp;
	 struct dirent *direntp;

	 /* Abrimos el directorio */
	 dirp = opendir(directoryPathCharacters);
	 if (dirp == NULL){
	 printf("Error: No se puede abrir el directorio\n");
	 exit(2);
	 }

	 int nivel = 0;
	 char *finalPath = (char*) malloc(MAXSIZE);
	 char *key = (char*) malloc(MAXSIZE);

	 /* Leemos las entradas del directorio */
	 while ((direntp = readdir(dirp)) != NULL) {

		 if (strlen(direntp->d_name) >= EXTENSIONLENGTH && strcmp(direntp->d_name + strlen(direntp->d_name) - EXTENSIONLENGTH, fileExtension) == 0) {

			 character *character_struct = (character *) malloc(sizeof(character));
			 string_append(&finalPath, directoryPathCharacters);
			 string_append(&finalPath, "/");
			 string_append(&finalPath, direntp->d_name);

			 configFile = config_create(finalPath);

			 character_struct->nombre = config_get_string_value(configFile, NOMBRE);
			 character_struct->simbolo = config_get_string_value(configFile, SIMBOLO);
			 char **planDeNiveles = config_get_array_value(configFile, PLANDENIVELES);
			 character_struct->vidas = config_get_string_value(configFile, VIDAS);
			 character_struct->orquestador = config_get_string_value(configFile, ORQUESTADOR);

			 while(planDeNiveles[nivel] != NULL){
				 list_add(character_struct->planDeNiveles, planDeNiveles[nivel]);
				 nivel++;
			 }

			 for(nivel = 0; nivel<list_size(character_struct->planDeNiveles) ; nivel++){
				 dictionary_put(character_struct->obj, list_get(character_struct->planDeNiveles, nivel), config_get_string_value(configFile, getFullKey(list_get(character_struct->planDeNiveles, nivel), key)));
			 }

			 dictionary_put(character_dictionary, character_struct->nombre, character_struct);

		 }
	 }
	 free(key);
	 free(finalPath);
	 /* Cerramos el directorio */
	 closedir(dirp);
	 return character_dictionary;
}

t_dictionary* getLevelsMap(){

	t_dictionary *level_addresses = dictionary_create();
	t_config *configFile;

	 char **levelString = (char*) malloc(MAXSIZE);
	 char *finalPath = (char*) malloc(MAXSIZE);
	 int niveles = 0;
	 char **listaNiveles;
	 char *levelName = (char*) malloc(MAXSIZE);


			 string_append(&finalPath, directoryPathConfig);
			 string_append(&finalPath, "/");
			 string_append(&finalPath, ORQUESTADOR);

			 configFile = config_create(finalPath);

			 listaNiveles = config_get_array_value(configFile, NIVELES);

			  while(listaNiveles[niveles] != NULL){
				  level_address *level = (level_address *) malloc(sizeof(level_address));
				  levelName = listaNiveles[niveles];
				  levelString = string_split(config_get_string_value(configFile, levelName),COMA);

				 level->nivel = levelString[0];
				 level->planificador = levelString[1];

				 dictionary_put(level_addresses, levelName, (void*) level);
				 niveles++;
			 }

	 free(levelName);
	 free(finalPath);

	 return level_addresses;
}

t_list* getLevelsInfo(){

	t_list *level_info = list_create();
	t_config *configFile;

	 /* Variables */
	 DIR *dirp;
	 struct dirent *direntp;

	 /* Abrimos el directorio */
	 dirp = opendir(directoryPathLevels);
	 if (dirp == NULL){
	 printf("Error: No se puede abrir el directorio\n");
	 exit(2);
	 }

	 char *finalPath = (char*) malloc(MAXSIZE);

	 /* Leemos las entradas del directorio */
	 while ((direntp = readdir(dirp)) != NULL) {

		 if (strlen(direntp->d_name) >= EXTENSIONLENGTH && strcmp(direntp->d_name + strlen(direntp->d_name) - EXTENSIONLENGTH, fileExtension) == 0) {

			 string_append(&finalPath, directoryPathLevels);
			 string_append(&finalPath, "/");
			 string_append(&finalPath, direntp->d_name);

			 configFile = config_create(finalPath);

			 list_add(level_info, config_get_string_value(configFile, NOMBRE));
		 }
	 }

	 free(finalPath);
	 /* Cerramos el directorio */
	 closedir(dirp);
	 return level_info;
}


level* getLevel(char *finalPath){

	level *level_struct = (level*) malloc(sizeof(level));
	t_config *configFile;

	configFile = config_create(finalPath);

	level_struct->caja1 = config_get_string_value(configFile, CAJA1);
	level_struct->caja2 = config_get_string_value(configFile, CAJA2);
	level_struct->caja3 = config_get_string_value(configFile, CAJA3);
	level_struct->nombre = config_get_string_value(configFile, NOMBRE);
	level_struct->orquestador = config_get_string_value(configFile, ORQUESTADOR);
	level_struct->recovery = config_get_string_value(configFile, RECOVERY);
	level_struct->tiempoChequeoDeadlock = config_get_string_value(configFile, TIEMPOCHEQUEODEADLOCK);

return level_struct;
}


char* getFullKey(char *nivel, char *key){

	string_append(&key, OPENBRACKET);
	string_append(&key, nivel);
	string_append(&key, CLOSEBRACKET);

	return key;
}

t_list *getLevelsList(){

	t_config *configFile;
	int nivel = 0;
	char **planDeNiveles;
	t_list *levelList = list_create();

	 char *finalPath = (char*) malloc(MAXSIZE);

			 string_append(&finalPath, directoryPathConfig);
			 string_append(&finalPath, "/");
			 string_append(&finalPath, ORQUESTADOR);

			 configFile = config_create(finalPath);

			 planDeNiveles = config_get_array_value(configFile, NIVELES);

			 while(planDeNiveles[nivel] != NULL){
				 list_add(levelList, planDeNiveles[nivel]);
				 nivel++;
			 }

			 free(finalPath);

	 return levelList;
}

level_attributes *getLevelAttributes(){

	t_config *configFile;

	level_attributes *level = (level_attributes*) malloc(sizeof(level_attributes));

	 char *finalPath = (char*) malloc(MAXSIZE);

			 string_append(&finalPath, directoryPathConfig);
			 string_append(&finalPath, "/");
			 string_append(&finalPath, ORQUESTADOR);

			 configFile = config_create(finalPath);

			 level->sleep = config_get_string_value(configFile, TURNOS);
			 level->turnos = config_get_string_value(configFile, SLEEP);

			 free(finalPath);

	 return level;
}
