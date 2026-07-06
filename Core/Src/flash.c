#include "flash.h"
#include "stdio.h"

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

    //printf("FLASH ID = 0x%06lX\r\n", id);

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
    uint32_t timeout = 1000000;  // ~1 секунда
    while (Flash_ReadStatus() & 0x01) {
        if (--timeout == 0) {
            break;
        }
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
    //printf("FLASH Sector Erase at 0x%08X\n\r", address);
}

void Flash_PageProgram(uint32_t address, uint8_t *data, uint32_t size) {
    if (size > 256) {size = 256;}
    
    //printf("FLASH Page Program at 0x%08X, size %lu bytes\n\r", address, size);

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

    uint8_t status = Flash_ReadStatus();
    if (status & 0x01) {
        //printf("[FLASH] ERROR: Write failed, status = 0x%02X\r\n", status);
    }

}

void Flash_Read(uint32_t address, uint8_t *buffer, uint32_t size) {
    //printf("FLASH Read at 0x%08X, size %lu bytes\n\r", address, size);
    
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

void Flash_FillMemory(uint8_t pattern) {
    const uint32_t FLASH_SIZE = 8 * 1024 * 1024;  // 8 МБ
    const uint32_t PAGE_SIZE = 256;
    const uint32_t SECTOR_SIZE = 4096;            // 4 КБ (сектор)
    
    uint8_t buffer[PAGE_SIZE];
    
    // Заполняем буфер
    for (uint32_t i = 0; i < PAGE_SIZE; i++) {
        buffer[i] = pattern + (i & 0xFF);
    }
    
    uint32_t total_sectors = FLASH_SIZE / SECTOR_SIZE;
    
    for (uint32_t sector = 0; sector < total_sectors; sector++) {
        uint32_t sector_addr = sector * SECTOR_SIZE;
        
        // Прогресс
        if (sector % 100 == 0) {
            printf("[%3lu%%] Sector %lu / %lu at 0x%08lX\r\n", 
                   (sector * 100) / total_sectors, sector, total_sectors, sector_addr);
        }
        
        // Стираем сектор
        Flash_SectorErase(sector_addr);
        
        // Записываем страницы внутри сектора
        for (uint32_t page = 0; page < SECTOR_SIZE / PAGE_SIZE; page++) {
            uint32_t page_addr = sector_addr + page * PAGE_SIZE;
            
            //Обновляем буфер
            for (uint32_t i = 0; i < PAGE_SIZE; i++) {
                buffer[i] = pattern + (page & 0xFF) + (i & 0xFF);
            }
            
            Flash_PageProgram(page_addr, buffer, PAGE_SIZE);
        }
    }
}

void Flash_DumpAllMemory() {
    const uint32_t FLASH_SIZE = 8 * 1024 * 1024;  // 8 МБ
    const uint32_t BUFFER_SIZE = 256;
    uint8_t buffer[BUFFER_SIZE];
    
    uint32_t total_bytes = FLASH_SIZE;
    uint32_t current_addr = 0;
    uint32_t chunk_size;
    uint32_t bytes_in_line = 0;
    
    while (current_addr < total_bytes) {
        chunk_size = (total_bytes - current_addr > BUFFER_SIZE) ? BUFFER_SIZE : (total_bytes - current_addr);
        Flash_Read(current_addr, buffer, chunk_size);
        
        // Прогресс
        if (current_addr % (1024 * 1024) == 0) {
            // printf('\n', "[%3lu%%] 0x%08lX\r\n", 
            //        (current_addr * 100) / total_bytes, current_addr);
        }
        
        // // ASCII вывод
        // for (uint32_t i = 0; i < chunk_size; i++) {
        //     printf('\n');
        //     printf("%c", buffer[i]);    
        // }
        
       // HEX вывод: по 16 байт в строке
        for (uint32_t i = 0; i < chunk_size; i++) {
            if (bytes_in_line == 0) {
                if (i > 0 || current_addr > 0) {
                    printf("\r\n");
                }
                printf("%08lX: ", current_addr + i);
            }
            printf("%02X ", buffer[i]);
            bytes_in_line++;
            if (bytes_in_line == 16) {
                bytes_in_line = 0;
            }
        }
        
        current_addr += chunk_size;
    }
}

/*
проблема записи:
FLASH ID = 0xC22017
[  0%] Sector 0 / 2048 at 0x00000000
[  4%] Sector 100 / 2048 at 0x00064000
[  9%] Sector 200 / 2048 at 0x000C8000
[ 14%] Sector 300 / 2048 at 0x0012C000
[ 19%] Sector 400 / 2048 at 0x00190000
[ 24%] Sector 500 / 2048 at 0x001F4000
[ 29%] Sector 600 / 2048 at 0x00258000
[ 34%] Sector 700 / 2048 at 0x002BC000
[ 39%] Sector 800 / 2048 at 0x00320000
[ 43%] Sector 900 / 2048 at 0x00384000
[ 48%] Sector 1000 / 2048 at 0x003E8000
[ 53%] Sector 1100 / 2048 at 0x0044C000
[ 58%] Sector 1200 / 2048 at 0x004B0000
[ 63%] Sector 1300 / 2048 at 0x00514000
[ 68%] Sector 1400 / 2048 at 0x00578000
[ 73%] Sector 1500 / 2048 at 0x005DC000
[ 78%] Sector 1600 / 2048 at 0x00640000
[ 83%] Sector 1700 / 2048 at 0x006A4000
[ 87%] Sector 1800 / 2048 at 0x00708000
[ 92%] Sector 1900 / 2048 at 0x0076C000
[ 97%] Sector 2000 / 2048 at 0x007D0000

и всё, не доходит до 2048

+ виснет при попытке записать, то есть проблема в:
  Flash_SectorErase(sector_addr);
        
        // Записываем страницы внутри сектора
        for (uint32_t page = 0; page < SECTOR_SIZE / PAGE_SIZE; page++) {
            uint32_t page_addr = sector_addr + page * PAGE_SIZE;
            
            // Обновляем буфер с разными данными для каждой страницы
            // for (uint32_t i = 0; i < PAGE_SIZE; i++) {
            //     buffer[i] = pattern + (page & 0xFF) + (i & 0xFF);
            // }
            
            Flash_PageProgram(page_addr, buffer, PAGE_SIZE);
        }


Проблема чтения:
FLASH ID = 0xC22017
FILL[  0%] 0x00000000
��������������������������������������������������������������������������������������


123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~������������������������������������������[  0%] 0x00000100
�������������������������������������������������������������������������������������


123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~�������������������������������������������[  0%] 0x00000200
������������������������������������������������������������������������������������


123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~��������������������������������������������[  0%] 0x00000300
�����������������������������������������������������������������������������������

*/