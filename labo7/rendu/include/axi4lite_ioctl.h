/*****************************************************************************************
 * HEIG-VD
 * Haute Ecole d'Ingenerie et de Gestion du Canton de Vaud
 * School of Business and Engineering in Canton de Vaud
 *****************************************************************************************
 * REDS Institute
 * Reconfigurable Embedded Digital Systems
 *****************************************************************************************
 *
 * File                 : axi4lite_ioctl.h
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

#ifndef AXI4LITE_IOCTL_H
#define AXI4LITE_IOCTL_H

#include <linux/ioctl.h>
#include <stdint.h>

// Base address
#define AXI_HPS_BASE_ADDR           0xFF200000
#define AXI_PAGE_SIZE               0x1000

// Constant
#define AXI4LITE_CONSTANT           0xBADB100D

// Register offsets
#define AXI4LITE_REG_ID             0x00 // R, 0xBADB100D
#define AXI4LITE_REG_TEST           0x04 // RW
#define AXI4LITE_REG_KEY_INPUT      0x08 // RW, for keys
#define AXI4LITE_REG_KEY_EDGE       0x0C // RW, for keys
#define AXI4LITE_REG_SW             0x10 // R
#define AXI4LITE_REG_LED_OUTPUT     0x14 // RW
#define AXI4LITE_REG_LED_SET        0x18 // W
#define AXI4LITE_REG_LED_CLR        0x1C // W
#define AXI4LITE_REG_HEX3_0         0x20 // RW
#define AXI4LITE_REG_HEX5_4         0x24 // RW

// Masks
#define MASK_SW                     0x3FF
#define MASK_LED                    0x3FF
#define MASK_KEY                    0xF
#define MASK_HEX0                   0x7F
#define MASK_HEX1                   0x7F00
#define MASK_HEX2                   0x7F0000
#define MASK_HEX3                   0x7F000000
#define MASK_HEX4                   0x7F
#define MASK_HEX5                   0x7F00
#define MASK_HEX3_0                 (MASK_HEX0 | MASK_HEX1 | MASK_HEX2 | MASK_HEX3)
#define MASK_HEX5_4                 (MASK_HEX4 | MASK_HEX5)

// Bit to shift
#define SHIFT_KEY                    1
#define SHIFT_HEX                    8

// IOCTL commands : divide offset by 4 (>> 2) since they are 32-bit aligned
#define AXI4LITE_NB                 'X'
#define AXI4LITE_READ(reg)          _IOR(AXI4LITE_NB, (reg) >> 2, uint32_t)
#define AXI4LITE_WRITE(reg)         _IOW(AXI4LITE_NB, (reg) >> 2, uint32_t) 

#endif // AXI4LITE_IOCTL_H