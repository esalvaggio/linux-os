#include "rtc.h"



#define STATUS_PORT     0x64
#define DATA_PORT       0x60
#define SLAVE_IRQ          2
#define RTC_IRQ            8
#define RTC_INDEX         40

volatile int int_flag;
//https://wiki.osdev.org/RTC

/* RTC_INIT
 * Inputs: none
 * OUTPUTS: none
 * This function initializes the RTC by setting the appropriate IDT entry and
 * reading from/writing to Register B
 */
void RTC_Init(){
    int_flag = 1;
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

    outb(NMI_MASK | REG_A, CMOS_REG); // set index to A
    prev = inb(PIC_REG);	// get initial value of register A
    outb(NMI_MASK | REG_A, CMOS_REG); // reset index to A
    outb((prev) | BOT_4, PIC_REG); // set rtc rate to 2hz

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
    //printf("1");   /* Uncomment to test RTC */
    outb(REG_C, CMOS_REG);	     // select register C
    inb(PIC_REG);		             // just throw away contents
    //test_interrupts();          /* Uncomment to test RTC with test_interrupts */
    sti();

    send_eoi(RTC_IRQ); //rtc port on slave
    int_flag = 1;
}


/* RTC_write
 *
 * Inputs: NBYTES is the new rate to set into the rtc.
 *         Note: the lower the rate, the faster the frequency
 * Outputs: 0 on success, -1 on failure (bad input)
 * This function takes an input rate and changes the rtc timer to tick at that new
 * rate. Interrupts are masked for the few operations with reading/writing with
 * the RTC.
 * Page 19 of datasheet
*/
int32_t RTC_write(void* buf, int32_t nbytes){
  char prev;
  int power = power_of_two(nbytes);
  if (power < LOW_RATE || power > HI_RATE)return ERROR;

  power = 16-power; //calculate the proper bits to write to rtc

  cli(); ////////////////////////////////////////////////////////////////
  outb(NMI_MASK | REG_A, CMOS_REG); // set index to A
  prev = inb(PIC_REG);	// get initial value of register A
  outb(NMI_MASK | REG_A, CMOS_REG); // reset index to A
  outb((prev & TOP_4) | power, PIC_REG); // write new rate to register A -> lower 4 bits
  sti(); //////////////////////////////////////////////////////////NOT SURE IF RIGHT
  return SUCCESS;
}
/* square_root
 * helper function for RTC_write.
 * Input = integer such that we want to find n in:
 *          2^(n) = input
 * Outputs: -1 if not a power of 2, n otherwise
 * Side effects: none
 *
*/
int32_t power_of_two(int32_t input){
    int32_t count = 0;
    while (input > 2){

        /* For example,
         * (float)(5 / 2) = 2.00
         * (5/2.0) = 2.50, so 5 is not a power of 2
         *
        */
        if ((float)(input / 2) != input/2.0) return ERROR;
        //else, divide by 2 and keep going
        input /= 2;
        count ++;
    }
    count++;
    return count;
}

int32_t RTC_open(){
    RTC_write(NULL, 1);
    return SUCCESS;
}

//Need to wait for interrupt
int32_t RTC_read(void* buf, int32_t nbytes){
    int_flag = 0;
    //while(int_flag == 0){} -> doesn't work for some reason
    return SUCCESS;
}
int32_t RTC_close(){
    return SUCCESS;
}
