#include "device_init.h"

#define STATUS_PORT     0x64
#define DATA_PORT       0x60

#define KEYBOARD_INDEX 33
#define RTC_INDEX   40




//https://wiki.osdev.org/RTC




//RTC_INIT
//Inputs: none
//OUTPUTS: none
//This function initializes the RTC by setting the appropriate IDT entry and
//reading from/writing to Register B
void RTC_Init(){
    cli();
    char prev;
    //idt[34].present = 1;
    idt[RTC_INDEX].present = 1;
    SET_IDT_ENTRY(idt[RTC_INDEX], RTC_Handler); //index 40 is the RTC in the IDT

    outb(NMI_MASK | REG_B, CMOS_REG);		// select register B, and disable NMI
    prev = inb(PIC_REG);	// read the current value of register B
    outb(NMI_MASK | REG_B, CMOS_REG);		// set the index again (a read will reset the index to register D)
    outb(prev | SET_PIE, PIC_REG);	// write the previous value OR'd with 0x40. This turns on bit 6 of register B
    outb(REG_C, CMOS_REG);	// select register C
    inb(PIC_REG);		// just throw away contents


    enable_irq(2); //enables slave port on master
    enable_irq(8); //enables rtc port on slave
    sti();
}

//RTC_Handler
//Inputs: none
//Outputs: none
//This function reads from Register C in order to accept interrupts from the RTC,
//and then sends an EOI to alert the PIC
void RTC_Handler(){

    outb(REG_C, CMOS_REG);	// select register C
    inb(PIC_REG);		// just throw away contents
    sti();

    send_eoi(8); //rtc port on slave
}

//Keyboard_Handler
//inputs: none
//outputs: none
//This function reads in data from the keyboard, maps the key to the ascii,
//and then displays it on screen. It finishes by sending EOI
void Keyboard_Handler() {
  cli();
    unsigned char status, output_key;
    char scan_code;

    status = inb(STATUS_PORT);

    if (status & 0x01) { //last value of status is the character to be displayed
          scan_code = inb(DATA_PORT);
          //printf("scan code: %d\n", scan_code);

          if (scan_code >= 0) { //read something from the keyboard
            output_key = keyboard_map[(unsigned char)scan_code]; //map the key
            printf("%c", output_key); //print to screen
          }


    }
    sti();
    send_eoi(1); //keyboard port on master


}


//Keyboard_Init
//Inputs: none
//Outputs: none
//This function sets the appropriate entry in the IDT to enable the keyboard and also
//enables the irq in the master PIC
void Keyboard_Init() {
    idt[KEYBOARD_INDEX].present = 1; //index is present
    SET_IDT_ENTRY(idt[KEYBOARD_INDEX], Keyboard_Handler); //index 33 is the keyboard in the IDT
    enable_irq(1); //keyboard port on master
}
