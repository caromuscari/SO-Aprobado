/*
 ============================================================================
 Name        : File.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "Funciones.h"
#include <funcionesCompartidas/log.h>
#include <funcionesCompartidas/funcionesNET.h>
#include <pthread.h>
#include "hiloSelect.h"
#include "hiloConsola.h"
#include "hiloDump.h"
#include "hiloInotify.h"
#include <commons/collections/dictionary.h>
#include <commons/collections/queue.h>
#include <commons/bitarray.h>
#include <sys/stat.h>
#include <signal.h>
#include "buscar.h"


char* magic_number;
int tBloques;
int cantBloques;

t_log* alog;
t_bitarray* bitmap;
int socketfs;
int controlador;
char* posicion;
int bitm;
t_queue * nombre;
int loop;

structConfig * config;
pthread_t hiloConsola, hiloSelect,hiloDump, hiloInotify;
t_dictionary * clientes, *memtable, *tablas;
t_list * listaTabla;

struct stat mystat;


int main(int argc, char *argv[]) {

	int metadata;

	inicializar();
	archivoDeConfiguracion(argv[1]);

	metadata = leer_metadata();
	bitm = abrir_bitmap();


	if(!(metadata ==-1 || bitm ==-1)){

		tablas = listarDirectorio();

        pthread_create(&hiloInotify, NULL, (void*)hiloinotify,argv[1]);
		pthread_create(&hiloSelect, NULL, (void*)hiloselect,NULL);
		pthread_create(&hiloConsola, NULL, (void*)hiloconsola,NULL);
		pthread_create(&hiloDump, NULL, (void*)hilodump,NULL);


        pthread_join(hiloInotify, NULL);
		pthread_join(hiloSelect,NULL);
		pthread_join(hiloConsola, NULL);
		pthread_join(hiloDump, NULL);
	}


	finalizar();

	return EXIT_SUCCESS;
}


