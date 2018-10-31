
#include "lib.h"
#include "idt_setup.h"

/* Exception handlers */
void divide_by_zero() {
  printf("EXCEPTION: division by zero.\n");
  while(1);
}

void debug() {
  printf("debug exception\n");
  while(1);
}

void not_used() {
  printf("not used exception\n");
  while(1);
}

void breakpoint() {
  printf("breakpoint exception\n");
  while(1);
}

void overflow() {
  printf("overflow\n");
  while(1);
}

void bounds_check() {
  printf("bounds check exception\n");
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
  printf("EXCEPTION: Page fault.\n");
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

void system_call_handler() {
  printf("system call handler\n");
  while(1);
}

void slave_handler(){
  printf("Slave handler\n");
  while(1);
}

/* create_IDT_entry
 *
 * Initializes the IDT, and sets the index to their
 * respective handler functions
 *
 * Inputs: None
 * Outputs: None
 * Side Effects: Sets up IDT
 */
void create_IDT_entry()
{

  int x;
  for(x = 0; x < NUM_VEC; x++)
  {

    idt[x].seg_selector = KERNEL_CS;    // KERNEL_CS
    idt[x].reserved4    = 0x00;         // have to be 0x00
    idt[x].reserved2    = 1;            // 1
    idt[x].reserved1    = 1;            // 1
    idt[x].size         = 1;            // 0 for 16 bits, 1 for 32 bits
    idt[x].reserved0    = 0;            // 0
    idt[x].dpl          = 0;            // set to zero bc we want kernel privilege access

    /* set the intel-defined exceptions to be used */
    if (x < INTEL_DEFINED_IDX) {
      idt[x].present    = 1;            // set to 1 for intel defined interrupts
      idt[x].reserved3  = 1;            /* set to 1 to indicate trap gate */
    } else {
      idt[x].present    = 0;            // set to 0 for other interrupts until we define them
      idt[x].reserved3  = 0;            /* set to 0 to indicate interrupt gate */
    }

    /* set the slave pic to be used (index 34) */
    if (x == SLAVE_HANDLER_IDT)
      idt[x].present = 1;

    /* initialize the system call handler to be used (index 127) */
    if (x == SYSTEM_CALL_IDT) {
      idt[x].present    = 1;
      idt[x].dpl        = 3;            // dp = 3 because we want user-level access
    }

  }

  SET_IDT_ENTRY(idt[DIVIDE_BY_ZERO_IDT], divide_by_zero);
  SET_IDT_ENTRY(idt[DEBUG_IDT], debug);
  SET_IDT_ENTRY(idt[NOT_USED_IDT], not_used);
  SET_IDT_ENTRY(idt[BREAKPOINT_IDT], breakpoint);
  SET_IDT_ENTRY(idt[OVERFLOW_IDT], overflow);
  SET_IDT_ENTRY(idt[BOUNDS_CHECK_IDT], bounds_check);
  SET_IDT_ENTRY(idt[INVALID_OPCODE_IDT], invalid_opcode);
  SET_IDT_ENTRY(idt[DEV_NOT_AVAILABLE_IDT], device_not_available);
  SET_IDT_ENTRY(idt[DOUBLE_FAULT_IDT], double_fault);
  SET_IDT_ENTRY(idt[COP_SEG_OVERRUN_IDT], cop_seg_overrun);
  SET_IDT_ENTRY(idt[INVALID_TSS_IDT], invalid_tss);
  SET_IDT_ENTRY(idt[SEG_NOT_PRESENT_IDT], seg_not_present);
  SET_IDT_ENTRY(idt[STACK_SEG_FAULT_IDT], stack_seg_fault);
  SET_IDT_ENTRY(idt[GEN_PROTECTION_IDT], general_protection);
  SET_IDT_ENTRY(idt[PAGE_FAULT_IDT], page_fault);
  SET_IDT_ENTRY(idt[RESERVED_IDT], reserved);
  SET_IDT_ENTRY(idt[FLT_POINT_ERROR_IDT], floating_point_err);
  SET_IDT_ENTRY(idt[ALIGN_FAULT_IDT_IDT], aligment_check_fault);
  SET_IDT_ENTRY(idt[MACHINE_CHECK_IDT], machine_check);
  SET_IDT_ENTRY(idt[SIMD_EXCEPTION_IDT], simd_exception);
  SET_IDT_ENTRY(idt[SYSTEM_CALL_IDT], system_call_handler);
  /*
  SET_IDT_ENTRY(idt[SLAVE_HANDLER_IDT], slave_handler);*/
}
