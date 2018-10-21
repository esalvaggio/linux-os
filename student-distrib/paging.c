#include "lib.h"
#include "paging.h"


/*
creates a page directory and page table, initializes both to 0,
creates 2 elements in page directory, one in 4KB and one in 4MB
creates page table entry (page) in the 4kB page memory
asm code puts page directory address into cr0

*/

void Paging_Init(){

//we initially set all the values to 0, which marks it all as unusable, then later
//define the usuable areas
  int i;
  for(i = 0; i<table_size; i++){
      page_directory[i] = 0; //change 0 to correct initialization value
      page_table[i] = 0; //change
  }
  page_directory[0] = ((uint32_t) page_table) + RE_WR + USER + PRESENT_BIT; //4kB
  page_directory[1] = KERNEL_LOAD_ADDR + PAGE_SIZE + RE_WR + PRESENT_BIT; //Kernel 4MB address
  page_table[SHIFTED_VGA_ADDR] = VIDEO + RE_WR + USER + PRESENT_BIT; //VGA 4kB address

  //Turn on paging, not yet initialized tables
  //input page directory in here
  /* moves page directory addr into cr3,cr4*/

  asm volatile ("                         \n\
                  movl %0, %%eax          \n\
                  movl %%eax, %%cr3       \n\
                  movl %%cr4, %%eax       \n\
                  orl $0x10, %%eax        \n\
                  movl %%eax, %%cr4       \n\
                  movl %%cr0, %%eax       \n\
                  orl  $0x80000001, %%eax \n\
                  movl %%eax, %%cr0       \n\
                  "
                :             // (no) ouput
                : "r"(page_directory)   // page_directory as input. r means go through a regster
                : "eax"    // clobbered register
                );

}
