/*****************************************************************************************
 * HEIG-VD
 * Haute Ecole d'Ingenerie et de Gestion du Canton de Vaud
 * School of Business and Engineering in Canton de Vaud
 *****************************************************************************************
 * REDS Institute
 * Reconfigurable Embedded Digital Systems
 *****************************************************************************************
 *
 * File                 : hps_gpio.c
 * Author               : 
 * Date                 : 
 *
 * Context              : SCF tutorial lab
 *
 *****************************************************************************************
 * Brief: light HPS user LED up when HPS user button pressed, for DE1-SoC board
 *
 *****************************************************************************************
 * Modifications :
 * Ver    Date        Student      Comments
 * 1.0    18.02.26    Nox          Lab01
 *
*****************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define OSC1_Timer_Module0_Addr 0xFFD00000 // Base Address
#define GPIO1_Addr 0xFF709000 // Base Address

#define timer1controlreg_offset 0x8
#define timer1loadcount_offset 0x0

#define timer1control_reg (OSC1_Timer_Module0_Addr + timer1controlreg_offset)
#define timer1loadcount_reg (OSC1_Timer_Module0_Addr + timer1loadcount_offset)

#define timer1_enable_bit 0x01
#define timer1_mode_bit 0x2
#define timer1_interrupt_mask_bit 0x4


void timer_init(uint32_t load_value){
	 // Disable timer
	*(volatile uint32_t *)timer1control_reg &= ~(uint32_t)timer1_enable_bit;

    // Set load value
    *(volatile uint32_t *)timer1loadcount_reg = load_value;

    // Set timer mode to user defined count mode
    *(volatile uint32_t *)timer1control_reg |= timer1_mode_bit;

    // Mask timer interrupts
    *(volatile uint32_t *)timer1control_reg &= ~(uint32_t)timer1_interrupt_mask_bit;

    // Enable timer
    *(volatile uint32_t *)timer1control_reg |= timer1_enable_bit;
}

int main(void){
    
    uint32_t repetition_i, time_i;

    printf("\nEnter the number of blinks wanted: ");
    scanf("%u", &repetition_i);
    printf("\nEnter the time in seconds: ");
    scanf("%u", &time_i);

    timer_init(time_i * repetition_i * 1000000); // Assuming the timer counts in microseconds



}
