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
    if(mensaje == NULL){
        return -1;
    }
    st_registro * registro = deserealizarRegistro(mensaje->buffer);
    printf("resultado de consulta\n------------");
    printf("key [%d]\n",registro->key),
    printf("value [%s]\n",registro->value);
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
                    consultarAMemoria(datoMemoria->ip, datoMemoria->puerto, SELECT, buffer, size_buffer));
            break;
        }
        default: {
            break;
        }
    }
    free(buffer);
    return resultado;
}