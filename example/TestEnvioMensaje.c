#include <stdio.h>
#include <funcionesCompartidas/funcionesNET.h>
#include <funcionesCompartidas/codigoMensajes.h>
#include <funcionesCompartidas/listaMemoria.h>
#include <commons/log.h>

int main(){
    int control = 0;
    t_log *log = crear_archivo_log("Server", true, "./TestL");
    int client = establecerConexion("127.0.0.1","8000",log,&control);
    if(control != 0){
        log_error(log,"no se puedo establecer conexcion");
        return -1;
    }
    header request;
    request.codigo = BUSCARTABLAGOSSIPING;
    request.letra = 'M';
    request.sizeData = 0;
    void * paqueteAenviar = createMessage(&request,NULL);

    enviar_message(client,paqueteAenviar,log,&control);
    if(control != 0){
        log_error(log,"no se puedo enviar un mensaje");
        return -1;
    }
    free(paqueteAenviar);

    header response;
    void * paqueteResivido = getMessage(client,&response,&control);
    if(control < 0){
        log_error(log,"error al recibir el mensaje");
        return -1;
    }

    st_data_memoria * memoria = deserealizarMemoria(paqueteResivido,response.sizeData);

    printf("%d\n",memoria->numero);
}