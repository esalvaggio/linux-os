
#include "lib.h"
#include "idt_setup.h"



int handlerFunction(int vectorNumber)
{
  switch(vectorNumber)
  {
    case 0:
      printf("Divide by Zero");
      break;

    case 1:
      printf("Debug");
      break;

    case 2:
      printf("Not Used");
      break;

    case 3:
      printf("Breakpoint");
      break;

    case 4:
      printf("Overflow");
      break;

    case 5:
      printf("Bounds Check");
      break;

    case 6:
      printf("Invalid Opcode");
      break;

    case 7:
      printf("Device Not Available");
      break;

    case 8:
      printf("Double Fault");
      break;

    case 9:
      printf("Coprocessor Segment Overrun");
      break;

    case 10:
      printf("Invalid TSS");
      break;

    case 11:
      printf("Segment Not Present");
      break;

    case 12:
      printf("Stack Segment Fault");
      break;

    case 13:
      printf("General Protection");
      break;

    case 14:
      printf("Page Fault");
      break;

    case 15:
      printf("Reserved");
      break;

    case 16:
      printf("Floating Point Error");
      break;

    case 17:
      printf("Alignment Check Fault");
      break;

    case 18:
      printf("Machine Check");
      break;

    case 19:
      printf("SIMD Floating Point Exception");
      break;
  }
  return 0;
}






void create_IDT_entry(idt_desc_t * entry)
{
//dpl of exceptions/interrupts need to be set to 0
//dpl of system calls needs to be set to 3
//I have no idea how to assign the handler

  int x;
  for(x = 0; x < NUM_VEC; x++)
  {

    idt[x].seg_selector = KERNEL_CS;    // KERNEL_CS
    idt[x].reserved4    = 0x00;         // have to be 0x00
    idt[x].reserved3    = 0;            // 0
    idt[x].reserved2    = 1;            // 1
    idt[x].reserved1    = 1;            // 1
    idt[x].size         = 1;            // 0 for 16 bits, 1 for 32 bits
    idt[x].reserved0    = 0;            // 0
    idt[x].dpl          = 0;            // set to zero bc we want kernel privilege access

    if (x < 20)
      idt[x].present    = 1;            // set to 1 for intel defined interrupts
    else
      idt[x].present    = 0;            // set to 0 for other interrupts until we define them

    SET_IDT_ENTRY(idt[x], handlerFunction(x));

  }

  return;
}
