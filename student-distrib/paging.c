#include "lib.h"
#include "paging.h"


/* Paging Initialization
 *
 * Creates a page directory and page table, initializes both to 0.
 * Creates 2 elements in page directory, one in 4KB and one in 4MB.
 * Creates page table entry (page) in the 4kB page Video memory.
 * Asm code puts page directory address into cr0, sets up 4MB mem.
 */

void Paging_Init(){

//we initially set all the values to 0, which marks it all as unusable, then later
//define the usuable areas
  int i;
  for(i = 0; i<TABLE_SIZE; i++){
      page_directory[i] = 0; //change 0 to correct initialization value
      page_table[i] = 0; //change
      vidmap_pagetable[i] = 0;
  }
  page_directory[0] = ((uint32_t) page_table) + RE_WR + USER + PRESENT_BIT; //4kB
  page_directory[1] = KERNEL_LOAD_ADDR + PAGE_SIZE + RE_WR + PRESENT_BIT; //Kernel 4MB address
  for (i = 0; i < 4; i++) {
      page_table[SHIFTED_VGA_ADDR + i] = VIDEO + RE_WR + USER + PRESENT_BIT + (i*FOURKB); //VGA 4kB
  }

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

void page_dir_init(uint32_t virtual_addr, uint32_t phys_addr){
  uint32_t index = virtual_addr / FOURMB;
  page_directory[index] = phys_addr + PAGE_SIZE + RE_WR + PRESENT_BIT + USER;
  flushTLB();
}

void page_dir_init_fourkb(uint32_t virtual_addr, uint32_t phys_addr){
  uint32_t index = virtual_addr / FOURMB;
  page_directory[index] = ((uint32_t) vidmap_pagetable) + RE_WR + USER + PRESENT_BIT;
  vidmap_pagetable[0] = phys_addr + RE_WR + USER + PRESENT_BIT;
  flushTLB();
}

void flushTLB(void){
  asm volatile ("                           \n\
                    movl	%%cr3,%%eax       \n\
                    movl	%%eax,%%cr3       \n\
                  "
                :             // (no) ouput
                :             // page_directory as input. r means go through a regster
                : "eax"       // clobbered register
                );
}
