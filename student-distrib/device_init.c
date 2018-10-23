#include "device_init.h"

#define STATUS_PORT     0x64
#define DATA_PORT       0x60
#define SLAVE_IRQ          2
#define RTC_IRQ            8
#define RTC_INDEX         40

//https://wiki.osdev.org/RTC

/* RTC_INIT
 * Inputs: none
 * OUTPUTS: none
 * This function initializes the RTC by setting the appropriate IDT entry and
 * reading from/writing to Register B
 */
void RTC_Init(){
    cli();
    char prev;

    idt[RTC_INDEX].present = 1;
    SET_IDT_ENTRY(idt[RTC_INDEX], RTC_Handler); //index 40 is the RTC in the IDT

    outb(NMI_MASK | REG_B, CMOS_REG);		// select register B, and disable NMI
    prev = inb(PIC_REG);	// read the current value of register B
    outb(NMI_MASK | REG_B, CMOS_REG);		// set the index again (a read will reset the index to register D)
    outb(prev | SET_PIE, PIC_REG);	// write the previous value OR'd with 0x40. This turns on bit 6 of register B
    outb(REG_C, CMOS_REG);	// select register C
    inb(PIC_REG);		// just throw away contents

    enable_irq(SLAVE_IRQ); //enables slave port on master
    enable_irq(RTC_IRQ); //enables rtc port on slave
    sti();
}

/* RTC_Handler
 *
 * Inputs: none
 * Outputs: none
 * This function reads from Register C in order to accept interrupts from the RTC,
 * and then sends an EOI to alert the PIC
 */
void RTC_Handler(){
    // printf("RTC Interrupt ");   /* Uncomment to test RTC */
    outb(REG_C, CMOS_REG);	     // select register C
    inb(PIC_REG);		             // just throw away contents
    // test_interrupts();          /* Uncomment to test RTC with test_interrupts */
    sti();

    send_eoi(RTC_IRQ); //rtc port on slave
}
