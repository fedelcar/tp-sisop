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
#include <unistd.h>
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
#define directoryPathLevels "/home/tp/config/niveles"
#define directoryPathCharacters "/home/tp/config/personajes"
#define directoryPathConfig "/home/tp/config/orquestador"
#define ORQUESTADOR "orquestador.config"
#define TURNOS "turnos"
#define SLEEP "sleep"
#define CAJASMAXIMAS 3

char* getFullKey(char *nivel, char *key);
t_caja* getBox(t_config *configFile, int *boxNumber, t_list *listaSimbolos);

t_dictionary* getCharacters() {

	t_dictionary *character_dictionary = dictionary_create();
	t_config *configFile;

	/* Variables */
	DIR *dirp;
	struct dirent *direntp;

	/* Abrimos el directorio */
	dirp = opendir(directoryPathCharacters);
	if (dirp == NULL ) {
		printf("Error: No se puede abrir el directorio\n");
		exit(2);
	}

	char *finalPath = (char*) malloc(MAXSIZE);
	int nivel = 0;
	char *key = (char*) malloc(MAXSIZE);

	/* Leemos las entradas del directorio */
	while ((direntp = readdir(dirp)) != NULL ) {

		if (strlen(direntp->d_name) >= EXTENSIONLENGTH
				&& strcmp(
						direntp->d_name
								+ strlen(direntp->d_name) - EXTENSIONLENGTH,
						fileExtension) == 0) {

			t_character *character_struct = (t_character *) malloc(
					sizeof(t_character));
			character_struct->planDeNiveles = list_create();
			character_struct->obj = dictionary_create();

			string_append(&finalPath, directoryPathCharacters);
			string_append(&finalPath, "/");
			string_append(&finalPath, direntp->d_name);

			configFile = config_create(finalPath);

			character_struct->nombre = config_get_string_value(configFile,
					NOMBRE);
			character_struct->simbolo = config_get_string_value(configFile,
					SIMBOLO);
			char **planDeNiveles = config_get_array_value(configFile,
					PLANDENIVELES);
			character_struct->vidas = atoi(
					config_get_string_value(configFile, VIDAS));
			character_struct->orquestador = config_get_string_value(configFile,
					ORQUESTADORBASE);

			while (planDeNiveles[nivel] != NULL ) {
				list_add(character_struct->planDeNiveles, planDeNiveles[nivel]);
				nivel++;
			}

			for (nivel = 0; nivel < list_size(character_struct->planDeNiveles);
					nivel++) {
				char *nombre = (char*) malloc(MAXSIZE);
				char **array = (char*) malloc(MAXSIZE);
				t_list *anotherList = list_create();
				int elemento = 0;

				nombre = list_get(character_struct->planDeNiveles, nivel);
				array = config_get_array_value(configFile,
						getFullKey(
								list_get(character_struct->planDeNiveles,
										nivel), key));

				while (array[elemento] != NULL ) {
					list_add(anotherList, array[elemento]);
					elemento++;
				}

				dictionary_put(character_struct->obj, nombre, anotherList);
				memset(key, 0, sizeof(key));
			}

			dictionary_put(character_dictionary, character_struct->nombre,
					character_struct);
			nivel = 0;
			memset(finalPath, 0, sizeof(finalPath));
		}
	}
	free(finalPath);
	free(key);
	/* Cerramos el directorio */
	closedir(dirp);
	return character_dictionary;
}

t_dictionary* getLevelsMap() {

	t_dictionary *level_addresses = dictionary_create();
	t_config *configFile;

	char **levelString = (char*) malloc(MAXSIZE);
	char *finalPath = (char*) malloc(MAXSIZE);
	memset(finalPath, 0, sizeof(finalPath));
	int niveles = 0;
	char **listaNiveles;
	char *levelName = (char*) malloc(MAXSIZE);

	string_append(&finalPath, directoryPathConfig);
	string_append(&finalPath, "/");
	string_append(&finalPath, ORQUESTADOR);

	configFile = config_create(finalPath);

	listaNiveles = config_get_array_value(configFile, NIVELES);

	while (listaNiveles[niveles] != NULL ) {
		t_level_address *level = (t_level_address *) malloc(
				sizeof(t_level_address));
		levelName = listaNiveles[niveles];
		levelString = string_split(
				config_get_string_value(configFile, levelName), COMA);

		level->nivel = levelString[0];
		level->planificador = levelString[1];

		dictionary_put(level_addresses, levelName, (void*) level);
		niveles++;
	}

	free(levelName);
	free(finalPath);

	return level_addresses;
}

t_list* getLevelsInfo() {

	t_list *level_info = list_create();
	t_config *configFile;

	/* Variables */
	DIR *dirp;
	struct dirent *direntp;

	/* Abrimos el directorio */
	dirp = opendir(directoryPathLevels);
	if (dirp == NULL ) {
		printf("Error: No se puede abrir el directorio\n");
		exit(2);
	}

	char *finalPath = (char*) malloc(MAXSIZE);
	memset(finalPath, 0, sizeof(finalPath));

	/* Leemos las entradas del directorio */
	while ((direntp = readdir(dirp)) != NULL ) {

		if (strlen(direntp->d_name) >= EXTENSIONLENGTH
				&& strcmp(
						direntp->d_name
								+ strlen(direntp->d_name) - EXTENSIONLENGTH,
						fileExtension) == 0) {

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

char* getFullKey(char *nivel, char *key) {

	string_append(&key, OPENBRACKET);
	string_append(&key, nivel);
	string_append(&key, CLOSEBRACKET);

	return key;
}

t_list *getLevelsList() {

	t_config *configFile;
	int nivel = 0;
	char **planDeNiveles;
	t_list *levelList = list_create();

	char *finalPath = (char*) malloc(MAXSIZE);
	memset(finalPath, 0, sizeof(finalPath));

	string_append(&finalPath, directoryPathConfig);
	string_append(&finalPath, "/");
	string_append(&finalPath, ORQUESTADOR);

	configFile = config_create(finalPath);

	planDeNiveles = config_get_array_value(configFile, NIVELES);

	while (planDeNiveles[nivel] != NULL ) {
		list_add(levelList, planDeNiveles[nivel]);
		nivel++;
	}

	free(finalPath);

	return levelList;
}

t_level_attributes *getLevelAttributes() {

	t_config *configFile;

	t_level_attributes *level = (t_level_attributes*) malloc(
			sizeof(t_level_attributes));

	char *finalPath = (char*) malloc(MAXSIZE);
	memset(finalPath, 0, sizeof(finalPath));

	string_append(&finalPath, directoryPathConfig);
	string_append(&finalPath, "/");
	string_append(&finalPath, ORQUESTADOR);

	configFile = config_create(finalPath);

	level->sleep = config_get_string_value(configFile, SLEEP);
	level->turnos = config_get_string_value(configFile, TURNOS);

	free(finalPath);

	return level;
}

//t_dictionary* getLevels() {
//
//	t_dictionary *level_dictionary = dictionary_create();
//	t_config *configFile;
//
//	/* Variables */
//	DIR *dirp;
//	struct dirent *direntp;
//
//	/* Abrimos el directorio */
//	dirp = opendir(directoryPathLevels);
//	if (dirp == NULL ) {
//		printf("Error: No se puede abrir el directorio\n");
//		exit(2);
//	}
//
//	char *finalPath = (char*) malloc(MAXSIZE);
//	int box = 0;
//	char *key = (char*) malloc(MAXSIZE);
//
//	/* Leemos las entradas del directorio */
//	while ((direntp = readdir(dirp)) != NULL ) {
//
//		if (strlen(direntp->d_name) >= EXTENSIONLENGTH
//				&& strcmp(
//						direntp->d_name
//								+ strlen(direntp->d_name) - EXTENSIONLENGTH,
//						fileExtension) == 0) {
//
//			t_level_config *level_config = (t_level_config *) malloc(
//					sizeof(t_level_config));
//
//			level_config->cajas = dictionary_create();
//
//			string_append(&finalPath, directoryPathLevels);
//			string_append(&finalPath, "/");
//			string_append(&finalPath, direntp->d_name);
//
//			configFile = config_create(finalPath);
//
//			level_config->nombre = config_get_string_value(configFile,
//					"Nombre");
//			level_config->orquestador = config_get_string_value(configFile,
//					"orquestador");
//			level_config->recovery = config_get_string_value(configFile,
//					RECOVERY);
//			level_config->tiempoChequeoDeadlock = config_get_string_value(
//					configFile, TIEMPOCHEQUEODEADLOCK);
//
//			for (box = 1; box <= CAJASMAXIMAS; box++) {
//				t_caja *caja = getBox(configFile, box, listaSimbolos);
//				dictionary_put(level_config->cajas, caja->nombre, caja);
//			}
//
//			dictionary_put(level_dictionary, level_config->nombre,
//					level_config);
//			box = 0;
//			memset(finalPath, 0, sizeof(finalPath));
//		}
//	}
//	free(finalPath);
//	free(key);
//	/* Cerramos el directorio */
//	closedir(dirp);
//	return level_dictionary;
//}

t_caja* getBox(t_config *configFile, int *boxNumber, t_list *listaSimbolos) {

	char *msg = (char*) malloc(MAXSIZE);
	sprintf(msg, "%d", boxNumber);

	char **elementos = (char*) malloc(MAXSIZE);
	char *oneBox = (char*) malloc(MAXSIZE);

	string_append(&oneBox, CAJA);
	string_append(&oneBox, msg);

	elementos = string_split(config_get_string_value(configFile, oneBox), COMA);

	t_caja *caja = (t_caja*) malloc(sizeof(t_caja));

	caja->nombre = elementos[0];
	caja->simbolo = elementos[1];
	caja->instancias = atoi(elementos[2]);
	caja->posX = atoi(elementos[3]);
	caja->posY = atoi(elementos[4]);

	list_add(listaSimbolos, caja->simbolo);

	free(elementos);
	free(oneBox);

	return caja;
}

t_character * getCharacter(char* finalPath) {

	t_dictionary *character_dictionary = dictionary_create();
	t_config *configFile;

	/* Variables */
	DIR *dirp;
	struct dirent *direntp;

	int nivel = 0;
	char *key = (char*) malloc(MAXSIZE);

	/* Leemos las entradas del directorio */

	t_character *character_struct = (t_character *) malloc(sizeof(t_character));
	character_struct->planDeNiveles = list_create();
	character_struct->obj = dictionary_create();

	configFile = config_create(finalPath);

	character_struct->nombre = config_get_string_value(configFile, NOMBRE);
	character_struct->simbolo = config_get_string_value(configFile, SIMBOLO);
	char **planDeNiveles = config_get_array_value(configFile, PLANDENIVELES);
	character_struct->vidas = atoi(config_get_string_value(configFile, VIDAS));
	character_struct->orquestador = config_get_string_value(configFile,
			ORQUESTADORBASE);

	while (planDeNiveles[nivel] != NULL ) {
		list_add(character_struct->planDeNiveles, planDeNiveles[nivel]);
		nivel++;
	}

	for (nivel = 0; nivel < list_size(character_struct->planDeNiveles);
			nivel++) {
		char *nombre = (char*) malloc(MAXSIZE);
		char **array = (char*) malloc(MAXSIZE);
		t_list *anotherList = list_create();
		int elemento = 0;

		nombre = list_get(character_struct->planDeNiveles, nivel);
		array = config_get_array_value(configFile,
				getFullKey(list_get(character_struct->planDeNiveles, nivel),
						key));

		while (array[elemento] != NULL ) {
			list_add(anotherList, array[elemento]);
			elemento++;
		}

		dictionary_put(character_struct->obj, nombre, anotherList);
		memset(key, 0, sizeof(key));
	}

	free(key);
	/* Cerramos el directorio */
	return character_struct;
}

t_level_config* getLevel(char* finalPath, t_list *listaSimbolos) {

	t_config *configFile;

	/* Variables */
	DIR *dirp;
	struct dirent *direntp;

	/* Abrimos el directorio */

	int box = 0;
	char *key = (char*) malloc(MAXSIZE);

	/* Leemos las entradas del directorio */

	t_level_config *level_config = (t_level_config *) malloc(
			sizeof(t_level_config));

	level_config->cajas = dictionary_create();

	configFile = config_create(finalPath);

	level_config->nombre = config_get_string_value(configFile, "Nombre");
	level_config->orquestador = config_get_string_value(configFile,
			"orquestador");
	level_config->recovery = atoi(config_get_string_value(configFile, RECOVERY));
	level_config->tiempoChequeoDeadlock = atoi(config_get_string_value(configFile,
			TIEMPOCHEQUEODEADLOCK));

	for (box = 1; box <= CAJASMAXIMAS; box++) {
		t_caja *caja = getBox(configFile, box, listaSimbolos);
		dictionary_put(level_config->cajas, caja->simbolo, caja);
	}

	return level_config;
}

t_orquestador* getOrquestador(char* finalPath) {

	t_config *configFile;

	int box = 0;
	char *key = (char*) malloc(MAXSIZE);

	/* Leemos las entradas del directorio */

	t_orquestador *orquestador = (t_level_config *) malloc(
			sizeof(t_orquestador));

	configFile = config_create(finalPath);

	orquestador->turnos = atoi(config_get_string_value(configFile, "turnos"));
	orquestador->puerto = atoi(
			config_get_string_value(configFile, "orquestador"));
	orquestador->intervalo = atoi(
			config_get_string_value(configFile, "intervalo"));
	orquestador->argumento1 = config_get_string_value(configFile, "argumento1");
	orquestador->argumento2 = config_get_string_value(configFile, "argumento2");
	orquestador->argumento3 = config_get_string_value(configFile, "argumento2");

	return orquestador;
}
