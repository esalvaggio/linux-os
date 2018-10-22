#ifndef PAGING_H
#define PAGING_H

void Paging_Init();

#define TABLE_SIZE 1024
#define AMT_BYTE     4096
#define KERNEL_LOAD_ADDR 0x400000
#define PRESENT_BIT 0x01
#define SHIFTED_VGA_ADDR 0xB8
#define USER        0x04
#define RE_WR       0x02
#define PAGE_SIZE   0x80
#define VIDEO       0xB8000



uint32_t page_directory[TABLE_SIZE] __attribute__((aligned (AMT_BYTE)));
uint32_t page_table[TABLE_SIZE] __attribute__((aligned (AMT_BYTE)));



#endif
