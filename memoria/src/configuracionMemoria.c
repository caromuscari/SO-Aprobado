#include "configuracionMemoria.h"

extern t_log *file_log;

t_configuracionMemoria configuracionMemoria;

t_configuracionMemoria leerConfiguracion(char* path){
	archivoConfiguracionMemoria = config_create(path);
	log_info(file_log, "Leyendo configuración");

	if (config_has_property(archivoConfiguracionMemoria, "PUERTO")) {
		configuracionMemoria.puerto = config_get_int_value(archivoConfiguracionMemoria,"PUERTO");
	} else {
		log_error(file_log,"No se encontro la key PUERTO en el archivo de configuracion");
		exit(EXIT_FAILURE);
	}
	if (config_has_property(archivoConfiguracionMemoria, "IP_FS")) {
		configuracionMemoria.ipFS = config_get_string_value(archivoConfiguracionMemoria,"IP_FS");
	} else {
		log_error(file_log,"No se encontro la key IP_FS en el archivo de configuracion");
		exit(EXIT_FAILURE);
	}
	if (config_has_property(archivoConfiguracionMemoria, "PUERTO_FS")) {
		configuracionMemoria.puertoFS = config_get_int_value(archivoConfiguracionMemoria,"PUERTO_FS");
	} else {
		log_error(file_log,"No se encontro la key PUERTO_FS en el archivo de configuracion");
		exit(EXIT_FAILURE);
		}
	if (config_has_property(archivoConfiguracionMemoria, "IP_SEEDS")) {
		configuracionMemoria.ipSeeds = config_get_array_value(archivoConfiguracionMemoria,"IP_SEEDS");
	} else {
		log_error(file_log,"No se encontro la key IP_SEEDS en el archivo de configuracion");
		exit(EXIT_FAILURE);
	}
	if (config_has_property(archivoConfiguracionMemoria, "PUERTO_SEEDS")) {
		char** arrayPuertos= config_get_array_value(archivoConfiguracionMemoria,"PUERTO_SEEDS");

		int cantPuertosSeeds = tamanioArray(arrayPuertos);
		char* ar [cantPuertosSeeds+1];
		int j =0;
		while(j<cantPuertosSeeds && arrayPuertos[j]!=NULL){
			ar[j]= arrayPuertos[j];
			j++;
		}
		for(int i =0; i<cantPuertosSeeds; i++){
			int a = atoi(ar[i]);
			configuracionMemoria.puertoSeeds[i]= a;
		}
		hacerFreeArray(arrayPuertos);
		free(arrayPuertos);
	} else {
		log_error(file_log,"No se encontro la key PUERTO_SEEDS en el archivo de configuracion");
		exit(EXIT_FAILURE);
	}
	if (config_has_property(archivoConfiguracionMemoria, "RETARDO_MEM")) {
		configuracionMemoria.retardoAccesoMP = config_get_int_value(archivoConfiguracionMemoria,"RETARDO_MEM");
	} else {
		log_error(file_log,"No se encontro la key RETARDO_MEM en el archivo de configuracion");
		exit(EXIT_FAILURE);
	}
	if (config_has_property(archivoConfiguracionMemoria, "RETARDO_FS")) {
			configuracionMemoria.retardoAccesoFS = config_get_int_value(archivoConfiguracionMemoria,"RETARDO_FS");
		} else {
			log_error(file_log,"No se encontro la key RETARDO_FS en el archivo de configuracion");
			exit(EXIT_FAILURE);
		}
	if (config_has_property(archivoConfiguracionMemoria, "TAM_MEM")) {
			configuracionMemoria.tamanioMemoria = config_get_int_value(archivoConfiguracionMemoria,"TAM_MEM");
		} else {
			log_error(file_log,"No se encontro la key TAM_MEM en el archivo de configuracion");
			exit(EXIT_FAILURE);
		}
	if (config_has_property(archivoConfiguracionMemoria, "TIEMPO_JOURNAL")) {
			configuracionMemoria.tiempoJournal = config_get_int_value(archivoConfiguracionMemoria,"TIEMPO_JOURNAL");
		} else {
			log_error(file_log,"No se encontro la key TIEMPO_JOURNAL en el archivo de configuracion");
			exit(EXIT_FAILURE);
		}
	if (config_has_property(archivoConfiguracionMemoria, "TIEMPO_GOSSIPING")) {
			configuracionMemoria.tiempoGossiping = config_get_int_value(archivoConfiguracionMemoria,"TIEMPO_GOSSIPING");
		} else {
			log_error(file_log,"No se encontro la key TIEMPO_GOSSIPING en el archivo de configuracion");
			exit(EXIT_FAILURE);
		}
	if (config_has_property(archivoConfiguracionMemoria, "NRO_MEMORIA")) {
			configuracionMemoria.nroMemoria = config_get_int_value(archivoConfiguracionMemoria,"NRO_MEMORIA");
		} else {
			log_error(file_log,"No se encontro la key NRO_MEMORIA en el archivo de configuracion");
			exit(EXIT_FAILURE);
		}

	return configuracionMemoria;

}

int tamanioArray(void** array){
	int i = 0;
	while(array[i]!=NULL){
		i++;
	}
	return i;
}

void hacerFreeArray(void** array){
	for(int i = 0; array[i]!= NULL; i++){
		free(array[i]);
	}
}
