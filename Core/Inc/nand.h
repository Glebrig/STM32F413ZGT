#ifndef NAND_H
#define NAND_H

#include "main.h"


void NAND_ReadPage(uint16_t page_addr, uint16_t col_addr, uint8_t *buffer, uint32_t size);

#endif // NAND_H