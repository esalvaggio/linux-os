#define ASM     1
#include "x86_desc.h"

# note: refer to syscalls/ece391syscall.S for the actual system call


# Inputs:
# eax = number of interrupt
# ebx = first argument
# ecx = second argument
# edx = third argument

.global sys_call_handler
# .globl system_call_jump_table
system_call_jump_table:
  .long 0x000000, halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn

  # EAX will have a number between 1 and 10 inclusive,
  #   halt = 1
  #   execute = 2. . .
  #   sigreturn = 10
  #   As defined in syscalls/ece391sysnum.h
  #
sys_call_handler:

  cli
  # might need to subtract 1 from eax to get jump table to work

  cmpl $1, %eax
  jl invalid
  cmpl $7, %eax
  jge invalid

  pushl %ebp
  pushl %edi
  pushl %esi
  pushl %edx
  pushl %ecx
  pushl %ebx

  sti
  call *system_call_jump_table(,%eax,4)
  cli

  popl %ebx
  popl %ecx
  popl %edx
  popl %esi
  popl %edi
  popl %ebp

end:
  sti
  iret

invalid:
  movl $-1, %eax
  jmp end
