#ifndef KEYPAD_H
#define KEYPAD_H

#include "stm32f103xb.h"

typedef struct{
    GPIO_TypeDef* puerto;
    int fil[4];
    int col[4];
} keypad_t;

void keypad_init(keypad_t*);
char keypad(keypad_t*);


#endif