#ifndef __AP3216C_H
#define __AP3216C_H
#include "sys.h"


#define AP3216C_ADDR    0X3C	//AP3216C器件IIC地址(左移了一位)

uint8_t AP3216C_Init(void); 
uint8_t AP3216C_WriteOneByte(uint8_t reg,uint8_t data);
uint8_t AP3216C_ReadOneByte(uint8_t reg);
void AP3216C_ReadData(uint16_t* ir,uint16_t* ps,uint16_t* als);
#endif
