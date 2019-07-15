//
// Created by miguelchauca on 14/07/19.
//

#include "requestMemoria.h"

extern t_log *file_log;

typedef struct {
    header cabezera;
    void *buffer;
} st_messageResponse;

void destroyStMessageResponse(st_messageResponse *stMR) {
    free(stMR->buffer);
    free(stMR);
}

st_messageResponse *consultarAMemoria(char *ip, char *puerto, int codigo, void *buffer, size_t sizeBuffer) {
    int control = 0;
    int socketClient = establecerConexion(ip, puerto, file_log, &control);
    if (control != 0) {
        log_error(file_log, "no se puedo conectar con la memoria");
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
        log_error(file_log, "no se puedo enviar el mensaje");
        return NULL;
    }

    header response;
    void *paqueteRespuesta = getMessage(socketClient, &response, &control);
    if (control < 0) {
        if (paqueteRespuesta) free(paqueteRespuesta);
        log_error(file_log, "no se puedo recibir el mensaje");
        return NULL;
    }
    st_messageResponse *messageResponse = malloc(sizeof(st_messageResponse));
    messageResponse->cabezera.letra = response.letra;
    messageResponse->cabezera.sizeData = response.sizeData;
    messageResponse->cabezera.codigo = response.codigo;
    messageResponse->buffer = paqueteRespuesta;
}

int atenderResultadoSelect(st_messageResponse *mensaje) {
    if (mensaje == NULL) {
        return -1;
    }
    if (mensaje->cabezera.codigo == 2) {
        printf("no se pudo encontra ese select\n");
        return -1;
    } else {
        st_registro *registro = deserealizarRegistro(mensaje->buffer);
        printf("resultado de consulta\n------------");
        printf("key [%d]\n", registro->key),
                printf("value [%s]\n", registro->value);
    }
    destroyStMessageResponse(mensaje);
    return 0;
}

int atenderResultadoInsert(st_messageResponse *mensaje, st_memoria *datoMemoria, stinstruccion *laInstruccion) {
    if (mensaje == NULL) {
        return -1;
    }
    if (mensaje->cabezera.codigo == 3) {
        log_info(file_log, "Memoria Full se hace journal");
        printf("Realizar journal\n");
        st_messageResponse *journalResponse = consultarAMemoria(datoMemoria->ip, datoMemoria->puerto, JOURNAL, NULL, 0);
        if (journalResponse->cabezera.codigo == 1) {
            log_info(file_log, "se relizo coreactamente el journal");
            return enviarRequestMemoria(laInstruccion, datoMemoria);
        } else {
            log_error(file_log, "no se pudo realizar el journal");
            return -1;
        }
    }
    if (mensaje->cabezera.codigo == 2) {
        printf("no se puedo relizar el insert\n");
        return -1;
    } else {
        printf("se relizo el insert todo ok\n");
    }
    destroyStMessageResponse(mensaje);
    return 0;
}

int atenderResultadoSDrop(st_messageResponse *mensaje, char *nameTable) {
    if (mensaje == NULL) {
        return -1;
    }
    if (mensaje->cabezera.codigo == 2) {
        printf("no se pudo encontra ese select\n");
        return -1;
    } else {
        removeTablaByName(nameTable);
    }
    destroyStMessageResponse(mensaje);
    return 0;
}

int enviarRequestMemoria(stinstruccion *laInstruccion, st_memoria *datoMemoria) {
    void *buffer = NULL;
    size_t size_buffer = 0;
    int resultado = -1;
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
                    consultarAMemoria(datoMemoria->ip, datoMemoria->puerto, INSERT, buffer, size_buffer),
                    ((st_drop *) laInstruccion->instruccion)->nameTable
            );
            break;

        }
        default: {
            break;
        }
    }
    free(buffer);
    return resultado;
}