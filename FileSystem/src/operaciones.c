/*
 * operaciones.c
 *
 *  Created on: 1 may. 2019
 *      Author: utnso
 */

#include "manejoArchivos.h"
#include <stdio.h>
#include "operaciones.h"
#include <commons/bitarray.h>
#include <commons/config.h>
#include "Funciones.h"
#include <sys/stat.h>
#include <sys/mman.h>
#include <dirent.h>
#include <funcionesCompartidas/listaMetadata.h>
#include "hiloCompactacion.h"
#include "buscar.h"
#include <commons/collections/queue.h>

extern structConfig * config;
extern t_bitarray* bitmap;
extern int cantBloques;
extern int tBloques;
extern t_queue * nombre;

extern char* posicion;
extern struct stat mystat;

st_metadata * leerMetadata(char * archivo){
	char *path;
	st_metadata * metadata = malloc(sizeof(st_metadata));
	t_config *configuracion;

	metadata->consistency = strdup("");

	path = armar_path(archivo);
	string_append(&path,"/Metadata");

	configuracion = config_create(path);

	metadata->nameTable = strdup(archivo);

	string_append(&metadata->consistency, config_get_string_value(configuracion, "CONSISTENCY"));
	metadata->partitions = config_get_int_value(configuracion, "PARTITIONS");
	metadata->compaction_time = config_get_int_value(configuracion, "COMPACTION_TIME");

	config_destroy(configuracion);
	free(path);

	return metadata;
}

int verificar_bloque()
{
	int bit = -1;
	bool comprobar = true;
	while(bit < cantBloques && comprobar == true)
	{
		bit++;
		comprobar = bitarray_test_bit(bitmap,bit);

	}
	if(bit == cantBloques && comprobar == false){
		return -1;
	}
	else {
		return bit;
	}

}

void actualizar_bitmap(){

	memcpy(posicion,bitmap,mystat.st_size);
	msync(posicion,mystat.st_size,MS_SYNC);

}

char * armar_path(char * archivo){
	char * path = string_new();
	string_append(&path,config->montaje);
	string_append(&path,"/Tables/");
	string_to_upper(archivo);
	string_append(&path,archivo);

	return path;
}

char * armar_PathBloque(char * bloque){
	char * path = string_new();
	string_append(&path,config->montaje);
	string_append(&path,"/Bloques/");
	string_append(&path,bloque);
	string_append(&path,".bin");

	return path;
}

bool crearMetadata(st_create * c, char * path){
	char * contenido;
	FILE * archivo;
	char * meta = string_new();

	contenido = string_from_format("CONSISTENCY=%s\nPARTITIONS=%d\nCOMPACTION_TIME=%d", c->tipoConsistencia,c->numeroParticiones,c->compactionTime);

	string_append(&meta,path);
	string_append(&meta,"/Metadata");
	archivo = fopen(meta,"a+");

	free(meta);

	if(archivo == NULL){
		free(contenido);
		return false;
	}else{
		fputs(contenido, archivo);
		free(contenido);
		fclose(archivo);
		return true;
	}
}

int crearParticiones(st_create * c, char * path){
	FILE * archivo;
	char * contenido;
	char * completo;
	int bit;
	int flag=-1;

	for (int i = 0; i < c->numeroParticiones; i++) {
		if(flag == -1){

			bit = verificar_bloque();
			if(bit == -1)
			{
				flag = i;

			}else{
				bitarray_set_bit(bitmap,bit);
				contenido = string_from_format("SIZE=0\nBLOQUES=[%d]", bit);

				completo = string_from_format("%s/%d.bin", path, i);
				archivo = fopen(completo, "a+");

				fputs(contenido,archivo);

				fclose(archivo);

				free(contenido);
				free(completo);
			}
		}
	}
	return flag;
}

void eliminarTemporales(char * path){
	FILE * archivo;
	structParticion * contenido;
	int i = 1;
	char * temp = string_from_format("%s/%d.tmp",path,i);

	archivo = fopen(temp, "r");

	while(archivo != NULL){
		fclose(archivo);

		contenido = leerParticion(temp);
		while(contenido->bloques[i] != NULL)
		{
			bitarray_clean_bit(bitmap,atoi(contenido->bloques[i]));
			i++;
		}
		remove(temp);

		free(temp);
		string_iterate_lines(contenido->bloques, (void*)free);
		free(contenido->bloques);
		free(contenido);


		i++;
		temp = string_from_format("%s/%d.tmp",path,i);
		archivo = fopen(temp, "r");
	}

	free(temp);
}

void eliminarDirectorio(char * path){
	char * meta = string_from_format("%s/Metadata", path);
	remove(meta);
	remove(path);
	free(meta);
}

void eliminarParticion(char *path, int particion){
	structParticion * contenido;
	char * part = string_from_format("%s/%d.bin", path, particion);
	int i = 0;

	contenido = leerParticion(part);

	while(contenido->bloques[i] != NULL)
	{
		bitarray_clean_bit(bitmap,atoi(contenido->bloques[i]));
		i++;
	}

	remove(part);

	free(part);
	string_iterate_lines(contenido->bloques, (void*)free);
	free(contenido->bloques);
	free(contenido);
}

structParticion * leerParticion(char * path){
	structParticion * contenido = malloc(sizeof(structParticion));
	t_config *config=config_create(path);

	contenido->Size = config_get_int_value(config,"SIZE");
	contenido->bloques = config_get_array_value(config,"BLOQUES");

	config_destroy(config);

	return contenido;
}

void actualizar_bloques(char * path,int bit){
	t_config *configuracion;
	char ** bloques;
	int i;

	configuracion = config_create(path);

	bloques = config_get_array_value(configuracion, "BLOQUES");
	while(bloques[i] != NULL) i++;
	bloques[i] = string_itoa(bit);
	bloques[i++] = NULL;

	config_get_string_value(configuracion, "BLOQUES");

	config_save(configuracion);

	config_destroy(configuracion);

	string_iterate_lines(bloques, (void*)free);
	free(bloques);
}

void actualizar_size(char * path,int size){
	t_config *configuracion;
	int sizeF;

	configuracion = config_create(path);

	sizeF = config_get_int_value(configuracion, "SIZE");
	config_set_value(configuracion, "SIZE", string_itoa(sizeF+size));

	config_save(configuracion);

	config_destroy(configuracion);
}

t_dictionary * listarDirectorio(){
	DIR *d;
	struct dirent *dir;
	t_dictionary * tablas;
	char * nombreTabla = string_from_format("%s/Tables", config->montaje);
    d = opendir(nombreTabla);
    if (d)
    {
    	tablas = dictionary_create();
        while ((dir = readdir(d)) != NULL)
        {
        	char * name = strdup(dir->d_name);

        	if(!string_equals_ignore_case(name,".") && !string_equals_ignore_case(name,"..")){
        		st_tablaCompac * tabla = malloc(sizeof(st_tablaCompac));
        		tabla->meta = leerMetadata(name);

        		sem_init(&tabla->compactacion,0,1);
        		sem_init(&tabla->opcional,0,0);
        		//tabla->sem = list_create();

        		char * table = strdup(name);
        		queue_push(nombre, table);
        		pthread_create(&tabla->hilo, NULL, (void*)hilocompactacion,NULL);
        		pthread_detach(tabla->hilo);

        		dictionary_put(tablas, name, tabla);
        	}

        	free(name);
        }
        closedir(d);
    }
    free(nombreTabla);

    return tablas;
}

structRegistro * leerBloque(char* bloque, uint16_t key, char ** exep){

	FILE * fbloque;
	char * linea;
	size_t tamBuffer = 100;
	char** split;
	structRegistro * reg;
	char * path;
	int flag = 0;
	char * value;

	path = armar_PathBloque(bloque);

	fbloque = fopen(path,"r");

	if(*exep != NULL){
		linea = malloc(sizeof(char) * tamBuffer);
		getline(&linea, &tamBuffer, fbloque);
		value = string_from_format("%s%s", *exep, linea);

		split = string_split(value,";");
		if(atoi(split[1]) == key){
			reg = malloc(sizeof(structRegistro));
			reg->time = atol(split[0]);
			reg->key = atoi(split[1]);
			reg->value = strdup(strtok(split[2], "\n"));

			flag = 1;
		}

		free(*exep);

		string_iterate_lines(split, (void*)free);
		free(split);
		free(linea);
	}

	linea = malloc(sizeof(char) * tamBuffer);
	while(getline(&linea, &tamBuffer, fbloque) != -1){
		split = string_split(linea,";");
		if(split[0] != NULL){
			if(split[1] == NULL) *exep = strdup(linea);
			else if(split[2] != NULL && string_contains(split[2], "\n")){
				if(atoi(split[1]) == key){
					if(flag == 0){
						reg = malloc(sizeof(structRegistro));
						reg->time = atol(split[0]);
						reg->key = atoi(split[1]);
						reg->value = strdup(strtok(split[2], "\n"));

						flag = 1;
					}else if(reg->time < atol(split[0])){
							reg->time = atol(split[0]);
							reg->key = atoi(split[1]);
							reg->value = strdup(strtok(split[2], "\n"));
						}
				}
			}else *exep = strdup(linea);
		}else if(!string_is_empty(linea)){
            *exep = strdup(linea);
		}


		string_iterate_lines(split, (void*)free);
		free(split);
		free(linea);

		linea = malloc(sizeof(char) * tamBuffer);
	}

	fclose(fbloque);
	free(path);
	free(linea);

	if(flag == 0) return NULL;
	else return reg;
}

bool chequearBitValido(int* bit){
    return *bit != -1;
}

void seteoBit(int* bit){
    bitarray_set_bit(bitmap,*bit);
}

char* armarStrBloques(char * strBloques, int *bit){
    if(!string_is_empty(strBloques)){
        string_append_with_format(&strBloques, ",%d", *bit);
    }else{
        string_append_with_format(&strBloques, "%d", *bit);
    }
    return strBloques;
}

t_list* crearArchivoTemporal(char * pathCompleto, size_t tamanio_size){
    FILE * archivo;
    char * contenido;
    t_list* bits = list_create();
    int tamanio = (int)tamanio_size;
    int tamanio_bloque = tBloques;
    while(tamanio > 0){
        int *bit = malloc(sizeof(int));
        *bit = verificar_bloque();
        list_add(bits, bit);
        tamanio -= tamanio_bloque;
    }

    bool valido = list_all_satisfy(bits, (void*)chequearBitValido);


    if(valido){
        list_iterate(bits, (void*) seteoBit);
        char* strBloques = list_fold(bits, string_new(), (void*)armarStrBloques);
        contenido = string_from_format("SIZE=%d\nBLOQUES=[%s]", tamanio_size, strBloques);

        archivo = fopen(pathCompleto, "a+");

        fputs(contenido,archivo);

        fclose(archivo);

        free(contenido);
    }


    return bits;
}
