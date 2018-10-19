/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"


/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* Initialize the 8259 PIC */
void i8259_init(void)
{
  //mask PIC
  // asm(
  //   "mov %al, 0xff \n\t"
  //   "out 0xa1, %al \n\t"
  //   "out 0x21, %al \n\t"
  // );

  outb(master_mask, MASTER_8259_PORT);
  outb(slave_mask, SLAVE_8259_PORT);

  outb(ICW1, MASTER_8259_PORT); //ICW1 to Master and slave ports
  outb(ICW1, SLAVE_8259_PORT);

  outb(ICW2_MASTER, MASTER_8259_DATA); //ICW2 Master/Slave to respective data
  outb(ICW2_SLAVE, SLAVE_8259_DATA);

  outb(ICW3_MASTER,MASTER_8259_DATA); //ICW3 Master/Slave to respective data
  outb(ICW3_SLAVE, SLAVE_8259_DATA);

  outb(ICW4, MASTER_8259_DATA); //ICW4 to Master and slave ports
  outb(ICW4, SLAVE_8259_DATA);


}

/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num)
{
  uint16_t port;
  uint8_t value;

  if(irq_num < 8)
  {
    port = MASTER_8259_DATA;
  }
else
{
    port = SLAVE_8259_DATA;
    irq_num -= 8;
}
value = inb(port) | (1 << irq_num);
outb(value, port);
}

/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num)
{
uint16_t port;
uint8_t value;

if(irq_num < 8)
{
    port = MASTER_8259_DATA;
}
else
{
    port = SLAVE_8259_DATA;
    irq_num -= 8;
}
value = inb(port) & ~(1 << irq_num);
outb(value,port);
}

/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num)
{
  if(irq_num >= 8)
  {
    outb(EOI, SLAVE_8259_PORT); //IRQ 8-15 slave
  }
  else
  {
    outb(EOI, MASTER_8259_PORT); //IRQ 0-7 = master
  }
}
