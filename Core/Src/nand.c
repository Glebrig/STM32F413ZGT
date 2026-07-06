#include "flash.h"
#include "nand.h"

void NAND_ReadPage(uint16_t page_addr, uint16_t col_addr, uint8_t *buffer, uint32_t size) {
    if (size > 2048) size = 2048;
    
    // Page Read (13h) — загрузить страницу в кэш
    FLASH_CS_LOW();
    SPI_Transmit(0x13);  
    SPI_Transmit(0x00); 
    SPI_Transmit((page_addr >> 8) & 0xFF);  
    SPI_Transmit(page_addr & 0xFF);
    FLASH_CS_HIGH();
    
    HAL_Delay(1);
    
   
    // Read From Cache (03h) — читаем данные
    FLASH_CS_LOW();
    SPI_Transmit(0x03); 
    SPI_Transmit((col_addr >> 8) & 0xFF);
    SPI_Transmit(col_addr & 0xFF);
    
    for (uint32_t i = 0; i < size; i++) {
        buffer[i] = SPI_TransmitReceive(0xFF);
    }
    FLASH_CS_HIGH();
}