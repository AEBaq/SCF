#ifndef AXI4LITE_IOCTL_H
#define AXI4LITE_IOCTL_H

#include <linux/ioctl.h>
#include <stdint.h>


#define AXI_HPS_FPGA_BASE_ADD   0xFF200000
#define AXI_MAP_SIZE            0x1000



#define REG_CST                 0x0000
#define REG_TEST                0x0004 
#define REG_KEYS                0x0008 
#define REG_KEYS_EDGE_CAPTURE   0x000C
#define REG_SWITCHS             0x0010 
#define REG_LEDS                0x0014 
#define REG_LEDS_OUTSET         0x0018
#define REG_LEDS_OUTCLR         0x001C   
#define REG_HEX3_0              0x0020  
#define REG_HEX5_4              0x0024  



#define BITS_SWITCHS    0x000003FF
#define BITS_LEDS       0x000003FF
#define BITS_KEYS       0x0000000F

#define BITS_HEX0       0x0000007F
#define BITS_HEX1       0x00007F00
#define BITS_HEX2       0x007F0000
#define BITS_HEX3       0x7F000000
#define BITS_HEX4       0x0000007F
#define BITS_HEX5       0x00007F00

#define BITS_HEX3_0     (BITS_HEX0 | BITS_HEX1 | BITS_HEX2 | BITS_HEX3)
#define BITS_HEX5_4     (BITS_HEX4 | BITS_HEX5)

#define HEX_BITS_OFFSET 8


#define KEY0    0x1
#define KEY1    0x2
#define KEY2    0x4
#define KEY3    0x8

/* ------------------------------------------------------------------ */
/*  Interface ioctl                                                     */
/*                                                                      */
/*  L'offset du registre est encodé dans le numéro de commande (NR).   */
/*  On divise par 4 (>> 2) car les offsets sont alignés sur 32 bits.   */
/*                                                                      */
/*  Usage :                                                             */
/*    uint32_t val;                                                     */
/*    ioctl(fd, AXI4LITE_READ(REG_LEDS),  &val);   // lecture         */
/*    ioctl(fd, AXI4LITE_WRITE(REG_LEDS), &val);   // écriture        */
/* ------------------------------------------------------------------ */
#define AXI4LITE_MAGIC          'X'
#define AXI4LITE_READ(reg)      _IOR(AXI4LITE_MAGIC, (reg) >> 2, uint32_t)
#define AXI4LITE_WRITE(reg)     _IOW(AXI4LITE_MAGIC, (reg) >> 2, uint32_t)

#endif /* AXI4LITE_IOCTL_H */