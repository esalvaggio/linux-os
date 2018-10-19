#include "lib.h"
#include "device_init.h"


//mask PIC
// asm volatile(
//   "mov al, 0xff \n
//   out 0xa1, al \n
//   out 0x21, al" \n
// );

void PIC_send_EOI(int irq)
{
  if(irq >= 8)
  {
    outb(SLAVE, PIC_EOI);
  }
  else
  {
    outb(PIC, PIC_EOI);
  }
}
