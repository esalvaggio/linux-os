#define ASM
.globl pit_setup

pit_setup:
  pushal
  call pit_handler
  popal
  iret
