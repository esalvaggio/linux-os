#define ASM     1
#include "x86_desc.h"
// #include "sys_calls.h"


.globl sys_call_handler
.globl system_call_jump_table
system_call_jump_table:
  .long halt, execute, read, write, open, close, getargs, vidmap

  #invalid check for eax less than 0, greater than 7, add invalid
sys_call_handler:

  movl  %esp, %ebp
  pushl %edi
  pushl %esi
  pushl %ebx
  pushl %ecx
  pushl %edx

  cmpl $0, %eax
  jb invalid
  cmpl $7, %eax
  ja invalid
  call *system_call_jump_table(,%eax,4)


  popl %edx
  popl %ecx
  popl %ebx
  popl %esi
  popl %edi

  leave
  ret

invalid:
  movl $-1, %eax
  leave
  ret
