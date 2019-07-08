//
// Created by miguelchauca on 16/06/19.
//

#include "time.h"

double obtenerMilisegundosDeHoy(){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    unsigned long long result = (((unsigned long long)tv.tv_sec) * 1000 + ((unsigned long long)tv.tv_usec)/1000);
    double a = result;
    return a;
}
