#include "nand.h"
#include <stdio.h>

void NAND_ReadPage(uint16_t page_addr, uint16_t col_addr, uint8_t *buffer, uint32_t size) {
    if (size > 2048) size = 2048;
    ResetCS();
    Transmit(0x13);
    Transmit(0x00);
    Transmit((page_addr >> 8) & 0xFF);
    Transmit(page_addr & 0xFF);
    SetCS();
    
    NAND_WaitForReady();
    
    ResetCS();
    Transmit(0x03);
    Transmit((col_addr >> 8) & 0xFF);
    Transmit(col_addr & 0xFF);
    
    for (uint32_t i = 0; i < size; i++) {
        buffer[i] = Receive();
    }
    SetCS();
}

// void NAND_ReadBlock(uint16_t block_addr, uint8_t *buffer) {
//     const uint32_t PAGES_PER_BLOCK = 64;
//     const uint32_t PAGE_SIZE = 2048;

    
//     for (uint32_t page = 0; page < PAGES_PER_BLOCK; page++) {
//         uint16_t page_addr = (block_addr << 6) | page;
//         uint32_t offset = page * PAGE_SIZE;
        
//         // Прогресс
//        // printf("[%3lu%%] Page %lu / %lu (0x%04X)\r\n", 
//          //      (page * 100) / PAGES_PER_BLOCK, page, PAGES_PER_BLOCK, page_addr);
        
//         // Читаем страницу
//         NAND_ReadPage(page_addr, 0, &buffer[offset], PAGE_SIZE);

//         for (int i = 0; i < 256; i++) {
//             if (i % 16 == 0) {
//                 if (i > 0) printf("\r\n");
//                 printf("    %04X: ", i);
//             }
//             printf("%02X ", buffer[offset + i]);
//         }
//         printf("\r\n");
        
//         HAL_Delay(1);
//     }
// }

void NAND_WaitForReady(void) {
    uint8_t status;
    uint32_t timeout = 1000000;
    
    do {
        ResetCS();
        Transmit(0x0F);
        Transmit(0xC0);
        status = Receive();
        SetCS();
        
        if (--timeout == 0) {
            printf("ERROR\r\n");
            break;
        }
    } while (status & 0x01);  // OIP = бит 0
}

void NAND_DumpBlock(uint16_t block_addr) {
    const uint32_t PAGES_PER_BLOCK = 64;
    const uint32_t PAGE_SIZE = 2048;
    uint8_t buffer[PAGE_SIZE];
    
    for (uint32_t page = 0; page < PAGES_PER_BLOCK; page++) {
        uint16_t page_addr = (block_addr << 6) | page;
        uint32_t absolute_offset = block_addr * PAGES_PER_BLOCK * PAGE_SIZE + page * PAGE_SIZE;
        
        NAND_ReadPage(page_addr, 0, buffer, PAGE_SIZE);
        
        for (uint32_t i = 0; i < PAGE_SIZE; i++) {
            if (i % 16 == 0) {
                if (i > 0) printf("\r\n");
                printf("%06lX: ", absolute_offset + i);
            }
            printf("%02X ", buffer[i]);
        }
        printf("\r\n");
    }
}


uint32_t NAND_ReadID(void){
    uint32_t id = 0;
    
    ResetCS();
    Transmit(0x9F);     
    Transmit(0xFF);
    
    id = (Receive() << 16) | (Receive() << 8)  | Receive();           
    
    SetCS();
    
    return id;
}
