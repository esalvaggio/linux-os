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
    int8_t filename[FILENAME_SIZE];
    int32_t filetype;
    int32_t inode_num;
    int8_t reserved[DENTRY_RESERVE_SIZE];
} dentry_t;

typedef struct boot_block {
    int32_t dir_count;
    int32_t inode_count;
    int32_t data_count;
    int8_t reserved[BOOT_BLOCK_RES_SIZE];
    dentry_t d_entries[NUM_OF_D_ENTRIES];
} boot_block_t;

typedef struct inode {
    int32_t length;
    int32_t data_block_num[NUM_OF_INODES];
} inode_t;


void fs_init(uint32_t module_addr);
int32_t read_dentry_by_name(const int8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index(int32_t index, dentry_t* dentry);
int32_t read_data(int32_t inode, int32_t offset, int8_t* buf, int32_t length);

/* Helper function for copying filename arrays */
void copy_string(int8_t* dest, const int8_t* src, uint32_t n);

/* Starting address of boot block */
boot_block_t* boot_block;

// NEED: terminal read/write, rtc read/write, fs read/write //
// separate file? ^^ //

#endif
