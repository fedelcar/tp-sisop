/*
 ============================================================================
 Name        : prueba.c
 Author      :
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "personaje.h"
#include <stdio.h>
#include <stdlib.h>
#include <commons/temporal.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/collections/node.h>
#include <uncommons/fileStructures.h>
#include <uncommons/SocketsCliente.h>
#include <uncommons/SocketsBasic.h>



t_log *initiate_log (int Level)
{
	t_log *logTemp;
	logTemp = log_create("/home/federico/workspace3/logs/logInfo.txt","personaje.c",1,Level);
	return logTemp;
}

int main (void){
	t_log *logInfo = initiate_log(LOG_LEVEL_INFO);
	t_log *logWarning = initiate_log(LOG_LEVEL_WARNING);
	t_log *logDebug = initiate_log(LOG_LEVEL_DEBUG);
	t_log *logError = initiate_log(LOG_LEVEL_ERROR);

	log_debug(logDebug,"Prueba de log de Debug");

	t_dictionary *personaje = getCharacters();



	t_config *config_Personaje = config_create("/home/federico/workspace3/configs/mario.txt");
	char *nombre_Personaje = config_get_string_value(config_Personaje, "Nombre");
	char *simbolo_Personaje = config_get_string_value(config_Personaje, "Simbolo");
	char **niveles_Personaje = config_get_array_value(config_Personaje,"planDeNiveles");
	log_debug(logDebug,"%s",niveles_Personaje[0]);

	int i =0;
	int cantNiveles = 0;
	do {
		cantNiveles ++;
		i++;
	} while (niveles_Personaje[i] != NULL);
	log_debug(logDebug,"%d",cantNiveles);

	t_list *lista_Objetivos = list_create();

	for (i = 0; i < cantNiveles; i++) {
		printf("obj[%s]\n",niveles_Personaje[i]);
	//char* nivel = ("obj[%s]",niveles_Personaje[i]);
	//log_debug(logDebug,nivel);
	//char** objetivos_nivel = config_get_array_value(config_Personaje,nivel);
	//list_add(lista_Objetivos, objetivos_nivel);
	}


	log_debug(logDebug, "Se finalizo el programa");
	return EXIT_SUCCESS;
}
