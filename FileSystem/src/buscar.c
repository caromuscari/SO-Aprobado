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

extern t_dictionary * memtable;

char * buscarKey(char * name, int key, int particion){
	char * value;
	long int time = 0;

	st_tabla * data;
	structRegistro * reg;

	char * path = armar_path(name);
	char * completo;

	if(dictionary_has_key(memtable,name)){
		data = dictionary_get(memtable, name);

		reg = buscarEnLista(data, key);

		if(reg != NULL){
			time = reg->time;
			value = strdup(reg->value);
			free(reg);
		}
	}

	completo = string_from_format("%s/%d.bin", path, particion);
	reg = buscarEnParticion(completo,key);

	if(reg != NULL){
		if(time < reg->time){
			time = reg->time;
			value = strdup(reg->value);
		}

		free(reg);
	}
	free(completo);

	reg = buscarEnTemporales(name,key);

	if(reg != NULL){
		if(time < reg->time){
			time = reg->time;
			value = strdup(reg->value);
		}

		free(reg);
	}

	free(path);
	return value;
}

structRegistro * buscarEnLista(st_tabla * data, uint16_t key){
	structRegistro * reg, *prueba;
	int size;

	sem_wait(&data->semaforo);

	size = list_size(data->lista);
	for(int i = 0; i < size; i++){
		prueba = list_get(data->lista, i);
		if(prueba->key == key){
			if(reg == NULL){
				reg = malloc(sizeof(structRegistro));
				reg->time = prueba->time;
				reg->key = prueba->key;
				reg->value = prueba->value;
			}else{
				if(reg->time < prueba->time){
					reg->time = prueba->time;
					reg->key = prueba->key;
					reg->value = prueba->value;
				}
			}
		}
		//free(prueba); No se si va
	}

	sem_post(&data->semaforo);


	return reg;
}

structRegistro * buscarEnParticion(char * path, uint16_t key){
	structRegistro * reg, * prueba;
	structParticion * part;
	int i = 0, flag = 0;

	part = leerParticion(path);

	while(part->bloques[i] != NULL && flag == 0){
		prueba = leerBloque(part->bloques[i], key);
		if(prueba != NULL){
			reg = malloc(sizeof(structRegistro));
			reg->time = prueba->time;
			reg->key = prueba->key;
			reg->value = prueba->value;

			flag = 1;

			free(prueba->value);
			free(prueba);
		}
	}

	string_iterate_lines(part->bloques, (void*)free);
	free(part->bloques);
	free(part);

	return reg;
}

structRegistro * buscarEnTemporales(char * name, uint16_t key){
	structRegistro * reg, *final;

	DIR *d;
	struct dirent *dir;
	char * path;
	int time = 0;
	char * nombre = armar_path(name);
    d = opendir(nombre);
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
        	char * name = strdup(dir->d_name);
        	if(string_ends_with(name,".tmp")){
        		path = string_from_format("%s/%s", nombre, name);
        		reg = buscarEnArchivo(path,key);

        		if(reg != NULL){
        			if(time < reg->time){
        				time = reg->time;
        				final = reg;
        			}
        			free(reg->value);
        			free(reg);
        		}
        		free(path);
        	}

        	free(name);
        }
        closedir(d);
    }
    free(nombre);

	return final;
}

structRegistro * buscarEnArchivo(char * path, uint16_t key){
	structRegistro * reg, * prueba;
	structParticion * part;
	int i = 0;

	part = leerParticion(path);

	while(part->bloques[i] != NULL){
		prueba = leerBloque(part->bloques[i], key);
		if(prueba != NULL){
			if(reg == NULL){
				reg = malloc(sizeof(structRegistro));
				reg->time = prueba->time;
				reg->key = prueba->key;
				reg->value = prueba->value;
			}else{
				if(reg->time < prueba->time){
					reg->time = prueba->time;
					reg->key = prueba->key;
					reg->value = prueba->value;
				}
			}

			free(prueba->value);
			free(prueba);
		}
	}

	string_iterate_lines(part->bloques, (void*)free);
	free(part->bloques);
	free(part);

	return reg;
}


