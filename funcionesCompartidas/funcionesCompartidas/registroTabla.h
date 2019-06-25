//
// Created by miguelchauca on 23/06/19.
//

#ifndef KERNEL_REGISTROTABLA_H
#define KERNEL_REGISTROTABLA_H

 #include <commons/string.h>
 #include <stdlib.h>
 #include <string.h>
 #include <stdint.h>

 typedef struct {
     uint16_t key;
     double timestamp;
     char *value;
 } st_registro;

 typedef struct {
     size_t key;
     size_t timestamp;
     size_t value;
 } __attribute__((packed)) st_size_registro;

 st_registro *cargarRegistro(char *text);
 void *serealizarRegistro(st_registro *registro, size_t *sizeBUffer);
 st_registro * deserealizarRegistro(void * buffer);
 void destroyRegistro(st_registro * registro);

#endif //KERNEL_REGISTROTABLA_H
