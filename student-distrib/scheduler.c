#include "scheduler.h"
#include "terminals.h"

/*
  This might all be wrong.
*/

void switch_processes(){
  /*
  term_t* terminal = get_curr_terminal();
  int current_terminal =

  uint32_t entry_point, stack_pointer, ds, cs;
  entry_point = 0x0;
  stack_pointer = 0x0;
  ds = 0x0;
  cs = 0x0;

  asm volatile ("                         \n\
                  movw %2, %%ax           \n\
                  movw %%ax, %%ds         \n\
                  movw %%ax, %%es         \n\
                  movw %%ax, %%fs         \n\
                  movw %%ax, %%gs         \n\
                  pushl %2                \n\
                  pushl %1                \n\
                  pushfl                  \n\
                  popl %%eax              \n\
                  orl $0x00000200,%%eax   \n\
                  pushl %%eax             \n\
                  pushl %3                \n\
                  pushl %0                \n\
                  iret                    \n\
                  "
                  :                 // (no) ouput
                  : "r"(entry_point), "r"(stack_pointer), "r"(ds), "r"(cs)
                  : "eax","edx"    // clobbered register
                );
    */
    // We need to return to the parent esp/ebp that should be saved in the pcb structure
    /*
    tss.esp0 = pcb_processes[old_num]->esp0;
    tss.ss0 = pcb_processes[old_num]->ss0; //kernel stack segment = kernel_DS


    // We need to restore the previous ebp (entering EXECUTE) in order to return

    int32_t old_ebp = pcb_processes[current_num]->ebp;

    //Finally, Clear pcb_processes[current_num]
    pcb_processes[current_num] = 0x0;
     // Update the number of pcbs in the terminal
    curr_terminal->num_of_pcbs--;
    //Changes ebp to value when we entered execute, and jump back to execute
    asm volatile ("                         \n\
                    movl %0, %%EBP          \n\
                    movl $0, %%eax          \n\
                    jmp END_OF_EXECUTE      \n\
                    "
                    :
                    : "r"(old_ebp)               // (no) ouput
                  );
    */
}
