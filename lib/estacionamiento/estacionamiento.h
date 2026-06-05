#ifndef ESTACIONAMIENTO_H
#define ESTACIONAMIENTO_H

#include "stm32f103xb.h"
#include "keypad.h"
#include "ctype.h"

#define ROJO 0
#define VERDE 1
#define ENTRADA 0
#define SALIDA 1


typedef struct{
    GPIO_TypeDef* puerto;
    int sensores[2]; //0:entrada ; 1:salida
    int semaforos[2];//0:rojo ; 1:verde
    int filas[4];
    int columnas[4];
    char modo; // 'n' normal ; 'r' restringido ; 'l' libre
} estacionamiento_t;

void estacionamiento_init(estacionamiento_t*);
void estacionamiento(estacionamiento_t*);

/* FUNCIONES INTERNAS */
void normal(estacionamiento_t*);
void restringido(estacionamiento_t*);
void libre(estacionamiento_t*);
#endif
