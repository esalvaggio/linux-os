#ifndef PAGING_H
#define PAGING_H

void Paging_Init();

#define table_size 1024
uint32_t page_directory[1024];
uint32_t page_table[1024];



#endif
