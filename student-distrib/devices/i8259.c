/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"


/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* Initialize the 8259 PIC */

//i8259_init
//Input: none
//Output: none
//This function initializes the PIC by sending the ICW defined constants in the h file
//to their respective master and slave ports
void i8259_init(void) {
    master_mask = 0xFF; //set to all 1's
    slave_mask = 0xFF;

    // initialize the master and slave pics
    outb(ICW1, MASTER_8259_PORT);
    outb(ICW1, SLAVE_8259_PORT);

    // send ICW2
    outb(ICW2_MASTER, MASTER_8259_DATA);
    outb(ICW2_SLAVE, SLAVE_8259_DATA);

    // send ICW3
    outb(ICW3_MASTER, MASTER_8259_DATA);
    outb(ICW3_SLAVE, SLAVE_8259_DATA);

    // send ICW4
    outb(ICW4, MASTER_8259_DATA);
    outb(ICW4, SLAVE_8259_DATA);

    // all done, clear out data registers
    outb(master_mask, MASTER_8259_DATA);
    outb(slave_mask, SLAVE_8259_DATA);
}

/* Enable (unmask) the specified IRQ */

//Enable_IRQ
//Input: IRQ number to unmask
//Output: none
//This function unmasks the specified IRQ in order to receive interrupts from that
//irq number. If the number is greater than 8 it should go to the slave, otherwise it
//goes to the master
void enable_irq(uint32_t irq_num) {

  if((irq_num >= 16) || (irq_num < 0)) //invalid irq_num, irq_num is invalid if >= 16 or if negative
  {
    return;
  }

  uint16_t port;
  uint8_t data;

  if(irq_num < EIGHT) { //send to master
      port = MASTER_8259_DATA;
  } else {
      port = SLAVE_8259_DATA; //send to slave
      irq_num -= EIGHT; //subtract 8 to get the slave's number
  }
  data = inb(port) & ~(0x01 << irq_num);
  outb(data, port); //write to proper port
}

/* Disable (mask) the specified IRQ */
//Disable_IRQ
//Input: IRQ number to mask
//Output: none
//This function masks the specified IRQ in order to prevent interrupts from that
//irq number. If the number is greater than 8, it should go to the slave, otherwise it
//goes to the master
void disable_irq(uint32_t irq_num) {

  if((irq_num >= 16) || (irq_num < 0)) //invalid irq_num, irq_num is invalid if >= 16 or if negative
  {
    return;
  }

  uint16_t port;
  uint8_t data;

  if(irq_num < EIGHT) { //send to master
      port = MASTER_8259_DATA;
  } else {
      port = SLAVE_8259_DATA; //send to slave
      irq_num -= EIGHT;
  }
  data = inb(port) | (0x01 << irq_num);

  outb(data, port); //write to proper port
}

/* Send end-of-interrupt signal for the specified IRQ */
/*
Uses OCW2 (specified on page 13 of 8259A datasheet)
[R, SL, EOI, 0, 0 ,L2, L1, L0]
Where R = 0, SL == EOI == 1 (0x60) for top byte,
and L0-L2 specify address of interrupt
*/

//Send_EOI
//Input: irq_num to send EOI
//Output: none
//This function sends an EOI from the given IRQ number, signifying that an interrupt has been
//properly handled. If irq_num is >=8, EOI needs to be sent from both the master and the slave
void send_eoi(uint32_t irq_num) {

  if((irq_num >= 16) || (irq_num < 0)) //invalid irq_num, irq_num is invalid if >= 16 or if negative
  {
    return;
  }


    // check if the irq came from a slave pic, still do both commands
    if (irq_num >= EIGHT){
      irq_num -= EIGHT;
      outb(EOI | irq_num, SLAVE_8259_PORT); //write to slave
      irq_num = 2; //slave port on master
    }
    outb(EOI | irq_num, MASTER_8259_PORT); //write to master
}
