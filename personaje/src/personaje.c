/*
 ============================================================================
 Name        : prueba.c
 Author      :
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <commons/temporal.h>
#include <commons/config.h>
#include "personaje.h"
#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/list.h>



t_log *initiate_log (int Level)
{
	t_log *logTemp;
	logTemp = log_create("/home/federico/workspace3/logs/logInfo.txt","prueba.c",1,Level);
	return logTemp;
}

int main (void){
	t_log *logInfo = initiate_log(LOG_LEVEL_INFO);
	t_log *logWarning = initiate_log(LOG_LEVEL_WARNING);
	t_log *logDebug = initiate_log(LOG_LEVEL_DEBUG);
	t_log *logError = initiate_log(LOG_LEVEL_ERROR);

	log_debug(logDebug,"Prueba de log de Debug");

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

	}


	log_debug(logDebug, "Se finalizo el programa");
	return 0;
}

int mainold(void){
	t_log *log = log_create("/home/federico/workspace3/log.txt","prueba.c",1,LOG_LEVEL_DEBUG);

	t_config *asd = config_create("/home/federico/workspace3/config.txt");
	char *nivel = config_get_string_value(asd,"Nombre");
	char *ip_orquestador = config_get_string_value(asd,"orquestador");
	log_info(log, nivel);
	log_info(log,ip_orquestador);
//	int *tiempo_chequeo_deadlock = config_get_int_value(asd,"TiempoChequeoDeadlock");
//	int *recovery = config_get_int_value(asd,"Recovery");
	char* caja_en_string = config_get_string_value(asd,"Caja1");
	char* caja2 = config_get_string_value(asd,"Caja2");
	char* caja3 = config_get_string_value(asd,"Caja3");

	printf("%s \n%s \n%s \n",caja_en_string,caja2,caja3);

	char* coma = ',';
	char** algo2 = string_split(caja_en_string, &coma);
	int i =0;
	do {
		log_info(log,algo2[i]);
		i++;
	} while (algo2[i]!= NULL);


//	printf("%s",puto2[1]);

//	t_caja *cargaCaja;
//	char **jose = malloc(20);
//	jose = string_split(caja_en_string, &coma);
//	//log_info(log,*(jose));
//	log_info(log,caja_en_string);



	//jorge = config_get_caja(asd,"Caja1");
//	log_info(log,"asd");
//	char *tipo = jorge->tipo;
//	char simbol = jorge->simbolo;
//	int instancia = jorge->instancias;
//	int posx = jorge->posX;
//	int posy = jorge->posY;
//
//	//log_info(log,"pito");
//	//log_info(log,tipo);
//
//	//log_info(log,simbol);
//	//log_info(log,instancia);
//	//log_info(log,posx);
//	//log_info(log,posy);
//
//	printf("%s %d %d %d \n",tipo,instancia,posx,posy);
//
//
	//free(jorge);
	return EXIT_SUCCESS;
}

