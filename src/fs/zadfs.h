#ifndef ZADFS_H
#define ZADFS_H

#include "../kernel/types.h"

#define ZADFS_MAX_FILES      128
#define ZADFS_MAX_FILENAME   32
#define ZADFS_MAX_PATH       128
#define ZADFS_DATA_SIZE      4096
#define ZADFS_MAGIC          0x5ADF55
#define ZADFS_SECTOR_SIZE    512

typedef enum { ZADFS_FILE=0, ZADFS_DIR=1 } zadfs_type_t;

typedef struct zadfs_entry {
    char name[ZADFS_MAX_FILENAME];
    zadfs_type_t type;
    int size;
    int data_offset;
    int parent;
    int first_child;
    int next_sibling;
    uint8_t used;
} zadfs_entry_t;

typedef struct {
    int magic;
    int root_idx;
    int num_entries;
    int next_data_offset;
    int hdd_mode;
    zadfs_entry_t entries[ZADFS_MAX_FILES];
    char data[ZADFS_DATA_SIZE];
} zadfs_t;

extern zadfs_t zadfs;

void zadfs_init(void);
void zadfs_mkdir(const char *path);
void zadfs_ls(const char *path, int cwd_idx);
void zadfs_create_file(const char *path, const char *content, int cwd_idx);
void zadfs_cat(const char *path, int cwd_idx);
void zadfs_rm(const char *path, int cwd_idx);
void zadfs_cp(const char *src, const char *dst, int cwd_idx);
void zadfs_save_to_hdd(void);
void zadfs_load_from_hdd(void);
void zadfs_get_cwd_path(int idx, char *out);
int zadfs_find(const char *path);

#endif
