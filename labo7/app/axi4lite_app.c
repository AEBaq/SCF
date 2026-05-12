/*****************************************************************************************
 * HEIG-VD
 * Haute Ecole d'Ingenerie et de Gestion du Canton de Vaud
 * School of Business and Engineering in Canton de Vaud
 *****************************************************************************************
 * REDS Institute
 * Reconfigurable Embedded Digital Systems
 *****************************************************************************************
 *
 * File                 : axi4lite_app.c
 * Author               : Me
 * Date                 : Today
 *
 * Context              : SCF tutorial lab
 *
 *****************************************************************************************
 * Brief: J'ai toujours faim...
 *
 *****************************************************************************************
 * Modifications :
 * Ver    Date        Student      Comments
 * 1.0    21.04.2026  AEB          Lab07
 *
*****************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdint.h>

#include "../include/axi4lite_ioctl.h"

/*****
 *    Helper
 *****/
static const uint8_t hex_letters[6] = {
    0x77, // A
    0x7C, // B
    0x39, // C
    0x5E, // D
    0x79, // E
    0x71  // F
};

/**
 * @brief Read a 32-bit value from a register
 * @param fd File descriptor of the device
 * @param reg Register offset
 * @return The value read from the register 
 */
static uint32_t reg_read(int fd, uint32_t reg) {
    uint32_t val = 0;
    if (ioctl(fd, AXI4LITE_READ(reg), &val) < 0)
        perror("[Error] ioctl READ");
    return val;
}

/**
 * @brief Write a 32-bit value to a register
 * @param fd File descriptor of the device
 * @param reg Register offset
 * @param val Value to write
 * @return void
 */
static void reg_write(int fd, uint32_t reg, uint32_t val) {
    if (ioctl(fd, AXI4LITE_WRITE(reg), &val) < 0)
        perror("[Error] ioctl WRITE");
}

/**
 * @brief Get the value to write to the HEX display for a given range of letters
 * @param start Index of the first letter (0 for 'A')
 * @param end Index of the last letter (5 for 'F')
 * @return The value to write to the HEX display
 */
static uint32_t hex_seg_value(int start, int end) {
    if (start < 0 || end > 5 || start > end) return 0;

    uint32_t v = hex_letters[start];

    for (int i = start + 1; i <= end; i++)
        v = (v << SHIFT_HEX) | hex_letters[i];
    return v;
}

/*****
 *    TESTS
 *****/

 /**
  * @brief Test if the constant register is correct
  * @param fd File descriptor of the device
  * @return 0 if the constant is correct, -1 otherwise
  */
static int test_constant(int fd) {
    uint32_t val = reg_read(fd, AXI4LITE_REG_ID);
    printf("[TEST] Constante : should be 0x%08X, value readed = 0x%08X %s\n", (unsigned int)AXI4LITE_CONSTANT, (unsigned int)val, (val == (uint32_t)AXI4LITE_CONSTANT) ? "Valide" : "Invalide");
    return (val == AXI4LITE_CONSTANT) ? 0 : -1;
}

/**
 * @brief Test reading the state of the switches
 * @param fd File descriptor of the device
 * @return void
 */
static void test_switches(int fd) {
    uint32_t val = reg_read(fd, AXI4LITE_REG_SW) & MASK_SW;
    printf("[TEST] Switches : 0x%03X\n", val);
}


/**
 * @brief Test writing to the LEDs and using the set/clear registers
 * @param fd File descriptor of the device
 * @return void
 */
static void test_leds(int fd) {
    uint32_t val;

    // all LEDs on for 2 seconds
    printf("[TEST] LEDs ON (without using SET register) for 2s...\n");
    reg_write(fd, AXI4LITE_REG_LED_OUTPUT, MASK_LED);
    sleep(2);
    reg_write(fd, AXI4LITE_REG_LED_OUTPUT, 0x00);

    // Test SET : every two LEDs on
    printf("[TEST] LEDs on via SET...\n");
    reg_write(fd, AXI4LITE_REG_LED_SET, 0x155);

    sleep(2); // Let 2 seconds to see the LEDs on

    // Test CLR : every two LEDs off
    printf("[TEST] LEDs off via CLEAR...\n");
    reg_write(fd, AXI4LITE_REG_LED_CLR, 0x155);
}

/**
 * @brief Test the keys
 * @param fd File descriptor of the device
 * @return void
 */
static void test_key(int fd) {
    // Clear edge capture register
    reg_write(fd, AXI4LITE_REG_KEY_EDGE, MASK_KEY);
    
    printf("[TEST] Press a key ...\n");
    while (1) {
        uint32_t edge = reg_read(fd, AXI4LITE_REG_KEY_EDGE) & MASK_KEY;
        if (edge) {
            printf("[TEST] Edge capture  : 0x%X\n", edge);
            reg_write(fd, AXI4LITE_REG_KEY_EDGE, edge);
            break;
        }
        usleep(100000);
    }
}

/**
 * @brief Test writing values to the HEX displays
 * @param fd File descriptor of the device
 * @return void
 */
static void test_hex(int fd) {
    printf("[TEST] HEX display on during 2s...\n");
    reg_write(fd, AXI4LITE_REG_HEX3_0, hex_seg_value(0, 3));
    reg_write(fd, AXI4LITE_REG_HEX5_4, hex_seg_value(4, 5));
    sleep(2);
    reg_write(fd, AXI4LITE_REG_HEX3_0, ~MASK_HEX3_0);
    reg_write(fd, AXI4LITE_REG_HEX5_4, ~MASK_HEX5_4);
}


/**
 * @brief Main function: Opens the device, runs the tests and closes the device
 * @param void
 * @return int : 0 on success, 1 on failure
 */
int main(void) {
    int fd = open("/dev/axi4lite", O_RDWR);
    if (fd < 0) {
        perror("[Error] Impossible to open /dev/axi4lite");
        return 1;
    }

    if (test_constant(fd) != 0) {
        printf("[ERROR] Constant test failed\n");
        close(fd);
        return 1;
    }

    // Run all the other tests
    test_switches(fd);
    test_leds(fd);
    test_key(fd);
    test_hex(fd);

    close(fd);
    return 0;
}