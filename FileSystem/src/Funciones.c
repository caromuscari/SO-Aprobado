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
#include <commons/collections/queue.h>
#include "hiloConsola.h"

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
extern int loop;
extern t_queue * nombre;
extern sem_t sMemtable, sTablas, sClientes, sNombre, sBitmap, sConfig;


void inicializar(){
	config = malloc(sizeof(structConfig));
	magic_number = strdup("");
	config->puerto = strdup("");
	//nombre = queue_create();
	loop = 1;
	clientes = dictionary_create();
	memtable = dictionary_create();
	tablas = dictionary_create();
	alog = crear_archivo_log("File System", false, "/home/utnso/lfs-base/log.txt");

	sem_init(&sMemtable,0,1);
	sem_init(&sTablas,0,1);
	sem_init(&sClientes,0,1);
	sem_init(&sNombre,0,1);
	sem_init(&sBitmap,0,1);
	sem_init(&sConfig,0,1);
}

void archivoDeConfiguracion(char* argv)
{
	t_config *configuracion;
	char * montaje = strdup("");
	configuracion = config_create(argv);
	string_append(&config->puerto, config_get_string_value(configuracion, "PUERTO_ESCUCHA​"));
	string_append(&montaje, config_get_string_value(configuracion, "PUNTO_MONTAJE"));
	config->retardo = config_get_int_value(configuracion, "RETARDO");
	config->tam_value = config_get_int_value(configuracion, "TAMAÑO_VALUE");
	config->tiempo_dump = config_get_int_value(configuracion, "TIEMPO_DUMP")/1000;

	log_info(alog, "Lee el archivo de configuracion");

	config->montaje = obtenerMontaje(montaje);

	free(montaje);
	config_destroy(configuracion);
}

char * obtenerMontaje(char * mont){
	char * respuesta;
	char ** split = string_split(mont,"\"");
	respuesta = string_substring_until(split[0],string_length(split[0])-1);
	string_iterate_lines(split, (void *) free);
	free(split);
	return respuesta;
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
	FILE * archivo;
	char * bytes;
	int tamBitmap;

	string_append(&ruta,config->montaje);
	string_append(&ruta,"/Metadata/Bitmap.bin");

	int fdbitmap = open(ruta,O_RDWR);

	if(fdbitmap == -1){
		log_info(alog,"Creacion del archivo bitmap\n");
		fdbitmap = open(ruta, O_CREAT|O_RDWR|O_TRUNC, S_IRUSR|S_IWUSR|S_IROTH|S_IWOTH|S_IRGRP|S_IWGRP);
		char * bloques = string_from_format("%s/Bloques", config->montaje);

		tamBitmap = cantBloques/8;

		for(int i=0; i< cantBloques; i++){
			char * file = string_from_format("%s/%d.bin", bloques, i);
			archivo = fopen(file,"a+");
			fclose(archivo);
			free(file);
		}

		ftruncate(fdbitmap, tamBitmap);
		//char * escribe = malloc(tamBitmap);
		//write(fdbitmap,escribe,tamBitmap);
		//free(escribe);

		bytes = (char *)mmap(0,tamBitmap,PROT_READ|PROT_WRITE,MAP_SHARED,fdbitmap,0);
		if(bytes == MAP_FAILED){
			log_info(alog,"error en mmap\n");
			fprintf(stderr, "mmap failed: %s\n", strerror(errno));
			close(fdbitmap);
			return -1;
		}

		memset(bytes,0,tamBitmap);
		msync(bytes,tamBitmap,MS_SYNC);
		munmap(bytes,tamBitmap);

		close(fdbitmap);

		free(bloques);

		fdbitmap = open(ruta, O_RDWR);
		if(fdbitmap == -1) return -1;

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

	//posicion = malloc(mystat.st_size);

	log_info(alog, "Abre el bitmap");

	free(ruta);

	return 0;

}

void finalizar(){

	log_info(alog, "Esta por finalizar el File System");

	free(config->montaje);
	free(magic_number);
	free(config->puerto);
	free(config);

	sem_destroy(&sMemtable);
	sem_destroy(&sTablas);
	sem_destroy(&sConfig);
	sem_destroy(&sNombre);
	sem_destroy(&sBitmap);
	sem_destroy(&sClientes);

	if(bitm != -1){
		//memcpy(posicion,bitmap,mystat.st_size);
		msync(posicion,mystat.st_size,MS_SYNC);
		munmap(posicion,mystat.st_size);
		bitarray_destroy(bitmap);
		//free(posicion);

		//queue_clean_and_destroy_elements(nombre, free);

		dictionary_clean_and_destroy_elements(tablas,(void*)liberarTablas);
		dictionary_destroy(tablas);

		dictionary_clean_and_destroy_elements(clientes,(void*)free);

		dictionary_clean_and_destroy_elements(memtable, (void*)limpiarMem);
	}

	dictionary_destroy(clientes);

	dictionary_destroy(memtable);
	liberar_log(alog);
}

void liberarTablas(st_tablaCompac * tabla){
	liberarMetadata(tabla->meta);
	sem_destroy(&tabla->compactacion);
	sem_destroy(&tabla->opcional);
	//list_destroy(tabla->sem);
	free(tabla);
}

void limpiarMem(st_tabla * tabla){
	sem_destroy(&tabla->semaforo);
	list_destroy_and_destroy_elements(tabla->lista, (void*)limpiarReg);
}

