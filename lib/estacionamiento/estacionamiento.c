#include "estacionamiento.h"

volatile uint16_t autos_totales=0; //el 16 asi da mas de 65 mil autos
estacionamiento_t *aux;

void estacionamiento_init(estacionamiento_t *parking){
    aux = parking;
    int PORT_SHIFT=0, EXTICR_SHIFT[2]={0,0},EXTICR_PORT_SHIFT[2]={0x0,0x0};
    uint16_t INTERRUPT_SHIFT;
    if(parking->puerto==GPIOA){      PORT_SHIFT=2; EXTICR_PORT_SHIFT[0]=0x0; EXTICR_PORT_SHIFT[1]=0x0;}
    else if(parking->puerto==GPIOB) {PORT_SHIFT=3; EXTICR_PORT_SHIFT[0]=0x1; EXTICR_PORT_SHIFT[1]=0x1;}
    else if(parking->puerto==GPIOC) {PORT_SHIFT=4; EXTICR_PORT_SHIFT[0]=0x2; EXTICR_PORT_SHIFT[1]=0x2;}
    else if(parking->puerto==GPIOD) {PORT_SHIFT=5; EXTICR_PORT_SHIFT[0]=0x3; EXTICR_PORT_SHIFT[1]=0x3;}
    else if(parking->puerto==GPIOE) {PORT_SHIFT=6; EXTICR_PORT_SHIFT[0]=0x4; EXTICR_PORT_SHIFT[1]=0x4;}
    RCC->APB2ENR|=(1<<PORT_SHIFT)|RCC_APB2ENR_AFIOEN;
    for(int i=0; i<2;i++){
        /* SENSORES COMO INTERRUPCION */
        if(parking->sensores[i]<8){
            parking->puerto -> CRL  &=~ (0xF<<(parking->sensores[i]*4));
            parking->puerto -> CRL  |=  (0x8<<(parking->sensores[i]*4));
            parking->puerto -> BSRR |=  (1  <<(parking->sensores[i]+16));
        } else{
            parking->puerto -> CRH  &=~ (0xF<<((parking->sensores[i]%8)*4));
            parking->puerto -> CRH  |=  (0x8<<((parking->sensores[i]%8)*4));
            parking->puerto -> BSRR |=  (1  <<( parking->sensores[i]+16));
        }
        /* VEO QUE PIN ESTOY USANDO PARA EL NUMERO EN EXTICR */
        if(     (parking->sensores[i]>=0) && (parking->sensores[i]<=3))   EXTICR_SHIFT[i]=0;
        else if((parking->sensores[i]>=4) && (parking->sensores[i]<=7))   EXTICR_SHIFT[i]=1;
        else if((parking->sensores[i]>=8) && (parking->sensores[i]<=11))  EXTICR_SHIFT[i]=2;
        else if((parking->sensores[i]>=12) && (parking->sensores[i]<=15)) EXTICR_SHIFT[i]=3;

        AFIO->EXTICR[EXTICR_SHIFT[i]]|=(EXTICR_PORT_SHIFT[i]<<((parking->sensores[i]%4)*4));
        EXTI->IMR |= (1<<(parking->sensores[i]));
        EXTI->RTSR|= (1<<(parking->sensores[i]));
        EXTI->FTSR&=~(1<<(parking->sensores[i]));

        if     (parking->sensores[i]==0) INTERRUPT_SHIFT=EXTI0_IRQn;
        else if(parking->sensores[i]==1) INTERRUPT_SHIFT=EXTI1_IRQn;
        else if(parking->sensores[i]==2) INTERRUPT_SHIFT=EXTI2_IRQn;
        else if(parking->sensores[i]==3) INTERRUPT_SHIFT=EXTI3_IRQn;
        else if(parking->sensores[i]==4) INTERRUPT_SHIFT=EXTI4_IRQn;
        else if((parking->sensores[i]>=5 )&&(parking->sensores[i]<=9 )) INTERRUPT_SHIFT=EXTI9_5_IRQn;
        else if((parking->sensores[i]>=10)&&(parking->sensores[i]<=15)) INTERRUPT_SHIFT=EXTI15_10_IRQn;

        NVIC_EnableIRQ(INTERRUPT_SHIFT);
        NVIC_SetPriority(INTERRUPT_SHIFT,1);


        /* SEMAFOROS COMO SALIDAS PUSH PULL */
        if(parking->semaforos[i]<8){
            parking->puerto -> CRL  &=~ (0xF<<(parking->semaforos[i]*4));
            parking->puerto -> CRL  |=  (0x8<<(parking->semaforos[i]*4));
            parking->puerto -> BSRR |=  (1  <<(parking->semaforos[i]+16));
        } else{
            parking->puerto -> CRH  &=~ (0xF<<((parking->semaforos[i]%8)*4));
            parking->puerto -> CRH  |=  (0x8<<((parking->semaforos[i]%8)*4));
            parking->puerto -> BSRR |=  (1  <<( parking->semaforos[i]+16));
        }
    }

    /* INICIALIZO KEYPAD */
    keypad_t teclado;
    for(int i=0;i<4;i++){
        teclado.col[i]=parking->columnas[i];
        teclado.fil[i]=parking->filas[i];
    }
    keypad_init(&teclado);

}

void estacionamiento(estacionamiento_t *parking){
    char modo = tolower(parking->modo);
    switch(modo){
        case 'n':
            normal(&parking);
        break;
        case 'r':
            restringido(&parking);
        break;
        case 'l':
            libre(&parking);
        break;
    }
}

/* FUNCIONES INTERNAS */

void normal(estacionamiento_t *parking){
    int cont=0,numero_ingresado[3]={0,0,0};
    uint16_t numero_final;
    /* LEO KEYPAD */
    keypad_t teclado;
    for(int i=0;i<4;i++){
        teclado.col[i]=parking->columnas[i];
        teclado.fil[i]=parking->filas[i];
    }
    while(cont<3){
        if(keypad(&teclado)!='x'){
            numero_ingresado[cont] = ((int)keypad(&teclado)-48);
            cont++;
        }
    }
    for(int i=0;i<3;i++){
        numero_final=numero_ingresado[0]*100+numero_ingresado[1]*10+numero_ingresado[2];
    }
    /* ME FIJO SI SUPERO A LA TOTALIDAD Y SETEO EL SEMAFORO */
    if(autos_totales>=numero_final) {
        parking->puerto -> BSRR |= (1<<parking->semaforos[ROJO]);
        parking->puerto -> BSRR |= (1<<(parking->semaforos[VERDE]+16));
    } else{
        parking->puerto -> BSRR |= (1<<(parking->semaforos[ROJO]+16));
        parking->puerto -> BSRR |= (1<<parking->semaforos[VERDE]);
    }
}
void restringido(estacionamiento_t *parking){
    parking->puerto -> BSRR |= (1<<parking->semaforos[ROJO]);
    parking->puerto -> BSRR |= (1<<(parking->semaforos[VERDE]+16));
}

void libre(estacionamiento_t *parking){
    parking->puerto -> BSRR |= (1<<(parking->semaforos[ROJO]+16));
    parking->puerto -> BSRR |= (1<<parking->semaforos[VERDE]);
}

/* SENSORES COMO INTERRUPCION PARA CUALQUIER EXTI */

void EXTI0IRQHandler(){
    if(EXTI->PR&(1<<0)){
        EXTI->PR|=(1<<0);
        if(aux->sensores[0]==0) autos_totales++; //pregunto si es una entrada
        else autos_totales--;
    }
}
void EXTI1IRQHandler(){
    if(EXTI->PR&(1<<1)){
        EXTI->PR|=(1<<1);
        if(aux->sensores[0]==1) autos_totales++; //pregunto si es una entrada
        else autos_totales--;
    }
}
void EXTI2IRQHandler(){
    if(EXTI->PR&(1<<2)){
        EXTI->PR|=(1<<2);
        if(aux->sensores[0]==2) autos_totales++; //pregunto si es una entrada
        else autos_totales--;
    }
}
void EXTI3IRQHandler(){
    if(EXTI->PR&(1<<3)){
        EXTI->PR|=(1<<3);
        if(aux->sensores[0]==3) autos_totales++; //pregunto si es una entrada
        else autos_totales--;
    }
}
void EXTI4IRQHandler(){
    if(EXTI->PR&(1<<4)){
        EXTI->PR|=(1<<4);
        if(aux->sensores[0]==4) autos_totales++; //pregunto si es una entrada
        else autos_totales--;
    }
}
void EXTI9_5IRQHandler(){
    if(EXTI->PR&(1<<5)){
        EXTI->PR|=(1<<5);
        if(aux->sensores[0]==5) autos_totales++; //pregunto si es una entrada
        else autos_totales--;
    } else if(EXTI->PR&(1<<6)){
        EXTI->PR|=(1<<6);
        if(aux->sensores[0]==6) autos_totales++; //pregunto si es una entrada
        else autos_totales--;
    } else if(EXTI->PR&(1<<7)){
        EXTI->PR|=(1<<7);
        if(aux->sensores[0]==7) autos_totales++; //pregunto si es una entrada
        else autos_totales--;
    }
     else if(EXTI->PR&(1<<8)){
        EXTI->PR|=(1<<8);
        if(aux->sensores[0]==8) autos_totales++; //pregunto si es una entrada
        else autos_totales--;
    }  else if(EXTI->PR&(1<<9)){
        EXTI->PR|=(1<<9);
        if(aux->sensores[0]==9) autos_totales++; //pregunto si es una entrada
        else autos_totales--;
    }
}
void EXTI15_10IRQHandler(){
    if(EXTI->PR&(1<<10)){
        EXTI->PR|=(1<<10);
        if(aux->sensores[0]==10) autos_totales++; //pregunto si es una entrada
        else autos_totales--;
    } else if(EXTI->PR&(1<<11)){
        EXTI->PR|=(1<<11);
        if(aux->sensores[0]==11) autos_totales++; //pregunto si es una entrada
        else autos_totales--;
    } else if(EXTI->PR&(1<<12)){
        EXTI->PR|=(1<<12);
        if(aux->sensores[0]==12) autos_totales++; //pregunto si es una entrada
        else autos_totales--;
    } else if(EXTI->PR&(1<<13)){
        EXTI->PR|=(1<<13);
        if(aux->sensores[0]==13) autos_totales++; //pregunto si es una entrada
        else autos_totales--;
    } else if(EXTI->PR&(1<<14)){
        EXTI->PR|=(1<<14);
        if(aux->sensores[0]==14) autos_totales++; //pregunto si es una entrada
        else autos_totales--;
    } else if(EXTI->PR&(1<<15)){
        EXTI->PR|=(1<<15);
        if(aux->sensores[0]==15) autos_totales++; //pregunto si es una entrada
        else autos_totales--;
    } 
}