//
// Created by miguelchauca on 14/07/19.
//

#include "requestMemoria.h"

extern t_log *file_log;

void destroyStMessageResponse(st_messageResponse *stMR) {
    free(stMR->buffer);
    free(stMR);
}

st_messageResponse *consultarAMemoria(char *ip, char *puerto, int codigo, void *buffer, size_t sizeBuffer) {
    int control = 0;
    int socketClient = establecerConexion(ip, puerto, file_log, &control);
    if (control != 0) {
        log_error(file_log, "no se pudo conectar con la memoria");
        return NULL;
    }
    header request;
    request.letra = 'K';
    request.codigo = codigo;
    request.sizeData = sizeBuffer;
    message *paqueteMensaje = createMessage(&request, buffer);
    enviar_message(socketClient, paqueteMensaje, file_log, &control);
    free(paqueteMensaje->buffer);
    free(paqueteMensaje);
    if (control != 0) {
        log_error(file_log, "no se pudo enviar el mensaje");
        return NULL;
    }

    header response;
    void *paqueteRespuesta = getMessage(socketClient, &response, &control);
    if (paqueteRespuesta == NULL) {
        log_error(file_log, "no se pudo recibir el mensaje");
        return NULL;
    }
    st_messageResponse *messageResponse = malloc(sizeof(st_messageResponse));
    messageResponse->cabezera.letra = response.letra;
    messageResponse->cabezera.sizeData = response.sizeData;
    messageResponse->cabezera.codigo = response.codigo;
    messageResponse->buffer = paqueteRespuesta;

    close(socketClient);
    return messageResponse;
}

int atenderResultadoSelect(st_messageResponse *mensaje) {
    int resultado = NO_SALIO_OK;
    if (mensaje == NULL) {
        return SE_DESCONECTO_SOCKET;
    }
    switch (mensaje->cabezera.codigo) {
        case SUCCESS: {
            st_registro *registro = deserealizarRegistro(mensaje->buffer);
            printf("********----resultado de consulta-------******\n");
            printf("key [%d]\n", registro->key),
                    printf("value [%s]\n", registro->value);
            destroyRegistro(registro);
            resultado = SALIO_OK;
            break;
        }
        case NOSUCCESS: {
            resultado = SALIO_OK;
            printf("no se pudo encontra ese select\n");
            break;
        }
        default: {
            printf("no entiendo el codigo de respuesta\n");
        }
    }
    destroyStMessageResponse(mensaje);
    return resultado;
}

int atenderResultadoInsert(st_messageResponse *mensaje, st_memoria *datoMemoria, st_instruccion *laInstruccion) {
    int resultado = NO_SALIO_OK;
    if (mensaje == NULL) {
        return SE_DESCONECTO_SOCKET;
    }
    switch (mensaje->cabezera.codigo) {
        case MEMORIAFULL: {
            resultado = journalMemoria(datoMemoria);
            if (resultado == SALIO_OK) {
                return enviarRequestMemoria(laInstruccion, datoMemoria);
            }
            break;
        }
        case SUCCESS: {
            printf("se relizo el insert todo ok\n");
            resultado = SALIO_OK;
            break;
        }
        case NOSUCCESS: {
            printf("no se puedo relizar el insert\n");
            break;
        }
    }
    destroyStMessageResponse(mensaje);
    return resultado;
}

int atenderResultadoSDrop(st_messageResponse *mensaje, char *nameTable) {
    int resultado = NO_SALIO_OK;
    if (mensaje == NULL) {
        return SE_DESCONECTO_SOCKET;
    }
    switch (mensaje->cabezera.codigo) {
        case SUCCESS: {
            printf("se puedo eliminar la tabla sin problemas\n");
            removeTablaByName(nameTable);
            resultado = SALIO_OK;
            break;
        }
        case NOSUCCESS: {
            printf("no se puedo eliminar la tabla\n");
            break;
        }

    }
    destroyStMessageResponse(mensaje);
    return resultado;
}

int atenderResultadoCreate(st_messageResponse *mensaje, st_create *_create) {
    int resultado = NO_SALIO_OK;
    if (mensaje == NULL) {
        return SE_DESCONECTO_SOCKET;
    }
    switch (mensaje->cabezera.codigo) {
        case SUCCESS: {
            printf("se creo la tabla sin problemas\n");
            st_metadata *newMetadata = malloc(sizeof(st_metadata));
            newMetadata->nameTable = strdup(_create->nameTable);
            newMetadata->consistency = strdup(_create->tipoConsistencia);
            newMetadata->compaction_time = _create->compactionTime;
            newMetadata->partitions = _create->numeroParticiones;
            addNuevaTabla(newMetadata);
            resultado = SALIO_OK;
            break;
        }
        case NOSUCCESS: {
            printf("no se pudo crear la tabla\n");
            break;
        }

    }
    destroyStMessageResponse(mensaje);
    return resultado;
}

int atenderResultadoDescribe(st_messageResponse *mensaje) {
    int resultado = NO_SALIO_OK;
    if (mensaje == NULL) {
        return SE_DESCONECTO_SOCKET;
    }
    switch (mensaje->cabezera.codigo) {
        case SUCCESS: {
            size_t size;
            st_metadata *metadata = deserealizarMetaData(mensaje->buffer, &size);
            printf("********----resultado de Describe -------******\n");
            printf("nameTable [%s]\n", metadata->nameTable),
            printf("Consistencia [%s]\n", metadata->consistency);
            addNuevaTabla(metadata);
            resultado = SALIO_OK;
            break;
        }
        case NOSUCCESS: {
            printf("no hay respuesta para esta tabla\n");
            break;
        }
        default: {
            printf("no entiendo el codigo de respuesta\n");
        }
    }
    destroyStMessageResponse(mensaje);
    return resultado;
}

int atenderResultadoDescribeGlobal(st_messageResponse *mensaje) {
    int resultado = NO_SALIO_OK;
    if (mensaje == NULL) {
        return SE_DESCONECTO_SOCKET;
    }
    switch (mensaje->cabezera.codigo) {
        case SUCCESS: {
            int i;
            st_metadata * metadata;
            t_list * listMetadata = deserealizarListaMetaData(mensaje->buffer, mensaje->cabezera.sizeData);
            for (i = 0; i < listMetadata->elements_count; ++i) {
                metadata = list_get(listMetadata, i);
                printf("********----resultado de Describe -------******\n");
                printf("nameTable [%s]\n", metadata->nameTable),
                printf("Consistencia [%s]\n", metadata->consistency);
            }
            updateListaMetadata(listMetadata);
            resultado = SALIO_OK;
            break;
        }
        case NOSUCCESS: {
            printf("no hay respuesta para describe global \n");
            break;
        }
        default: {
            printf("no entiendo el codigo de respuesta\n");
        }
    }
    destroyStMessageResponse(mensaje);
    return resultado;
}

int enviarRequestMemoria(st_instruccion *laInstruccion, st_memoria *datoMemoria) {
    void *buffer = NULL;
    size_t size_buffer = 0;
    int resultado = NO_SALIO_OK;
    switch (laInstruccion->operacion) {
        case SELECT: {
            buffer = serealizarSelect(laInstruccion->instruccion, &size_buffer);
            resultado = atenderResultadoSelect(
                    consultarAMemoria(
                            datoMemoria->ip, datoMemoria->puerto, SELECT, buffer, size_buffer));
            break;
        }
        case INSERT: {
            buffer = serealizarInsert(laInstruccion->instruccion, &size_buffer);
            resultado = atenderResultadoInsert(
                    consultarAMemoria(datoMemoria->ip, datoMemoria->puerto, INSERT, buffer, size_buffer),
                    datoMemoria,
                    laInstruccion
            );
            break;
        }
        case DROP: {
            buffer = serealizarDrop(laInstruccion->instruccion, &size_buffer);
            resultado = atenderResultadoSDrop(
                    consultarAMemoria(datoMemoria->ip, datoMemoria->puerto, DROP, buffer, size_buffer),
                    ((st_drop *) laInstruccion->instruccion)->nameTable
            );
            break;
        }
        case CREATE: {
            buffer = serealizarCreate(laInstruccion->instruccion, &size_buffer);
            resultado = atenderResultadoCreate(
                    consultarAMemoria(datoMemoria->ip, datoMemoria->puerto, CREATE, buffer, size_buffer),
                    laInstruccion->instruccion
            );
            break;
        }
        case DESCRIBE: {
            buffer = serealizarDescribe(laInstruccion->instruccion, &size_buffer);
            resultado = atenderResultadoDescribe(
                    consultarAMemoria(datoMemoria->ip, datoMemoria->puerto, DESCRIBE, buffer, size_buffer)
            );
            break;
        }
        case DESCRIBEGLOBAL: {
            buffer = strdup("1");
            size_buffer = strlen(buffer);
            resultado = atenderResultadoDescribeGlobal(
                    consultarAMemoria(datoMemoria->ip, datoMemoria->puerto, DESCRIBEGLOBAL, buffer, size_buffer)
            );
            break;
        }
        default: {
            break;
        }
    }
    if (resultado == SE_DESCONECTO_SOCKET) {
        eliminarMemoria(datoMemoria->numero);
    }
    free(buffer);
    destroyMemoria(datoMemoria);
    return resultado;
}
