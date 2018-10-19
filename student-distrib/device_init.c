#include "lib.h"
#include "device_init.h"


//https://wiki.osdev.org/8259_PIC


void setPIC()
{





// mask PIC
// asm(
//   "mov %al, 0xff \n\t"
//   "out 0xa1, %al \n\t"
//   "out 0x21, %al \n\t"
// );

PIC_send_EOI(1);

}

void PIC_send_EOI(int irq)
{
  if(irq >= 8)
  {
    outb(SLAVE, PIC_EOI); //IRQ 8-15 = slave
  }
  else
  {
    outb(PIC, PIC_EOI); //IRQ 0-7 = master
  }
}
