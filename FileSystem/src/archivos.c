/*
 * archivos.c
 *
 *  Created on: 18 may. 2019
 *      Author: utnso
 */
#include "operaciones.h"
#include <stdio.h>
#include "archivos.h"
#include <commons/bitarray.h>
#include "hiloConsola.h"

//extern int tBloques;
//extern t_bitarray* bitmap;
/*
int Insert(st_insert * insert){
	int respuesta;
	int particion;
	char * pathPart;
	char* path;
	st_metadata * metadata;
	structParticion * part;


	if(validarArchivos(insert->nameTable, &respuesta)){

		metadata = leerMetadata(insert->nameTable);

		particion = insert->key % metadata->partitions;
		path = armar_path(insert->nameTable);
		pathPart = string_from_format("%s/%d.bin",path , particion);

		part = leerParticion(pathPart);

		respuesta = escribir(particion,part, insert);

		liberarMetadata(metadata);
	}else respuesta = 6;

	return respuesta;
}


int escribir(int particion, structParticion * part, st_insert * insert){
	int i = 0;
	FILE * bloque;
	char * pathBloque;
	char * data;
	int bit;
	int respuesta;
	structActualizar * actualizar;

	while(part->bloques[i] != NULL) i++;

	if(tBloques * i - string_length(insert->value) <= part->Size){
		pathBloque = armar_PathBloque(part->bloques[i-1]);

		data = string_from_format("%i;%i;%s\n", insert->timestamp, insert->key, insert->value);

		bloque = fopen(pathBloque,"a");

		fwrite(data,sizeof(char),string_length(data),bloque);

		fclose(bloque);

		respuesta = 5;
	}else{
		bit = verificar_bloque();
		if(bit == -1)
		{
			respuesta = 11;
		}else{
			bitarray_set_bit(bitmap,bit);
			data = string_from_format("%i;%i;%s\n", insert->timestamp, insert->key, insert->value);

			actualizar = malloc(sizeof(structActualizar));

			actualizar->nameTable = insert->nameTable;
			actualizar->particion = particion;
			actualizar->bit = bit;
			actualizar->size = string_length(data);

			actualizar_Particion(actualizar);

			pathBloque = armar_PathBloque(string_itoa(bit));

			bloque = fopen(pathBloque,"a");

			fwrite(data,sizeof(char),string_length(data),bloque);

			fclose(bloque);

			respuesta = 5;
		}

	}

	return respuesta;
}
*/
