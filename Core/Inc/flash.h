#ifndef FLASH_H
#define FLASH_H

#include "main.h"

// ------------------------------------------------------------------------
// Настройка пинов 
// ------------------------------------------------------------------------
#define FLASH_MOSI_PIN         GPIO_PIN_3
#define FLASH_SCK_PIN          GPIO_PIN_4
#define FLASH_MISO_PIN         GPIO_PIN_5
#define FLASH_CS_PIN           GPIO_PIN_6

// Управление CS
#define FLASH_CS_LOW()   HAL_GPIO_WritePin(GPIOG, FLASH_CS_PIN, GPIO_PIN_RESET)
#define FLASH_CS_HIGH()  HAL_GPIO_WritePin(GPIOG, FLASH_CS_PIN, GPIO_PIN_SET)

// Управление SCK
#define FLASH_SCK_HIGH() HAL_GPIO_WritePin(GPIOG, FLASH_SCK_PIN, GPIO_PIN_SET)
#define FLASH_SCK_LOW()  HAL_GPIO_WritePin(GPIOG, FLASH_SCK_PIN, GPIO_PIN_RESET)

// Управление MOSI
#define FLASH_MOSI_HIGH() HAL_GPIO_WritePin(GPIOG, FLASH_MOSI_PIN, GPIO_PIN_SET)
#define FLASH_MOSI_LOW()  HAL_GPIO_WritePin(GPIOG, FLASH_MOSI_PIN, GPIO_PIN_RESET)

// Чтение MISO
#define FLASH_MISO_READ() HAL_GPIO_ReadPin(GPIOG, FLASH_MISO_PIN)

// ------------------------------------------------------------------------
// Коды команд
// ------------------------------------------------------------------------
#define FLASH_CMD_WREN   0x06
#define FLASH_CMD_RDSR   0x05
#define FLASH_CMD_READ   0x03
#define FLASH_CMD_PP     0x02
#define FLASH_CMD_SE     0x20
#define FLASH_CMD_RDID   0x9F

// ------------------------------------------------------------------------
// Прототипы функций
// ------------------------------------------------------------------------
void     SPI_Delay(void);

uint32_t Flash_ReadID(void);
uint8_t  Flash_ReadStatus(void);
void     Flash_WaitForReady(void);
void     Flash_WriteEnable(void);
void     Flash_SectorErase(uint32_t address);
void     Flash_PageProgram(uint32_t address, uint8_t *data, uint32_t size);
void     Flash_Read(uint32_t address, uint8_t *buffer, uint32_t size);

#endif // FLASH_H