#include "estacionamiento.h"

#define NORMAL 0
#define RESTRINGIDO 1
#define LIBRE 2

estacionamiento_t parking_castelar;

int main(void){

    int botones[3]={NORMAL,RESTRINGIDO,LIBRE};
    parking_castelar.puerto = GPIOA;
    for(int i=0;i<4;i++){
        parking_castelar.filas[i]=i;
    }
    for(int i=4;i<8;i++){
        parking_castelar.columnas[i%2]=i;
    }
    for(int i=8;i<10;i++){
        parking_castelar.semaforos[i%2]=i;
    }
    for(int i=8;i<10;i++){
        parking_castelar.sensores[i%2]=i;
    }
    
    estacionamiento_init(&parking_castelar);
    /* INICIALIZACIÓN BOTONES */
    
    RCC->APB2ENR|=RCC_APB2ENR_IOPBEN|RCC_APB2ENR_AFIOEN;
    for(int i=0;i<3;i++){
        GPIOB->CRL&=~(0xF<<botones[i]*4);
        GPIOB->CRL|= (0x8<<botones[i]*4);
        GPIOB->BSRR|= (1<<botones[i]+16);
        AFIO->EXTICR[0]|=(0x1<<(botones[i]%4)*4);
    }
    NVIC_EnableIRQ(EXTI0_IRQn);
    NVIC_EnableIRQ(EXTI1_IRQn);
    NVIC_EnableIRQ(EXTI2_IRQn);
    NVIC_SetPriority(EXTI0_IRQn,1);
    NVIC_SetPriority(EXTI1_IRQn,0);
    NVIC_SetPriority(EXTI2_IRQn,1);

    while(1){
        estacionamiento(&parking_castelar);
    }
    return 0;
}

//NORMAL
void EXTI0_IRQHandler(){
    if(EXTI->PR&(1<<NORMAL)){
        EXTI->PR|=(1<<NORMAL);
        parking_castelar.modo='n';
    }
}
//NORMAL
void EXTI1_IRQHandler(){
if(EXTI->PR&(1<<RESTRINGIDO)){
        EXTI->PR|=(1<<RESTRINGIDO);
        parking_castelar.modo='r';
    }
}
//LIBRE
void EXTI2_IRQHandler(){
if(EXTI->PR&(1<<LIBRE)){
        EXTI->PR|=(1<<LIBRE);
        parking_castelar.modo='l';
    }
}