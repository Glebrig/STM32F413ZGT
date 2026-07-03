#include "flash.h"

// ------------------------------------------------------------------------
// Вспомогательные функции
// ------------------------------------------------------------------------

void SPI_Delay(void) {
    for (volatile int i = 0; i < 200; i++);
}

static uint8_t SPI_TransmitReceive(uint8_t data) {
    uint8_t received = 0;

    // Передаём биты, начиная со старшего
    for (int i = 7; i >= 0; i--) {
        // Выставить бит на MOSI
        if (data & (1 << i))
            FLASH_MOSI_HIGH();
        else
            FLASH_MOSI_LOW();

        SPI_Delay();

        // Поднять SCK (передний фронт) – в режиме 0 данные читаются по переднему фронту
        FLASH_SCK_HIGH();
        SPI_Delay();

        // Прочитать бит с MISO
        if (FLASH_MISO_READ())
            received |= (1 << i);

        // Опустить SCK (задний фронт)
        FLASH_SCK_LOW();
        SPI_Delay();
    }

    return received;
}

static void SPI_Transmit(uint8_t data) {
    for (int i = 7; i >= 0; i--) {
        if (data & (1 << i))
            FLASH_MOSI_HIGH();
        else
            FLASH_MOSI_LOW();

        FLASH_SCK_HIGH();
        SPI_Delay();
        FLASH_SCK_LOW();
        SPI_Delay();
    }
}


static uint8_t SPI_Receive(void){
    return 0;

}

// ------------------------------------------------------------------------
// Реализация функций
// ------------------------------------------------------------------------


uint32_t Flash_ReadID(void) {
    uint32_t id = 0;

    FLASH_CS_LOW();
    
    // Отправка 0x9F
    SPI_Transmit(FLASH_CMD_RDID);
    
    // Чтение 3 байт ID
    for (int byte = 0; byte < 3; byte++) {
        uint8_t val = 0;
        for (int bit = 7; bit >= 0; bit--) {
            FLASH_SCK_HIGH();
            SPI_Delay();
            if (FLASH_MISO_READ()) {
                val |= (1 << bit);
            }
            FLASH_SCK_LOW();
            SPI_Delay();
        }
        id = (id << 8) | val;
    }
    
    FLASH_CS_HIGH();

    printf("FLASH ID = 0x%06lX\r\n", id);

    return id;
}


uint8_t Flash_ReadStatus(void) {
    uint8_t status = 0;

    FLASH_CS_LOW();
    SPI_Transmit(FLASH_CMD_RDSR);
    status = SPI_TransmitReceive(0xFF);
    FLASH_CS_HIGH();

    //printf("FLASH Status register = 0x%02X\n", status);
    return status;
}

void Flash_WaitForReady(void) {
    while (Flash_ReadStatus() & 0x01) {
        
    }
}

void Flash_WriteEnable(void) {
    FLASH_CS_LOW();
    SPI_Transmit(FLASH_CMD_WREN); //0x06
    FLASH_CS_HIGH();
}

void Flash_SectorErase(uint32_t address) {
    Flash_WriteEnable();

    FLASH_CS_LOW();
    SPI_Transmit(FLASH_CMD_SE);
    SPI_Transmit((address >> 16) & 0xFF);
    SPI_Transmit((address >> 8) & 0xFF);
    SPI_Transmit(address & 0xFF);
    FLASH_CS_HIGH();

    Flash_WaitForReady();
    printf("FLASH Sector Erase at 0x%08X\n\r", address);
}

void Flash_PageProgram(uint32_t address, uint8_t *data, uint32_t size) {
    if (size > 256) {size = 256;}
    
    printf("FLASH Page Program at 0x%08X, size %lu bytes\n\r", address, size);

    Flash_WriteEnable();
    Flash_WaitForReady();

    FLASH_CS_LOW();
    SPI_Transmit(FLASH_CMD_PP);
    SPI_Transmit((address >> 16) & 0xFF);
    SPI_Transmit((address >> 8) & 0xFF);
    SPI_Transmit(address & 0xFF);

    for (uint32_t i = 0; i < size; i++) {
        SPI_Transmit(data[i]);
    }

    FLASH_CS_HIGH();
    Flash_WaitForReady();
}

void Flash_Read(uint32_t address, uint8_t *buffer, uint32_t size) {
    printf("FLASH Read at 0x%08X, size %lu bytes\n\r", address, size);
    
    Flash_WaitForReady();

    FLASH_CS_LOW();
    SPI_Transmit(FLASH_CMD_READ);
    SPI_Transmit((address >> 16) & 0xFF);
    SPI_Transmit((address >> 8) & 0xFF);
    SPI_Transmit(address & 0xFF);

    for (uint32_t i = 0; i < size; i++) {
        buffer[i] = SPI_TransmitReceive(0xFF);
    }

    FLASH_CS_HIGH();
}