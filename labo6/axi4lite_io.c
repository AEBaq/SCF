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
#define OUTPUT_HEX30_REG    0x20
#define OUTPUT_HEX54_REG    0x24

#define MASK_LEDS           0x3FF  // Mask for 10 LEDs
#define MASK_KEYS           0xF    // Mask for 4 keys
#define MASK_SWITCHES       0x3FF  // Mask for 10 switches

static const uint8_t hex_letters[6] = {
    0x77, // A
    0x7C, // B
    0x39, // C
    0x5E, // D
    0x79, // E
    0x71  // F
};

/**
 * Generate a 32-bit value for the HEX displays based on the start and end indices
 */
uint32_t hex_seg_value(int start, int end){
    if (start < 0 || start > 5 || end < 0 || end > 5 || start > end) {
        return 0x00; // Invalid range, return blank
    }

    size_t i = start;
    uint32_t value = hex_letters[i++];

    while (i <= end) {
        value <<= 8; // Shift left by 8 bits to make room for the next character
        value |= hex_letters[i++]; // Add the next character
    }

    return value;
}

int main(void) {
    int fd, ret;
    volatile uint32_t *base_addr;
    uint32_t value, past_key;
    
    // Open /dev/mem
    fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        printf("[Error] Cannot open /dev/mem\n");
        return 1;
    }
    
    // Map the AXI4-Lite address space
    base_addr = (uint32_t *)mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, 
                                  MAP_SHARED, fd, AXI4LITE_BASE_ADDR);
    if (base_addr == MAP_FAILED) {
        printf("[Error] mmap failed\n");
        close(fd);
        return 1;
    }
    
    // [Test] Read the constant
    value = *(base_addr + CONST_REG);
    printf("[INFO] Constant: 0x%X \n", value);
    
    // [Test] Write to LEDs
    *(base_addr + OUTPUT_LED_REG) = MASK_LEDS;  // All LEDs ON
    printf("[INFO] LEDs set to 0x%X\n", MASK_LEDS);
    
    usleep(2000000); // Short delay to see the LEDs on for 2 second
    
    *(base_addr + OUTPUT_LED_REG) = ~MASK_LEDS;  // All LEDs OFF
    
    // [Test] Read inputs
    value = *(base_addr + INPUT_SW_REG) & MASK_SWITCHES; // Switches
    printf("[INFO] Switches: 0x%X\n", value);

    past_key = *(base_addr + INPUT_KEYS_REG) & MASK_KEYS; // Initial key state
    
    // Check edge capture
    while (1) {
        value = *(base_addr + EDGE_CAPTURE_REG) & MASK_KEYS; // Edge capture for keys
        if (value != past_key) { // Check for any change in keys
            printf("[INFO] Edge Capture (Keys): %d\n", value);
            break; // Exit after detecting an edge
        }
        printf("[INFO] Waiting for a key to be pressed...\n");
        past_key = value; // Update past key state
        usleep(500000); // Sleep for 0.5 seconds before checking again to avoid rebouncing issues
    }
    
    // [Test] Write to HEX displays
    *(base_addr + OUTPUT_HEX30_REG) = hex_seg_value(0, 3);  // HEX3-0 : AAAA
    *(base_addr + OUTPUT_HEX54_REG) = hex_seg_value(4, 5);  // HEX5-4 : AA
    
    usleep(2000000); // Short delay to see the HEX displays for 2 seconds

    *(base_addr + OUTPUT_HEX30_REG) = 0x00;  // Clear HEX3-0
    *(base_addr + OUTPUT_HEX54_REG) = 0x00;  // Clear HEX5-4

    // Exit
    ret = munmap((void *)base_addr, PAGE_SIZE);
    if (ret != 0) {
        printf("[Error] munmap failed\n");
    }
    close(fd);
    
    return 0;
}
