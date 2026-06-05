#include "keypad.h"

char keys[4][4]={
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
};

void keypad_init(keypad_t* teclado){
    int PORT_SHIFT=0;
    if(teclado->puerto==GPIOA) PORT_SHIFT=2;
    else if(teclado->puerto==GPIOB) PORT_SHIFT=3;
    else if(teclado->puerto==GPIOC) PORT_SHIFT=4;
    else if(teclado->puerto==GPIOD) PORT_SHIFT=5;
    else if(teclado->puerto==GPIOE) PORT_SHIFT=6;

    /* HABILITO CLOCK PUERTO */
    RCC->APB2ENR|=(1<<PORT_SHIFT);
    
    for(int i=0;i<4;i++){
        /* FILAS COMO SALIDAS */
        if(teclado->fil[i]<8){ // pines [0:7] -> CRL
            teclado->puerto -> CRL &=~ (0xF<<(teclado->fil[i]*4)); // Borro estado de reset del registro (normal es 0x4 y si ponemos 0x1 queda cualquier cosa)
            teclado->puerto -> CRL |=  (0x1<<(teclado->fil[i]*4)); // 0x1: salida push pull
        } else{ // pines [8:15] -> CRH
            teclado->puerto -> CRH &=~ (0xF<<((teclado->fil[i]%8)*4)); // Borro estado de reset del registro (normal es 0x4 y si ponemos 0x1 queda cualquier cosa)
            teclado->puerto -> CRH |=  (0x1<<((teclado->fil[i]%8)*4)); // 0x1: salida push pull
        }
        /* COLUMNAS COMO ENTRADAS PULL UP */
        if(teclado->col[i]<8){ // pines [0:7] -> CRL
            teclado->puerto -> CRL &=~ (0xF<<(teclado->col[i]*4)); // Borro estado de reset del registro (normal es 0x4 y si ponemos 0x1 queda cualquier cosa)
            teclado->puerto -> CRL |=  (0x8<<(teclado->col[i]*4)); // 0x8: Entrada pull up o pull down
            teclado->puerto -> BSRR |= (1<<teclado->col[i]); // pongo el pin en 1 para seleccionar pull up
        } else{ // pines [8:15] -> CRH
            teclado->puerto -> CRH &=~ (0xF<<((teclado->col[i]%8)*4)); // Borro estado de reset del registro (normal es 0x4 y si ponemos 0x1 queda cualquier cosa)
            teclado->puerto -> CRH |=  (0x8<<((teclado->col[i]%8)*4)); // 0x8: Entrada pull up o pull down
            teclado->puerto -> BSRR |= (1<<teclado->col[i]); // pongo el pin en 1 para seleccionar pull up
        }
        /* FILAS EN 1 */
        teclado->puerto -> BSRR |= (1<<teclado->fil[i]);
    }
}

char keypad(keypad_t* teclado){

    /* ESCANEO FILAS: Apago una fila y paso a escanear la columna */
    for(int i=0;i<4;i++){
        teclado->puerto -> BSRR |= (1<<(teclado->fil[i]+16));
        /* ESCANEO COLUMNAS: veo si el 0 de la fila paso a la columna, si retorno el caracter presionado, sino paso a la otra */
        for(int j=0;j<4;j++){
            if(!(teclado->puerto -> IDR & (1<<teclado->col[i]))) return keys[i][j];
            //NO HAY ELSE
        }
        teclado->puerto -> BSRR |= (1<<teclado->fil[i]); //si no detecto ninguna columna en 0 prende la fila que apago y pasa a la otra
    }
    return 'x'; //si no detecto que se presionó nada retorna una x
}
