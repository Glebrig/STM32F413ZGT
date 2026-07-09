#include "flash.h"
#include "stdio.h"

uint32_t Flash_ReadID(void) {
    uint32_t id = 0;

    ResetCS();
    
    // Отправка 0x9F
    Transmit(FLASH_CMD_RDID);
    
    // Чтение 3 байт ID
    for (int byte = 0; byte < 3; byte++) {
        uint8_t val = 0;
        for (int bit = 7; bit >= 0; bit--) {
            SetSCK();
            Delay();
            if (ReadMISO()) {
                val |= (1 << bit);
            }
            ResetSCK();
            Delay();
        }
        id = (id << 8) | val;
    }
    
    SetCS();

    //printf("FLASH ID = 0x%06lX\r\n", id);

    return id;
}

uint8_t Flash_ReadStatus(void) {
    uint8_t status = 0;

    ResetCS();
    Transmit(FLASH_CMD_RDSR);
    status = Receive();
    SetCS();

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
    ResetCS();
    Transmit(FLASH_CMD_WREN); //0x06
    SetCS();
}

void Flash_SectorErase(uint32_t address) {
    Flash_WriteEnable();

    ResetCS();
    Transmit(FLASH_CMD_SE);
    Transmit((address >> 16) & 0xFF);
    Transmit((address >> 8) & 0xFF);
    Transmit(address & 0xFF);
    SetCS();

    Flash_WaitForReady();
    //printf("FLASH Sector Erase at 0x%08X\n\r", address);
}

void Flash_PageProgram(uint32_t address, uint8_t *data, uint32_t size) {
    if (size > 256) {size = 256;}
    
    //printf("FLASH Page Program at 0x%08X, size %lu bytes\n\r", address, size);

    Flash_WriteEnable();
    Flash_WaitForReady();

    ResetCS();
    Transmit(FLASH_CMD_PP);
    Transmit((address >> 16) & 0xFF);
    Transmit((address >> 8) & 0xFF);
    Transmit(address & 0xFF);

    for (uint32_t i = 0; i < size; i++) {
        Transmit(data[i]);
    }

    SetCS();
    Flash_WaitForReady();

    uint8_t status = Flash_ReadStatus();
    if (status & 0x01) {
        //printf("[FLASH] ERROR: Write failed, status = 0x%02X\r\n", status);
    }

}

void Flash_Read(uint32_t address, uint8_t *buffer, uint32_t size) {
    //printf("FLASH Read at 0x%08X, size %lu bytes\n\r", address, size);
    
    Flash_WaitForReady();

    ResetCS();
    Transmit(FLASH_CMD_READ);
    Transmit((address >> 16) & 0xFF);
    Transmit((address >> 8) & 0xFF);
    Transmit(address & 0xFF);

    for (uint32_t i = 0; i < size; i++) {
        buffer[i] = Receive();
    }

    SetCS();
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
        // if (current_addr % (1024 * 1024) == 0) {
        //     printf('\n', "[%3lu%%] 0x%08lX\r\n", 
        //            (current_addr * 100) / total_bytes, current_addr);
        // }
        
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
