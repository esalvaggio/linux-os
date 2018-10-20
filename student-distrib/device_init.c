#include "lib.h"
#include "device_init.h"


#define STATUS_PORT     0x64
#define DATA_PORT       0x60


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

void Keyboard_Handler() {
  printf("Set keyboard handler 3\n");
  unsigned char status, output_key;
  char scan_code;

  send_eoi(1);

  status = inb(STATUS_PORT);

  if (status & 0x01) {
        scan_code = inb(DATA_PORT);
        if (scan_code < 0)
            return;

        output_key = keysofthekeys[scan_code];
        printf("%u", output_key);

  }
}

void Keyboard_Init() {
    // printf("Set keyboard handler\n");
    // printf("Set keyboard handler 1\n");
    SET_IDT_ENTRY(idt[33], Keyboard_Handler);
    // enable_irq(1);
    outb(0xFD, 0x21);
    // printf("Set keyboard handler 2\n");
}
