/*****************************************************************************************
 * HEIG-VD
 * Haute Ecole d'Ingenerie et de Gestion du Canton de Vaud
 * School of Business and Engineering in Canton de Vaud
 *****************************************************************************************
 * REDS Institute
 * Reconfigurable Embedded Digital Systems
 *****************************************************************************************
 *
 * File                 : axi4lite_io.c
 * Author               :
 * Date                 :
 *
 * Context              : SCF tutorial lab
 *
 *****************************************************************************************
 * Brief: Test application for AXI4-Lite FPGA IO laboratory
 *
 *****************************************************************************************
 * Modifications :
 * Ver    Date        Student      Comments
 * 1.0    14.04.2026  AEB          Lab04
 *
*****************************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdint.h>

// Address of the AXI4-Lite IP on the HPS-to-FPGA bus
#define AXI4LITE_BASE_ADDR  0xFF200000
#define PAGE_SIZE           0x1000      // 4KB

// Offset
#define CONST_REG           0x00
#define TEST_REG            0x04
#define INPUT_KEYS_REG      0x08
#define EDGE_CAPTURE_REG    0x0C
#define INPUT_SW_REG        0x10
#define OUTPUT_LED_REG      0x14
#define SET_REG             0x18
#define CLEAR_REG           0x1C
#define OUTPUT_HEX34_REG    0x20
#define OUTPUT_HEX54_REG    0x24

int main(void) {
    int fd;
    volatile uint32_t *base_addr;
    uint32_t value;
    
    // Open /dev/mem
    fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        printf("Error: Cannot open /dev/mem\n");
        return 1;
    }
    
    // Map the AXI4-Lite address space
    base_addr = (uint32_t *)mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, 
                                  MAP_SHARED, fd, AXI4LITE_BASE_ADDR);
    if (base_addr == MAP_FAILED) {
        printf("Error: mmap failed\n");
        close(fd);
        return 1;
    }
    
    // [Test] Read the constant
    value = *(base_addr + CONST_REG / 4);
    printf("[INFO] Constant: 0x%X \n", value);
    
    // [Test] Read inputs (Keys + Switches)
    value = *(base_addr + INPUT_KEYS_REG / 4);
    printf("[INFO] Keys: 0x%X\n", value);
    
    value = *(base_addr + INPUT_SW_REG / 4);
    printf("[INFO] Switches: 0x%X\n", value);
    
    // [Test] Write to LEDs
    *(base_addr + OUTPUT_LED_REG / 4) = 0x3FF;  // All LEDs ON
    printf("[INFO] LEDs set to 0x3FF\n");
    
    // [Test] Use Set/Clear registers
    *(base_addr + SET_REG / 4) = 0x001;  // Set bit 0
    *(base_addr + CLEAR_REG / 4) = 0x002;  // Clear bit 1 
    
    // [Test] Write to HEX displays
    *(base_addr + OUTPUT_HEX34_REG / 4) = 0x00;  // HEX3-0
    *(base_addr + OUTPUT_HEX54_REG / 4) = 0x00;  // HEX5-4
    
    // Exit
    munmap((void *)base_addr, PAGE_SIZE);
    close(fd);
    
    return 0;
}
