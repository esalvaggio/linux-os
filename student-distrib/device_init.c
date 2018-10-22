#include "device_init.h"

#define STATUS_PORT     0x64
#define DATA_PORT       0x60

#define VIDEO       0xB8000
#define NUM_COLS    80
#define NUM_ROWS    25


static char* video_mem = (char *)VIDEO;

//https://wiki.osdev.org/RTC


/*
RTC init is working, but throws an simd_exception
'segment not present' when the idt is called.
*/
void test_interrupts2(void) {
    int32_t i;
    for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
        video_mem[i << 1]++;
    }
}
void RTC_Init(){
    cli();
    char prev;
    //idt[34].present = 1;
    idt[40].present = 1;
    SET_IDT_ENTRY(idt[40], RTC_Handler);

    outb(NMI_MASK | REG_B, CMOS_REG);		// select register B, and disable NMI
    prev = inb(PIC_REG);	// read the current value of register B
    outb(NMI_MASK | REG_B, CMOS_REG);		// set the index again (a read will reset the index to register D)
    outb(prev | SET_PIE, PIC_REG);	// write the previous value OR'd with 0x40. This turns on bit 6 of register B
    outb(REG_C, CMOS_REG);	// select register C
    inb(PIC_REG);		// just throw away contents


    enable_irq(2);
    enable_irq(8);
    sti();
}
//This has never been called
void RTC_Handler(){

    outb(REG_C, CMOS_REG);	// select register C
    inb(PIC_REG);		// just throw away contents
    sti();

    send_eoi(8);
}

void Keyboard_Handler() {

    unsigned char status, output_key;
    char scan_code;

    status = inb(STATUS_PORT);

    if (status & 0x01) {
          scan_code = inb(DATA_PORT);
          //printf("scan code: %d\n", scan_code);

          if (scan_code >= 0) {
            output_key = keyboard_map[(unsigned char)scan_code];
            printf("%c", output_key);
          }

          scan_code = inb(DATA_PORT);
    }
    sti();
    send_eoi(1);


}

void Keyboard_Init() {
    idt[33].present = 1;
    SET_IDT_ENTRY(idt[33], Keyboard_Handler);
    enable_irq(1);
}
