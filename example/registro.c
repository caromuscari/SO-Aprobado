//
// Created by miguelchauca on 24/06/19.
//
#include <stdio.h>
#include <funcionesCompartidas/registroTabla.h>

void showRegistro(st_registro * data){
    printf("timestamp = %lf\n",data->timestamp);
    printf("key = %d\n",data->key);
    printf("value = %s\n",data->value);
}

int main(){
    char * miguel = "23434;234243;miguel";
    st_registro * registro = cargarRegistro(miguel);
    showRegistro(registro);
    size_t sizePaquete;
    void * paquete = serealizarRegistro(registro,&sizePaquete);
    st_registro * registroDeserealizado = deserealizarRegistro(paquete);
    showRegistro(registroDeserealizado);
    //limpiar
    destroyRegistro(registro);
    destroyRegistro(registroDeserealizado);
    free(paquete);

    return 0;
}
