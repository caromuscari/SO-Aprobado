/*
 * buscar.c
 *
 *  Created on: 15 may. 2019
 *      Author: utnso
 */

#include "manejoArchivos.h"
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include "operaciones.h"
#include "buscar.h"
#include <dirent.h>
#include "Funciones.h"
#include "Semaforos.h"


char * buscarKey(char * name, int key, int particion){
	char * value;
	double time = 0;

	st_tabla * data;
	structRegistro * reg;

	char * path = armar_path(name);
	char * completo;

	if(existeEnMemtable(name)){
		data = leerDeMemtable(name);

		reg = buscarEnLista(data, key);

		if(reg != NULL){
			time = reg->time;
			value = strdup(reg->value);
			free(reg->value);
			free(reg);
		}
	}

	completo = string_from_format("%s/%d.bin", path, particion);
	reg = buscarEnParticion(completo,key);

	if(reg != NULL){
		if(time == 0 || time < reg->time){
			time = reg->time;
			value = strdup(reg->value);
		}

		free(reg->value);
		free(reg);
	}
	free(completo);

	reg = buscarEnTemporales(name,key);

	if(reg != NULL){
		if(time == 0 || time < reg->time){
			time = reg->time;
			value = strdup(reg->value);
		}

		free(reg->value);
		free(reg);
	}

	free(path);
	if(time != 0){
		char * devolver = string_from_format("%.0f;%d;%s", time, key, value);
		free(value);
		return devolver;
	}
	else return NULL;
}

structRegistro * buscarEnLista(st_tabla * data, uint16_t key){
	structRegistro * reg, *prueba;
	int size;
	int flag = 0;

	sem_wait(&data->semaforo);

	size = list_size(data->lista);
	for(int i = 0; i < size; i++){
		prueba = list_get(data->lista, i);
		if(prueba->key == key){
			if(flag == 0){
				reg = malloc(sizeof(structRegistro));
				reg->time = prueba->time;
				reg->key = prueba->key;
				reg->value = strdup(prueba->value);

				flag = 1;
			}else{
				if(reg->time < prueba->time){
					reg->time = prueba->time;
					reg->key = prueba->key;
					reg->value = strdup(prueba->value);
				}
			}
		}
	}

	sem_post(&data->semaforo);

	if(flag == 0) return NULL;
	else return reg;
}

structRegistro * buscarEnParticion(char * path, uint16_t key){
	structRegistro * reg, * prueba;
	structParticion * part;
	int i = 0, flag = 0;
	char * exep = NULL;

	part = leerParticion(path);

	if(part->Size != 0){
		while(part->bloques[i] != NULL && flag == 0){
			prueba = leerBloque(part->bloques[i], key, &exep);
			if(prueba != NULL){
				reg = malloc(sizeof(structRegistro));
				reg->time = prueba->time;
				reg->key = prueba->key;
				reg->value = strdup(prueba->value);

				flag = 1;

				free(prueba->value);
				free(prueba);
			}
			i++;
		}
	}

	string_iterate_lines(part->bloques, (void*)free);
	free(part->bloques);
	free(part);

	if(flag == 0) return NULL;
	else return reg;
}

structRegistro * buscarEnTemporales(char * name, uint16_t key){
	structRegistro * reg, *final;

	DIR *d;
	struct dirent *dir;
	char * path;
	int time = 0, flag = 0;
	char * nombre = armar_path(name);
    d = opendir(nombre);
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
        	char * name = strdup(dir->d_name);
        	if(string_ends_with(name,".tmp") || string_ends_with(name,".tmpc")){
        		path = string_from_format("%s/%s", nombre, name);
        		reg = buscarEnArchivo(path,key);

        		if(reg != NULL){
        			if(time == 0 || time < reg->time){
        				time = reg->time;
        				final = reg;
        			}
        			flag = 1;
        			/*free(reg->value);
        			free(reg);*/
        		}
        		free(path);
        	}

        	free(name);
        }
        closedir(d);
    }
    free(nombre);

	if(flag == 0) return NULL;
	else return final;
}

structRegistro * buscarEnArchivo(char * path, uint16_t key){
	structRegistro * reg, * prueba;
	structParticion * part;
	int i = 0, flag = 0;
	char * exep = NULL;

	part = leerParticion(path);

	while(part->bloques[i] != NULL){
		prueba = leerBloque(part->bloques[i], key, &exep);
		if(prueba != NULL){
			if(flag == 0){
				reg = malloc(sizeof(structRegistro));
				reg->time = prueba->time;
				reg->key = prueba->key;
				reg->value = strdup(prueba->value);

				flag = 1;
			}else{
				if(reg->time < prueba->time){
					reg->time = prueba->time;
					reg->key = prueba->key;
					reg->value = strdup(prueba->value);
				}
			}

			free(prueba->value);
			free(prueba);
		}
		i++;
	}

	string_iterate_lines(part->bloques, (void*)free);
	free(part->bloques);
	free(part);

	if(flag == 0)return NULL;
	else return reg;
}

