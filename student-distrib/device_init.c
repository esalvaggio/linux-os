#include "lib.h"
#include "device_init.h"


//https://wiki.osdev.org/RTC

void RTC_Init(){
  char prev;
  //disable_ints();			// disable interrupts (cli)
  outb(NMI_MASK | REG_B, CMOS_REG);		// select register B, and disable NMI
  prev = inb(PIC_REG);	// read the current value of register B
  outb(NMI_MASK | REG_B, CMOS_REG);		// set the index again (a read will reset the index to register D)
  outb(prev | SET_PIE, PIC_REG);	// write the previous value OR'd with 0x40. This turns on bit 6 of register B
//  enable_ints(); (sti)

}

void Keyboard_Init(){

  
}
