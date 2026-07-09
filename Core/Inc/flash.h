#ifndef FLASH_H
#define FLASH_H

#include "main.h"
#include "control.h"

#define FLASH_CMD_WREN   0x06
#define FLASH_CMD_RDSR   0x05
#define FLASH_CMD_READ   0x03
#define FLASH_CMD_PP     0x02
#define FLASH_CMD_SE     0x20
#define FLASH_CMD_RDID   0x9F

uint32_t Flash_ReadID(void);
uint8_t  Flash_ReadStatus(void);
void     Flash_WaitForReady(void);
void     Flash_WriteEnable(void);
void     Flash_SectorErase(uint32_t address);
void     Flash_PageProgram(uint32_t address, uint8_t *data, uint32_t size);
void     Flash_Read(uint32_t address, uint8_t *buffer, uint32_t size);
void     Flash_FillMemory(uint8_t pattern);
void     Flash_DumpAllMemory();

#endif // FLASH_H