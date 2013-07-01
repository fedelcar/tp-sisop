/*
 ============================================================================
 Name        : pruebaKoopa.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "memoria.h"

void main(){

char* txtmemoria = crear_memoria(100);

int exito = almacenar_particion(txtmemoria,'a',5,"holas");
int exito4 = almacenar_particion(txtmemoria,'c',23," prueba fantinofantino ");
int exito2 = almacenar_particion(txtmemoria,'b',9,"hola fede");
int exito3 = almacenar_particion(txtmemoria,'c',2,"aa");

int exito5 = eliminar_particion(txtmemoria,'a');

printf(txtmemoria);
printf ("\n");

t_list* prueba = particiones(txtmemoria);
liberar_memoria(txtmemoria);
}
