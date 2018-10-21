#ifndef PAGING_H
#define PAGING_H

void Paging_Init();

#define table_size 1024
#define KERNEL_LOAD_ADDR 0x400000
#define PRESENT_BIT 0x01
#define SHIFTED_VGA_ADDR 0xB8
#define USER        0x04
#define RE_WR       0x02
#define PAGE_SIZE   0x80
#define VIDEO       0xB8000



uint32_t page_directory[1024] __attribute__((aligned (4096)));
uint32_t page_table[1024] __attribute__((aligned (4096)));



#endif
