
#include "lib.h"
#include "idt_setup.h"


void divide_by_zero() {
  printf("divide by zero\n");
  while(1);
}

void debug() {
  printf("debug\n");
  while(1);
}

void not_used() {
  printf("not used\n");
  while(1);
}

void breakpoint() {
  printf("breakpoint\n");
  while(1);
}

void overflow() {
  printf("overflow\n");
  while(1);
}

void bounds_check() {
  printf("bounds check\n");
  while(1);
}

void invalid_opcode() {
  printf("invalid opcode\n");
  while(1);
}

void device_not_available() {
  printf("device not available\n");
  while(1);
}

void double_fault() {
  printf("double fault\n");
  while(1);
}

void cop_seg_overrun() {
  printf("coprocessor segment overrun\n");
  while(1);
}

void invalid_tss() {
  printf("invalid tss\n");
  while(1);
}

void seg_not_present() {
  printf("segment not present\n");
  while(1);
}

void stack_seg_fault() {
  printf("stack seg fault\n");
  while(1);
}

void general_protection() {
  printf("general protection\n");
  while(1);
}

void page_fault() {
  printf("page fault\n");
  while(1);
}

void reserved() {
  printf("reserved\n");
  while(1);
}

void floating_point_err() {
  printf("floating point error\n");
  while(1);
}

void aligment_check_fault() {
  printf("aligment check fault\n");
  while(1);
}

void machine_check() {
  printf("machine check\n");
  while(1);
}

void simd_exception() {
  printf("simd floating point exeption\n");
  while(1);
}



void create_IDT_entry()
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

  if (x < 32)
    idt[x].present    = 1;            // set to 1 for intel defined interrupts
  else
    idt[x].present    = 0;            // set to 0 for other interrupts until we define them

}

SET_IDT_ENTRY(idt[0], divide_by_zero);
SET_IDT_ENTRY(idt[1], debug);
SET_IDT_ENTRY(idt[2], not_used);
SET_IDT_ENTRY(idt[3], breakpoint);
SET_IDT_ENTRY(idt[4], overflow);
SET_IDT_ENTRY(idt[5], bounds_check);
SET_IDT_ENTRY(idt[6], invalid_opcode);
SET_IDT_ENTRY(idt[7], device_not_available);
SET_IDT_ENTRY(idt[8], double_fault);
SET_IDT_ENTRY(idt[9], cop_seg_overrun);
SET_IDT_ENTRY(idt[10], invalid_tss);
SET_IDT_ENTRY(idt[11], seg_not_present);
SET_IDT_ENTRY(idt[12], stack_seg_fault);
SET_IDT_ENTRY(idt[13], general_protection);
SET_IDT_ENTRY(idt[14], page_fault);
SET_IDT_ENTRY(idt[15], reserved);
SET_IDT_ENTRY(idt[16], floating_point_err);
SET_IDT_ENTRY(idt[17], aligment_check_fault);
SET_IDT_ENTRY(idt[18], machine_check);
SET_IDT_ENTRY(idt[19], simd_exception);



// for(x = 20; x < NUM_VEC; x++)
// {
//
//   idt[x].seg_selector = KERNEL_CS;    // KERNEL_CS
//   idt[x].reserved4    = 0x00;         // have to be 0x00
//   idt[x].reserved3    = 0;            // 0
//   idt[x].reserved2    = 1;            // 1
//   idt[x].reserved1    = 1;            // 1
//   idt[x].size         = 1;            // 0 for 16 bits, 1 for 32 bits
//   idt[x].reserved0    = 0;            // 0
//   idt[x].dpl          = 0;            // set to zero bc we want kernel privilege access
//
//   if (x < 32)
//     idt[x].present    = 1;            // set to 1 for intel defined interrupts
//   else
//     idt[x].present    = 0;            // set to 0 for other interrupts until we define them
//
// }

}
