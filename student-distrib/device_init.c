#include "device_init.h"

#define STATUS_PORT     0x64
#define DATA_PORT       0x60
#define LOW_BITMASK     0x01
#define KEYBOARD_IRQ       1
#define SLAVE_IRQ          2
#define RTC_IRQ            8
#define KEYBOARD_INDEX    33
#define RTC_INDEX         40

//https://wiki.osdev.org/RTC

unsigned char keyboard_map[KB_MAP_SIZE] = {
                                    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',
                                    '9', '0', '-', '=', '\b','\t','q', 'w', 'e', 'r',
                                    't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
                                    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
                                   '\'', '`',   0,'\\', 'z', 'x', 'c', 'v', 'b', 'n',
                                    'm', ',', '.', '/',   0,'*',0,' ',0
                                  };

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

/* Keyboard_Handler
 * inputs: none
 * outputs: none
 * This function reads in data from the keyboard, maps the key to the ascii,
 * and then displays it on screen. It finishes by sending EOI
 */
void Keyboard_Handler() {
    cli();
    unsigned char status, output_key;
    char scan_code;

    status = inb(STATUS_PORT);

    if (status & LOW_BITMASK) {  // get last bit value of status is the character to be displayed
          scan_code = inb(DATA_PORT);

          if (scan_code >= 0)
          {   //read something from the keyboard
            output_key = keyboard_map[(unsigned char)scan_code]; //map the key
            printf("%c", output_key);   //print to screen
          }
    }
    sti();
    send_eoi(KEYBOARD_IRQ);   //keyboard port on master
}


/* Keyboard_Init
 * Inputs: none
 * Outputs: none
 * This function sets the appropriate entry in the IDT to enable the keyboard and also
 * enables the irq in the master PIC
 */
void Keyboard_Init() {
    idt[KEYBOARD_INDEX].present = 1; //index is present
    SET_IDT_ENTRY(idt[KEYBOARD_INDEX], Keyboard_Handler); //index 33 is the keyboard in the IDT
    enable_irq(KEYBOARD_IRQ); //keyboard port on master
}
