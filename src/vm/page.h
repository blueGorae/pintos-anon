#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <hash.h>
#include <stdbool.h>
#include "filesys/file.h"
#include "threads/palloc.h"
#include "threads/thread.h"
#include "threads/vaddr.h"

#include "userprog/pagedir.h"
#include "userprog/process.h"
#include "userprog/syscall.h"
#include "vm/frame.h"
#include "vm/page.h"

struct s_pte{
    void * page;

    struct file * file;
    off_t offset;
    uint32_t page_read_bytes;
    uint32_t page_zero_bytes; 
    bool writable;

    bool is_loaded;
    struct hash_elem elem;

};

void s_page_table_init(struct hash * s_page_table);

struct s_pte* s_pte_alloc(struct file * file, off_t offset, uint32_t page_read_bytes, uint32_t page_zero_bytes, bool writable, void * page);

void s_page_table_destroy();

struct s_pte* s_pte_search_by_page(void* vaddr);

bool load_page(void * vaddr);

//bool is_loaded(void * page);

#endif /* vm/page.h */
