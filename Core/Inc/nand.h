#ifndef NAND_H
#define NAND_H

#include "main.h"
#include "flash.h"
#include "control.h"


void NAND_ReadPage(uint16_t page_addr, uint16_t col_addr, uint8_t *buffer, uint32_t size);
// void NAND_ReadBlock(uint16_t block_addr, uint8_t *buffer);
void NAND_WaitForReady(void);
void NAND_DumpBlock(uint16_t block_addr);
uint32_t NAND_ReadID(void);

#endif // NAND_H
