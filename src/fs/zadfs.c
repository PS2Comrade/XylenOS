#include "zadfs.h"
#include "../lib/string.h"
#include "../lib/memory.h"
#include "../drivers/vga.h"
#include "../drivers/ata.h"
#include <stdint.h>

zadfs_t zadfs;

static int zadfs_split_path(const char *path, char *parent_out, char *name_out) {
    int len = strlen(path);
    if(len==0 || path[0]!='/') return 0;
    int lastslash = -1;
    for(int i=0;i<len;i++) if(path[i]=='/') lastslash=i;
    if(lastslash==0) strcpy(parent_out,"/"); else { strncpy(parent_out,path,lastslash); parent_out[lastslash]=0; }
    strcpy(name_out,path+lastslash+1);
    return 1;
}

static int zadfs_alloc_entry() {
    for(int i=0;i<ZADFS_MAX_FILES;i++) if(!zadfs.entries[i].used) return i;
    return -1;
}

void zadfs_init() {
    zadfs.magic = ZADFS_MAGIC;
    zadfs.num_entries = 1;
    zadfs.next_data_offset = 0;
    zadfs.hdd_mode = 0;
    for(int i=0;i<ZADFS_MAX_FILES;i++) zadfs.entries[i].used=0;
    zadfs_entry_t *root = &zadfs.entries[0];
    root->used=1; root->type=ZADFS_DIR; root->parent=-1;
    root->name[0]=0; root->first_child=-1; root->next_sibling=-1; root->size=0;
    zadfs.root_idx = 0;
    for(int i=0;i<ZADFS_DATA_SIZE;i++) zadfs.data[i]=0;
}

int zadfs_find(const char *path) {
    if(!path || !*path || path[0]!='/') return -1;
    int idx = zadfs.root_idx;
    const char *p = path+1;
    char part[ZADFS_MAX_FILENAME];
    while(*p) {
        int j=0;
        while(*p && *p!='/') part[j++]=*p++;
        part[j]=0;
        if(*p=='/') p++;
        int found=-1;
        int child = zadfs.entries[idx].first_child;
        while(child!=-1) {
            if(zadfs.entries[child].used && strcmp(zadfs.entries[child].name,part)==0) {
                found=child; break;
            }
            child = zadfs.entries[child].next_sibling;
        }
        if(found==-1) return -1;
        idx=found;
    }
    return idx;
}

void zadfs_mkdir(const char *path) {
    char dirpath[ZADFS_MAX_PATH], dname[ZADFS_MAX_FILENAME];
    if(!zadfs_split_path(path, dirpath, dname)) { prints("Invalid path!\n"); return; }
    int parent_idx = zadfs_find(dirpath);
    if(parent_idx==-1 || zadfs.entries[parent_idx].type!=ZADFS_DIR) { prints("Parent dir not found!\n"); return; }
    int child = zadfs.entries[parent_idx].first_child;
    while(child!=-1) {
        if(zadfs.entries[child].used && strcmp(zadfs.entries[child].name,dname)==0) { prints("Already exists!\n"); return; }
        child = zadfs.entries[child].next_sibling;
    }
    int idx = zadfs_alloc_entry();
    if(idx==-1) { prints("Too many files!\n"); return; }
    zadfs_entry_t *e = &zadfs.entries[idx];
    e->used=1; e->type=ZADFS_DIR; e->parent=parent_idx; strncpy(e->name,dname,ZADFS_MAX_FILENAME);
    e->first_child=-1; e->next_sibling=zadfs.entries[parent_idx].first_child;
    zadfs.entries[parent_idx].first_child=idx; zadfs.entries[parent_idx].size++;
    zadfs.num_entries++;
    prints("Directory created!\n");
}

void zadfs_ls(const char *path, int cwd_idx) {
    int idx;
    if(path && *path) {
        if(path[0] == '/') idx = zadfs_find(path);
        else {
            char abs[ZADFS_MAX_PATH*2];
            if (cwd_idx == zadfs.root_idx) {
                abs[0] = '/'; abs[1] = 0;
                strncat(abs, path, ZADFS_MAX_PATH-2);
            } else {
                int stack[ZADFS_MAX_FILES], sp=0, walk=cwd_idx;
                while(walk!=-1 && walk!=zadfs.root_idx) { stack[sp++] = walk; walk = zadfs.entries[walk].parent; }
                abs[0] = '/'; abs[1] = 0;
                for(int i=sp-1;i>=0;i--) { strncat(abs, zadfs.entries[stack[i]].name, ZADFS_MAX_FILENAME-1); strncat(abs, "/", 1); }
                strncat(abs, path, ZADFS_MAX_PATH-2);
            }
            idx = zadfs_find(abs);
        }
    } else idx = cwd_idx;
    if(idx==-1 || zadfs.entries[idx].type!=ZADFS_DIR) { prints("No such directory!\n"); return; }
    prints("Contents:\n");
    int child = zadfs.entries[idx].first_child, empty=1;
    while(child!=-1) {
        zadfs_entry_t *e = &zadfs.entries[child];
        if(e->used) { prints("  "); prints(e->name); if(e->type==ZADFS_DIR) prints("/"); prints("\n"); empty=0; }
        child = e->next_sibling;
    }
    if(empty) prints("  (empty)\n");
}

void zadfs_create_file(const char *path, const char *content, int cwd_idx) {
    char dirpath[ZADFS_MAX_PATH], fname[ZADFS_MAX_FILENAME];
    if(!zadfs_split_path(path, dirpath, fname)) { prints("Invalid path!\n"); return; }
    int parent_idx;
    if(dirpath[0] == '/') parent_idx = zadfs_find(dirpath);
    else {
        char abs[ZADFS_MAX_PATH*2];
        if (cwd_idx == zadfs.root_idx) { abs[0] = '/'; abs[1] = 0; strncat(abs, dirpath, ZADFS_MAX_PATH-2); }
        else {
            int stack[ZADFS_MAX_FILES], sp=0, walk=cwd_idx;
            while(walk!=-1 && walk!=zadfs.root_idx) { stack[sp++] = walk; walk = zadfs.entries[walk].parent; }
            abs[0] = '/'; abs[1] = 0;
            for(int i=sp-1;i>=0;i--) { strncat(abs, zadfs.entries[stack[i]].name, ZADFS_MAX_FILENAME-1); strncat(abs, "/", 1); }
            strncat(abs, dirpath, ZADFS_MAX_PATH-2);
        }
        parent_idx = zadfs_find(abs);
    }
    if(parent_idx==-1 || zadfs.entries[parent_idx].type!=ZADFS_DIR) { prints("Parent dir not found!\n"); return; }
    int child = zadfs.entries[parent_idx].first_child;
    while(child!=-1) {
        if(zadfs.entries[child].used && strcmp(zadfs.entries[child].name,fname)==0) { prints("Already exists!\n"); return; }
        child = zadfs.entries[child].next_sibling;
    }
    int idx = zadfs_alloc_entry();
    if(idx==-1) { prints("Too many files!\n"); return; }
    int len = strlen(content);
    if(zadfs.next_data_offset+len > ZADFS_DATA_SIZE) { prints("Out of space!\n"); return; }
    zadfs_entry_t *e = &zadfs.entries[idx];
    e->used=1; e->type=ZADFS_FILE; e->parent=parent_idx; strncpy(e->name,fname,ZADFS_MAX_FILENAME);
    e->size=len; e->data_offset=zadfs.next_data_offset; e->next_sibling=zadfs.entries[parent_idx].first_child;
    zadfs.entries[parent_idx].first_child=idx; zadfs.entries[parent_idx].size++;
    memcpy(zadfs.data+zadfs.next_data_offset, content, len);
    zadfs.next_data_offset+=len; zadfs.num_entries++;
    prints("File created!\n");
}

void zadfs_cat(const char *path, int cwd_idx) {
    int idx;
    if(path && *path) {
        if(path[0]=='/') idx = zadfs_find(path);
        else {
            char abs[ZADFS_MAX_PATH*2];
            if (cwd_idx == zadfs.root_idx) { abs[0] = '/'; abs[1] = 0; strncat(abs, path, ZADFS_MAX_PATH-2); }
            else {
                int stack[ZADFS_MAX_FILES], sp=0, walk=cwd_idx;
                while(walk!=-1 && walk!=zadfs.root_idx) { stack[sp++] = walk; walk = zadfs.entries[walk].parent; }
                abs[0] = '/'; abs[1] = 0;
                for(int i=sp-1;i>=0;i--) { strncat(abs, zadfs.entries[stack[i]].name, ZADFS_MAX_FILENAME-1); strncat(abs, "/", 1); }
                strncat(abs, path, ZADFS_MAX_PATH-2);
            }
            idx = zadfs_find(abs);
        }
    } else { prints("cat: missing file\n"); return; }
    if(idx==-1 || zadfs.entries[idx].type!=ZADFS_FILE) { prints("No such file!\n"); return; }
    zadfs_entry_t *e = &zadfs.entries[idx];
    for(int i=0;i<e->size;i++) putchar(zadfs.data[e->data_offset+i]);
    prints("\n");
}

void zadfs_rm(const char *path, int cwd_idx) {
    int idx;
    if(path && *path) {
        if(path[0]=='/') idx = zadfs_find(path);
        else {
            char abs[ZADFS_MAX_PATH*2];
            if (cwd_idx == zadfs.root_idx) { abs[0] = '/'; abs[1] = 0; strncat(abs, path, ZADFS_MAX_PATH-2); }
            else {
                int stack[ZADFS_MAX_FILES], sp=0, walk=cwd_idx;
                while(walk!=-1 && walk!=zadfs.root_idx) { stack[sp++] = walk; walk = zadfs.entries[walk].parent; }
                abs[0] = '/'; abs[1] = 0;
                for(int i=sp-1;i>=0;i--) { strncat(abs, zadfs.entries[stack[i]].name, ZADFS_MAX_FILENAME-1); strncat(abs, "/", 1); }
                strncat(abs, path, ZADFS_MAX_PATH-2);
            }
            idx = zadfs_find(abs);
        }
    } else { prints("rm: missing file/dir\n"); return; }
    if(idx==-1) { prints("No such entry!\n"); return; }
    zadfs_entry_t *e = &zadfs.entries[idx];
    if(e->type==ZADFS_DIR && e->first_child!=-1) { prints("Dir not empty!\n"); return; }
    int par = e->parent;
    int *link = &zadfs.entries[par].first_child;
    while(*link!=-1) {
        if(*link==idx) { *link=e->next_sibling; break; }
        link = &zadfs.entries[*link].next_sibling;
    }
    zadfs.entries[par].size--;
    e->used=0; zadfs.num_entries--;
    prints("Removed!\n");
}

void zadfs_cp(const char *src, const char *dst, int cwd_idx) {
    int idx;
    if(src && *src) {
        if(src[0]=='/') idx = zadfs_find(src);
        else {
            char abs[ZADFS_MAX_PATH*2];
            if (cwd_idx == zadfs.root_idx) { abs[0] = '/'; abs[1] = 0; strncat(abs, src, ZADFS_MAX_PATH-2); }
            else {
                int stack[ZADFS_MAX_FILES], sp=0, walk=cwd_idx;
                while(walk!=-1 && walk!=zadfs.root_idx) { stack[sp++] = walk; walk = zadfs.entries[walk].parent; }
                abs[0] = '/'; abs[1] = 0;
                for(int i=sp-1;i>=0;i--) { strncat(abs, zadfs.entries[stack[i]].name, ZADFS_MAX_FILENAME-1); strncat(abs, "/", 1); }
                strncat(abs, src, ZADFS_MAX_PATH-2);
            }
            idx = zadfs_find(abs);
        }
    } else { prints("cp: missing src\n"); return; }
    if(idx==-1 || zadfs.entries[idx].type!=ZADFS_FILE) { prints("No such file!\n"); return; }
    zadfs_entry_t *e = &zadfs.entries[idx];
    char buf[ZADFS_DATA_SIZE];
    memcpy(buf, zadfs.data+e->data_offset, e->size);
    buf[e->size]=0;
    zadfs_create_file(dst, buf, cwd_idx);
}

void zadfs_save_to_hdd() {
    uint8_t *p = (uint8_t*)&zadfs;
    int total = sizeof(zadfs_t);
    int sector = 0;
    for(int o=0;o<total;o+=ZADFS_SECTOR_SIZE,sector++) {
        uint8_t buf[ZADFS_SECTOR_SIZE];
        int len = (o+ZADFS_SECTOR_SIZE<=total?ZADFS_SECTOR_SIZE:total-o);
        memcpy(buf, p+o, len);
        if(len<ZADFS_SECTOR_SIZE) for(int i=len;i<ZADFS_SECTOR_SIZE;i++) buf[i]=0;
        ata_write_sector(sector, buf);
    }
}

void zadfs_load_from_hdd() {
    uint8_t *p = (uint8_t*)&zadfs;
    int total = sizeof(zadfs_t);
    int sector = 0;
    for(int o=0;o<total;o+=ZADFS_SECTOR_SIZE,sector++) {
        uint8_t buf[ZADFS_SECTOR_SIZE];
        ata_read_sector(sector, buf);
        int len = (o+ZADFS_SECTOR_SIZE<=total?ZADFS_SECTOR_SIZE:total-o);
        memcpy(p+o, buf, len);
    }
    if(zadfs.magic!=ZADFS_MAGIC) { prints("Invalid FS, formatting.\n"); zadfs_init(); }
}

void zadfs_get_cwd_path(int idx, char *out) {
    if(idx==0) { out[0] = '/'; out[1] = 0; return; }
    int stack[ZADFS_MAX_FILES], sp=0, walk=idx;
    while(walk!=-1 && walk!=zadfs.root_idx) { stack[sp++] = walk; walk = zadfs.entries[walk].parent; }
    out[0] = '/'; out[1] = 0;
    for(int i=sp-1;i>=0;i--) { strncat(out, zadfs.entries[stack[i]].name, ZADFS_MAX_FILENAME-1); strncat(out, "/", 1); }
    int l = strlen(out); if(l>1 && out[l-1]=='/') out[l-1]=0;
}
