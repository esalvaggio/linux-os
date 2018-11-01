#define ASM     1
#include "x86_desc.h"
#include "sys_calls.h"
#include "sys_call_handler.h"
/*
note: refer to syscalls/ece391syscall.S for the actual system call
*/

/*
Inputs:
eax = number of interrupt
ebx = first argument
ecx = second argument
edx = third argument

*/

.globl sys_call_handler
/*.globl system_call_jump_table*/
system_call_jump_table:
  .long halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn

  /*EAX will have a number between 1 and 10 inclusive,
    halt = 1
    execute = 2. . .
    sigreturn = 10
    As defined in syscalls/ece391sysnum.h
    */
sys_call_handler:

  pushl %ebp
  movl  %esp, %ebp

  /*might need to subtract 1 from eax to get jump table to work*/

  cmpl $0, %eax
  jb invalid
  cmpl $9, %eax
  ja invalid

  pushl %edi
  pushl %esi
  pushl %edx
  pushl %ecx
  pushl %ebx


  call *system_call_jump_table(,%eax,4)


  popl %ebx
  popl %ecx
  popl %edx
  popl %esi
  popl %edi
end:
  leave
  iret

invalid:
  movl $-1, %eax
  jmp end
