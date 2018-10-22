#include "lib.h"
#include "device_init.h"



//https://wiki.osdev.org/RTC


/*
RTC init is working, but throws an simd_exception
'segment not present' when the idt is called.
*/
void RTC_Init(){
    char prev;
    idt[34].present = 1;
    idt[40].present = 1;
    outb(NMI_MASK | REG_B, CMOS_REG);		// select register B, and disable NMI
    prev = inb(PIC_REG);	// read the current value of register B
    outb(NMI_MASK | REG_B, CMOS_REG);		// set the index again (a read will reset the index to register D)
    outb(prev | SET_PIE, PIC_REG);	// write the previous value OR'd with 0x40. This turns on bit 6 of register B
    outb(0x0C, CMOS_REG);	// select register C
    inb(PIC_REG);		// just throw away contents
    SET_IDT_ENTRY(idt[40], RTC_Handler);
    enable_irq(2);
    enable_irq(8);

}
//This has never been called
void RTC_Handler(){
    printf("RTC Interrupt");
    send_eoi(8);
    outb(0x0C, CMOS_REG);	// select register C
    inb(PIC_REG);		// just throw away contents
}
