/*
 * Funciones.c
 *
 *  Created on: 12 abr. 2019
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <funcionesCompartidas/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include <commons/bitarray.h>
#include <errno.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "Funciones.h"
#include <signal.h>
#include <pthread.h>

extern struct stat mystat;

extern char* magic_number;
extern int tBloques;
extern int cantBloques;

extern structConfig * config;
extern t_dictionary * clientes, *memtable, *tablas;
extern t_log* alog;
extern t_bitarray* bitmap;
extern char* posicion;
extern int bitm;

extern pthread_t hiloConsola, hiloSelect,hiloDump;

void inicializar(){
	config = malloc(sizeof(structConfig));
	config->montaje = strdup("");
	magic_number = strdup("");
	config->puerto = strdup("");
	clientes = dictionary_create();
	memtable = dictionary_create();
	alog = crear_archivo_log("File System", true, "/home/utnso/Escritorio/log.txt");
}

void archivoDeConfiguracion(char* argv)
{
	t_config *configuracion;
	configuracion = config_create(argv);
	string_append(&config->puerto, config_get_string_value(configuracion, "PUERTO_ESCUCHA​"));
	string_append(&config->montaje, config_get_string_value(configuracion, "PUNTO_MONTAJE"));
	config->retardo = config_get_int_value(configuracion, "RETARDO");
	config->tam_value = config_get_int_value(configuracion, "TAMAÑO_VALUE");
	config->tiempo_dump = config_get_int_value(configuracion, "TIEMPO_DUMP");

	log_info(alog, "Lee el archivo de configuracion");

	config_destroy(configuracion);
}

int leer_metadata()
{
	char *ruta = strdup("");
	t_config *configuracion;

	string_append(&ruta,config->montaje);
	string_append(&ruta,"/Metadata/Metadata.bin");

	configuracion = config_create(ruta);

	tBloques = config_get_int_value(configuracion, "BLOCK_SIZE");
	cantBloques = config_get_int_value(configuracion, "BLOCKS");
	string_append(&magic_number, config_get_string_value(configuracion, "MAGIC_NUMBER"));

	log_info(alog, "Se lee la metadata del File System");

	if(strcmp(magic_number, "LISSANDRA"))
	{
		config_destroy(configuracion);
		log_info(alog, "No es LISSANDRA");
		free(ruta);
		return -1;
	}

	free(ruta);
	config_destroy(configuracion);
	return 0;

}

int abrir_bitmap()
{
	char *ruta = strdup("");

	string_append(&ruta,config->montaje);
	string_append(&ruta,"/Metadata/Bitmap.bin");

	int fdbitmap = open(ruta,O_RDWR);
	free(ruta);
	if(fdbitmap==0){
		log_info(alog,"no abrio el bitmap\n");
		close(fdbitmap);
		return -1;
	}

	fstat(fdbitmap,&mystat);

	posicion = (char *)mmap(0,mystat.st_size,PROT_READ|PROT_WRITE,MAP_SHARED,fdbitmap,0);
	if(posicion == MAP_FAILED){
		log_info(alog,"error en mmap\n");
		fprintf(stderr, "mmap failed: %s\n", strerror(errno));
		close(fdbitmap);
		return -1;
	}
	bitmap = bitarray_create_with_mode(posicion,mystat.st_size, LSB_FIRST);

	close(fdbitmap);

	log_info(alog, "Abre el bitmap");

	return 0;

}

void finalizar(){

	log_info(alog, "Esta por finalizar el File System");

	free(config->montaje);
	free(magic_number);
	free(config->puerto);
	free(config);
	if(bitm != -1){
		memcpy(posicion,bitmap,mystat.st_size);
		msync(posicion,mystat.st_size,MS_SYNC);
		munmap(posicion,mystat.st_size);
		bitarray_destroy(bitmap);
	}
	dictionary_clean(tablas);
	dictionary_destroy(tablas);

	dictionary_clean(clientes);
	dictionary_destroy(clientes);

	dictionary_clean(memtable);
	dictionary_destroy(memtable);
	liberar_log(alog);
}

void finalizarFile(){
	pthread_kill(hiloSelect,SIGKILL);
	pthread_kill(hiloConsola,SIGKILL);
	dictionary_iterator(clientes,(void*)cerrarClientes);
	dictionary_iterator(tablas,(void*)cerrarTablas);
	pthread_kill(hiloDump,SIGKILL);
}

void cerrarClientes(char * key, cliente_t * cliente){
	pthread_kill(cliente->hilo,SIGKILL);
}

void cerrarTablas(char * key, st_tablaCompac * tabla){
	pthread_kill(tabla->hilo,SIGKILL);
}

void senial(){
	pthread_exit(NULL);
}


