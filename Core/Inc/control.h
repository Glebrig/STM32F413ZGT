#ifndef CONTROL_H
#define CONTROL_H

#include "main.h"


#define ResetCS() HAL_GPIO_WritePin(GPIOG, GPIO_PIN_6, GPIO_PIN_RESET)
#define SetCS() HAL_GPIO_WritePin(GPIOG, GPIO_PIN_6, GPIO_PIN_SET)

#define SetSCK() HAL_GPIO_WritePin(GPIOG, GPIO_PIN_4, GPIO_PIN_SET)
#define ResetSCK() HAL_GPIO_WritePin(GPIOG, GPIO_PIN_4, GPIO_PIN_RESET)

#define SetMOSI() HAL_GPIO_WritePin(GPIOG, GPIO_PIN_3, GPIO_PIN_SET)
#define ResetMOSI()  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_3, GPIO_PIN_RESET)

#define ReadMISO() HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_5)


void Delay(void);
uint8_t TransmitReceive(uint8_t data);
void Transmit(uint8_t data);
uint8_t Receive(void);

#endif //