#include "lib.h"
#include "paging.h"

void Paging_Init(){

  int i;
  for(i = 0; i++; i<table_size){
      page_directory[i] = 0; //change 0 to correct initialization value
      page_table[i] = 0; //change

  }
  
  //Turn on paging, not yet initialized tables
  //input page directory in here
  /* moves page directory addr into cr3,cr4*/
  asm volatile ("             \n\
                  movl %0, %%eax    \n\
                  movl %%eax, %%cr  3   \n\
                  movl %%cr4, %%eax  \n\
                  orl 0x00000010,  %%eax \n\
                  movl %%eax, %%cr4  \n\
                "
                :             /* (no) ouput */
                : "r"(page_directory)   // page_directory as input. r means go through a regster
                : "%eax"    /* clobbered register */
                );
}
