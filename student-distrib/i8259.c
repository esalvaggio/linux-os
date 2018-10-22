/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"


/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* Initialize the 8259 PIC */
void i8259_init(void) {
    master_mask = 0xFF;
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
void enable_irq(uint32_t irq_num) {
  uint16_t port;
  uint8_t data;

  if(irq_num < 8) {
      port = MASTER_8259_DATA;
  } else {
      port = SLAVE_8259_DATA;
      irq_num -= 8;
  }
  data = inb(port) & ~(0x01 << irq_num);
  outb(data, port);
}

/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {
  uint16_t port;
  uint8_t data;

  if(irq_num < 8) {
      port = MASTER_8259_DATA;
  } else {
      port = SLAVE_8259_DATA;
      irq_num -= 8;
  }
  data = inb(port) | (0x01 << irq_num);

  outb(data, port);
}

/* Send end-of-interrupt signal for the specified IRQ */
/*
Uses OCW2 (specified on page 13 of 8259A datasheet)
[R, SL, EOI, 0, 0 ,L2, L1, L0]
Where R = 0, SL == EOI == 1 (0x60) for top byte,
and L0-L2 specify address of interrupt
*/
void send_eoi(uint32_t irq_num) {

    // check if the irq came from a slave pic, still do both commands
    if (irq_num >= 8){
      irq_num -= 8;
      outb(EOI | irq_num, SLAVE_8259_PORT);
      irq_num = 2;
    }
    outb(EOI | irq_num, MASTER_8259_PORT);
}
