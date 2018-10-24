#ifndef _FS_SETUP_H
#define _FS_SETUP_H

#include "types.h"

#define DENTRY_RESERVE_SIZE   24
#define FILENAME_SIZE         32
#define BOOT_BLOCK_RES_SIZE   52
#define NUM_OF_D_ENTRIES      63
#define NUM_OF_INODES       1023

/* User-defined structs for file system */
typedef struct dentry {
    uint8_t filename[FILENAME_SIZE];
    uint32_t filetype;
    uint32_t inode_num;
    uint8_t reserved[DENTRY_RESERVE_SIZE];
} dentry_t;

typedef struct boot_block {
    uint32_t dir_count;
    uint32_t inode_count;
    uint32_t data_count;
    uint8_t reserved[BOOT_BLOCK_RES_SIZE];
    dentry_t d_entries[NUM_OF_D_ENTRIES];
} boot_block_t;

typedef struct inode {
    uint32_t length;
    uint32_t data_block_num[NUM_OF_INODES];
} inode_t;


void fs_init(uint32_t module_addr);
int32_t read_dentry_by_name(const uint8_t fname, dentry_t* dentry);
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

/* Starting address of boot block */
uint32_t bb_startaddr;


// NEED: terminal read/write, rtc read/write, fs read/write //
// separate file? ^^ //

#endif
