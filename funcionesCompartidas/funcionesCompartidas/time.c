//
// Created by miguelchauca on 16/06/19.
//

#include "time.h"

double obtenerMilisegundosDeHoy(){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec) * 1000 + (float)tv.tv_usec / 1000;
}
