#include "lib.h"
#include "paging.h"




void Paging_Init(){

  int i;
  for(i = 0; i<table_size; i++){
      page_directory[i] = 0; //change 0 to correct initialization value
      page_table[i] = 0; //change
  }
  page_directory[0] = ((uint32_t) page_table) + 0x06 + 0x01;
  page_directory[1] = 0x00400000 + 0x82 + 0x01;
  page_table[0xB8] = 0x000B8000 + 0x06 + 0x01;

  //Turn on paging, not yet initialized tables
  //input page directory in here
  /* moves page directory addr into cr3,cr4*/

  asm volatile ("               \n\
                  movl %0, %%eax \n\
                  movl %%eax, %%cr3 \n\
                  movl %%cr4, %%eax \n\
                  orl $0x10, %%eax \n\
                  movl %%eax, %%cr4 \n\
                  movl %%cr0, %%eax \n\
                  orl  $0x80000001, %%eax \n\
                  movl %%eax, %%cr0 \n\
                  "
                :             // (no) ouput
                : "r"(page_directory)   // page_directory as input. r means go through a regster
                : "eax"    // clobbered register
                );

}
