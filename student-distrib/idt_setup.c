#include "idt_setup.h"

void create_IDT_entry(idt_desc_t * entry)
{
//dpl of exceptions/interrupts need to be set to 0
//dpl of system calls needs to be set to 3
  int x;

  for(x = 0; x < NUM_VEC; x++)
  {
    idt[x].offset_15_00 = 1;
    idt[x].offset_31_16 = 2;
    //SET_IDT_ENTRY(idt[x], handler);
  }

  return;
}
