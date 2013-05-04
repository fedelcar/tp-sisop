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

#define EXTENSIONLENGTH 7
#define NEXTPORT 2
#define MAXSIZE 1024
#define START 0
#define IP "127.0.0.1"
#define STARTINGPORT 9931
const char *fileExtension = ".config";
const char *directoryPathLevels = "/home/lucas/Levels";
const char *directoryPathCharacters = "/home/lucas/Characters";

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

	 /* Leemos las entradas del directorio */
	 while ((direntp = readdir(dirp)) != NULL) {

		 if (strlen(direntp->d_name) >= EXTENSIONLENGTH && strcmp(direntp->d_name + strlen(direntp->d_name) - EXTENSIONLENGTH, fileExtension) == 0) {

			 char *finalPath = (char*) malloc(MAXSIZE);
			 character *character_struct = (character *) malloc(sizeof(character));

			 string_append(&finalPath, directoryPathCharacters);
			 string_append(&finalPath, "/");
			 string_append(&finalPath, direntp->d_name);

			 configFile = config_create(finalPath);

			 character_struct->nombre = config_get_string_value(configFile, NOMBRE);
			 character_struct->simbolo = config_get_string_value(configFile, SIMBOLO);
			 character_struct->planDeNiveles = config_get_string_value(configFile, PLANDENIVELES);
			 character_struct->obj1 = config_get_string_value(configFile, OBJ1);
			 character_struct->obj2 = config_get_string_value(configFile, OBJ2);
			 character_struct->obj3 = config_get_string_value(configFile, OBJ3);
			 character_struct->vidas = config_get_string_value(configFile, VIDAS);
			 character_struct->orquestador = config_get_string_value(configFile, ORQUESTADOR);

			 dictionary_put(character_dictionary, character_struct->nombre, character_struct);

			 free(finalPath);
			 free(character_struct);
		 }

	 }

	 /* Cerramos el directorio */
	 closedir(dirp);
	 return character_dictionary;
}

t_dictionary* getLevelsMap(){

	t_dictionary *level_addresses = dictionary_create();
	t_config *configFile;
	int PORTSCHEDULER = STARTINGPORT;
	int PORTLEVEL = PORTSCHEDULER + 1;
	char *bufferScheduler = malloc(MAXSIZE);
	char *bufferLevel = malloc(MAXSIZE);

	 /* Variables */
	 DIR *dirp;
	 struct dirent *direntp;

	 /* Abrimos el directorio */
	 dirp = opendir(directoryPathLevels);
	 if (dirp == NULL){
	 printf("Error: No se puede abrir el directorio\n");
	 exit(2);
	 }

	 /* Leemos las entradas del directorio */
	 while ((direntp = readdir(dirp)) != NULL) {

		 if (strlen(direntp->d_name) >= EXTENSIONLENGTH && strcmp(direntp->d_name + strlen(direntp->d_name) - EXTENSIONLENGTH, fileExtension) == 0) {

			 char *finalPath = (char*) malloc(MAXSIZE);
			 level_address *level = (level_address *) malloc(sizeof(level_address));
			 sprintf(bufferScheduler, "%d", PORTSCHEDULER);
			 sprintf(bufferLevel, "%d", PORTLEVEL);

			 string_append(&finalPath, directoryPathLevels);
			 string_append(&finalPath, "/");
			 string_append(&finalPath, direntp->d_name);

			 configFile = config_create(finalPath);

			 level->nombre = config_get_string_value(configFile, NOMBRE);
			 level->path = finalPath;
			 level->portScheduler = bufferScheduler;
			 level->portLevel = bufferLevel;

			 dictionary_put(level_addresses, level->nombre, level);

			 PORTLEVEL = PORTLEVEL + NEXTPORT;
			 PORTSCHEDULER = PORTSCHEDULER + NEXTPORT;
			 free(finalPath);
			 free(level);
		 }

	 }

	 /* Cerramos el directorio */
	 closedir(dirp);
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

	 /* Leemos las entradas del directorio */
	 while ((direntp = readdir(dirp)) != NULL) {

		 if (strlen(direntp->d_name) >= EXTENSIONLENGTH && strcmp(direntp->d_name + strlen(direntp->d_name) - EXTENSIONLENGTH, fileExtension) == 0) {

			 char *finalPath = (char*) malloc(MAXSIZE);

			 string_append(&finalPath, directoryPathLevels);
			 string_append(&finalPath, "/");
			 string_append(&finalPath, direntp->d_name);

			 configFile = config_create(finalPath);

			 list_add(level_info, config_get_string_value(configFile, NOMBRE));
			 free(finalPath);
		 }
	 }

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

