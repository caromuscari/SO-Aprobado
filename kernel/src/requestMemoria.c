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

int atenderResultadoSelect(st_messageResponse *mensaje,
        st_memoria *datoMemoria,
        st_instruccion *laInstruccion) {
    int resultado = NO_SALIO_OK;
    log_info(file_log,"[Request] Evaluando respuesta de SELECT");
    if (mensaje == NULL) {
        log_info(file_log,"[Request] Se Desconecto la Memoria");
        return SE_DESCONECTO_SOCKET;
    }
    switch (mensaje->cabezera.codigo) {
        case MEMORIAFULL: {
            log_info(file_log,"[Request] Full memoria haciendo journal");
            resultado = journalMemoria(datoMemoria);
            log_info(file_log,"[Request] Evaluando resultado de journal");
            if (resultado == SALIO_OK) {
                return enviarRequestMemoria(laInstruccion, datoMemoria);
            }
            break;
        }
        case SUCCESS: {
            st_registro *registro = deserealizarRegistro(mensaje->buffer);
            log_info(file_log,"[SELECT] key=[%d] value=[%s]",registro->key,registro->value);
            destroyRegistro(registro);
            resultado = SALIO_OK;
            break;
        }
        case NOSUCCESS: {
            resultado = NO_HAY_RESULTADO_EN_SELECT;
            log_info(file_log,"[SELECT] no se pudo encontra ese select");
            break;
        }
        default: {
            log_info(file_log,"[SELECT] no entiendo el codigo de respuesta");
        }
    }
    destroyStMessageResponse(mensaje);
    return resultado;
}

int atenderResultadoInsert(st_messageResponse *mensaje,
        st_memoria *datoMemoria,
        st_instruccion *laInstruccion) {
    int resultado = NO_SALIO_OK;
    log_info(file_log,"[Request] Evaluando respuesta de INSERT");
    if (mensaje == NULL) {
        log_info(file_log,"[Request] Se Desconecto la Memoria");
        return SE_DESCONECTO_SOCKET;
    }
    switch (mensaje->cabezera.codigo) {
        case MEMORIAFULL: {
            log_info(file_log,"[Request] Full memoria haciendo journal");
            resultado = journalMemoria(datoMemoria);
            log_info(file_log,"[Request] Evaluando resultado de journal");
            if (resultado == SALIO_OK) {
                return enviarRequestMemoria(laInstruccion, datoMemoria);
            }
            break;
        }
        case SUCCESS: {
            log_info(file_log,"[INSERT] se inserto correctamente");
            resultado = SALIO_OK;
            break;
        }
        case NOSUCCESS: {
            printf("[INSERT] no se pudo realizer el insert");
            log_info(file_log,"[INSERT] no se pudo realizer el insert");
            break;
        }
    }
    destroyStMessageResponse(mensaje);
    return resultado;
}

int atenderResultadoSDrop(st_messageResponse *mensaje, char *nameTable) {
    int resultado = NO_SALIO_OK;
    log_info(file_log,"[Request] Evaluando respuesta de DROP");
    if (mensaje == NULL) {
        log_info(file_log,"[Request] Se Desconecto la Memoria");
        return SE_DESCONECTO_SOCKET;
    }
    switch (mensaje->cabezera.codigo) {
        case SUCCESS: {
            log_info(file_log,"[DROP] se puedo eliminar la tabla sin problemas");
            removeTablaByName(nameTable);
            resultado = SALIO_OK;
            break;
        }
        case NOSUCCESS: {
            printf("[DROP] no se puedo eliminar la tabla\n");
            log_info(file_log,"[DROP] no se puedo eliminar la tabla");
            break;
        }

    }
    destroyStMessageResponse(mensaje);
    return resultado;
}

int atenderResultadoCreate(st_messageResponse *mensaje, st_create *_create) {
    int resultado = NO_SALIO_OK;
    log_info(file_log,"[Request] Evaluando respuesta de CREATE");
    if (mensaje == NULL) {
        log_info(file_log,"[Request] Se Desconecto la Memoria");
        return SE_DESCONECTO_SOCKET;
    }
    switch (mensaje->cabezera.codigo) {
        case SUCCESS: {
            log_info(file_log,"[CREATE] se creo la tabla sin problemas");
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
            printf("[CREATE] no se pudo crear la tabla\n");
            log_info(file_log,"[CREATE] no se pudo crear la tabla");
            break;
        }

    }
    destroyStMessageResponse(mensaje);
    return resultado;
}

int atenderResultadoDescribe(st_messageResponse *mensaje) {
    int resultado = NO_SALIO_OK;
    log_info(file_log,"[Request] Evaluando respuesta de DESCRIBE");
    if (mensaje == NULL) {
        log_info(file_log,"[Request] Se Desconecto la Memoria");
        return SE_DESCONECTO_SOCKET;
    }
    switch (mensaje->cabezera.codigo) {
        case SUCCESS: {
            size_t size;
            st_metadata *metadata = deserealizarMetaData(mensaje->buffer, &size);
            log_info(file_log,"[DESCRIBE] tabla = [%s]; Consistencia = [%s]",metadata->nameTable,metadata->consistency);
            addNuevaTabla(metadata);
            resultado = SALIO_OK;
            break;
        }
        case NOSUCCESS: {
            printf("[DESCRIBE] no hay respuesta para esta tabla\n");
            log_info(file_log,"[DESCRIBE] no hay respuesta para esta tabla");
            break;
        }
        default: {
            log_info(file_log,"[DESCRIBE] no entiendo el codigo de respuesta");
        }
    }
    destroyStMessageResponse(mensaje);
    return resultado;
}

int atenderResultadoDescribeGlobal(st_messageResponse *mensaje) {
    int resultado = NO_SALIO_OK;
    log_info(file_log,"[Request] Evaluando respuesta de DESCRIBE GLOBAL");

    if (mensaje == NULL) {
        log_info(file_log,"[Request] Se Desconecto la Memoria");
        return SE_DESCONECTO_SOCKET;
    }
    switch (mensaje->cabezera.codigo) {
        case SUCCESS: {
            int i;
            st_metadata * metadata;
            t_list * listMetadata = deserealizarListaMetaData(mensaje->buffer, mensaje->cabezera.sizeData);
            for (i = 0; i < listMetadata->elements_count; ++i) {
                metadata = list_get(listMetadata, i);
                log_info(file_log,"[DESCRIBE GLOBAL] tabla = [%s]; Consistencia = [%s]",metadata->nameTable,metadata->consistency);
            }
            updateListaMetadata(listMetadata);
            resultado = SALIO_OK;
            break;
        }
        case NOSUCCESS: {
            printf("[DESCRIBE GLOBAL] no hay respuesta para describe global \n");
            log_info(file_log,"[DESCRIBE GLOBAL] no hay respuesta para describe global");
            break;
        }
        default: {
            log_info(file_log,"[DESCRIBE GLOBAL] no entiendo el codigo de respuesta");
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
            log_info(file_log,"[Request] Serealizando SELECT");
            buffer = serealizarSelect(laInstruccion->instruccion, &size_buffer);
            log_info(file_log,"[Request] Enviando mensaje solicitando SELECT Memoria[%d]",datoMemoria->numero);
            resultado = atenderResultadoSelect(
                    consultarAMemoria(
                            datoMemoria->ip, datoMemoria->puerto, SELECT, buffer, size_buffer),
                            datoMemoria,laInstruccion);
            break;
        }
        case INSERT: {
            log_info(file_log,"[Request] Serealizando INSERT");
            ((st_insert *)laInstruccion->instruccion)->timestamp = obtenerMilisegundosDeHoy();
            buffer = serealizarInsert(laInstruccion->instruccion, &size_buffer);
            log_info(file_log,"[Request] Enviando mensaje INSERT Memoria[%d]", datoMemoria->numero);
            resultado = atenderResultadoInsert(
                    consultarAMemoria(datoMemoria->ip, datoMemoria->puerto, INSERT, buffer, size_buffer),
                    datoMemoria,
                    laInstruccion
            );
            break;
        }
        case DROP: {
            log_info(file_log,"[Request] Serealizando DROP");
            buffer = serealizarDrop(laInstruccion->instruccion, &size_buffer);
            log_info(file_log,"[Request] Enviando mensaje DROP Memoria[%d]", datoMemoria->numero);
            resultado = atenderResultadoSDrop(
                    consultarAMemoria(datoMemoria->ip, datoMemoria->puerto, DROP, buffer, size_buffer),
                    ((st_drop *) laInstruccion->instruccion)->nameTable
            );
            break;
        }
        case CREATE: {
            log_info(file_log,"[Request] Serealizando CREATE");
            buffer = serealizarCreate(laInstruccion->instruccion, &size_buffer);
            log_info(file_log,"[Request] Enviando mensaje CREATE Memoria[%d]",datoMemoria->numero);
            resultado = atenderResultadoCreate(
                    consultarAMemoria(datoMemoria->ip, datoMemoria->puerto, CREATE, buffer, size_buffer),
                    laInstruccion->instruccion
            );
            break;
        }
        case DESCRIBE: {
            log_info(file_log,"[Request] Serealizando DESCRIBE ");
            buffer = serealizarDescribe(laInstruccion->instruccion, &size_buffer);
            log_info(file_log,"[Request] Enviando mensaje CREATE Memoria[%d]",datoMemoria->numero);
            resultado = atenderResultadoDescribe(
                    consultarAMemoria(datoMemoria->ip, datoMemoria->puerto, DESCRIBE, buffer, size_buffer)
            );
            break;
        }
        case DESCRIBEGLOBAL: {
            buffer = strdup("1");
            size_buffer = strlen(buffer);
            log_info(file_log,"[Request] Enviando mensaje DESCRIBEGLOBAL Memoria[%d]",datoMemoria->numero);
            resultado = atenderResultadoDescribeGlobal(
                    consultarAMemoria(datoMemoria->ip, datoMemoria->puerto, DESCRIBEGLOBAL, buffer, size_buffer)
            );
            break;
        }
        default: {
            log_info(file_log,"[Request] NO se reconoce el API");
            break;
        }
    }
    if (resultado == SE_DESCONECTO_SOCKET) {
        eliminarMemoria(datoMemoria->numero);
    }
    free(buffer);
    return resultado;
}
