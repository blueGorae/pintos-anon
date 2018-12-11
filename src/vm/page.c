#include "vm/page.h"
#include "userprog/process.h"

static bool install_page (void *upage, void *kpage, bool writable);
 
static unsigned page_hash_func (const struct hash_elem *e, void *aux UNUSED)
{
  struct s_pte *spte = hash_entry(e, struct s_pte, elem);
  return hash_int((int) spte->page);
}

static bool page_less_func (const struct hash_elem *a, const struct hash_elem *b, void *aux UNUSED)
{
  struct s_pte *spte_a = hash_entry(a, struct s_pte, elem);
  struct s_pte *spte_b = hash_entry(b, struct s_pte, elem);
  if (spte_a->page < spte_b->page)
      return true;

  return false;
}


void s_page_table_init(struct hash * s_page_table){
    hash_init (s_page_table, page_hash_func, page_less_func, NULL);
}

struct s_pte* s_pte_alloc(struct file * file, off_t offset, uint32_t page_read_bytes, uint32_t page_zero_bytes, bool writable, void * page){
    struct s_pte * spte = (struct s_pte *) malloc(sizeof(struct s_pte));
 
    spte-> file = file;
    spte -> offset = offset;
    spte -> page_read_bytes = page_read_bytes;
    spte -> page_zero_bytes = page_zero_bytes;
    spte -> writable = writable;
    spte -> page = page;
    spte -> is_loaded = false;

    hash_insert(&thread_current()->s_page_table, &spte->elem);

    return spte;
}

static void s_page_action_func (struct hash_elem *e, void *aux UNUSED)
{
    //printf("page free called \n");
  struct s_pte *spte = hash_entry(e, struct s_pte, elem);
    // if(spte->is_loaded)
    //     palloc_free_page(fte_search_by_spte(spte)->frame);
    //}
    free(spte);
}

void s_page_table_destroy(){
    hash_destroy (&thread_current()->s_page_table, s_page_action_func);
}

struct s_pte* s_pte_search_by_vaddr(void* vaddr){
    struct s_pte spte;
    spte.page = pg_round_down(vaddr);
    struct hash_elem *e = hash_find(&thread_current()->s_page_table, &spte.elem);
    

    if (e == NULL)
        return NULL;

    return hash_entry (e, struct s_pte, elem);
}

bool load_page(void * vaddr){
    struct s_pte * spte = s_pte_search_by_vaddr(vaddr);
    enum palloc_flags flags = PAL_USER;
    if(spte == NULL){
        return false;
    }
    if(spte -> is_loaded)
        return true;

    if (spte-> page_read_bytes == 0){
        flags = PAL_USER | PAL_ZERO;
    }

    void * frame = palloc_get_page(flags);
    
    
    if(frame == NULL){
        frame_evict();
	    frame =  palloc_get_page(flags);
    }

    fte_search_by_frame(frame)->spte = spte;


    /* Load this page. */
    if( (flags & PAL_ZERO) == 0){
        file_seek(spte->file, spte->offset);
        lock_acquire(&file_lock);
        //file_seek(spte->cur_file_info-> file, spte-> cur_file_info-> offset);
       //file_seek(spte->file, spte->offset);

        if (file_read (spte->file, frame, spte -> page_read_bytes) != (int) spte -> page_read_bytes)
        {
            lock_release(&file_lock);
            palloc_free_page(frame);
            return false; 
        }
        lock_release(&file_lock);

        memset (frame + spte->page_read_bytes, 0, spte-> page_zero_bytes);
    }
    /* Add the page to the process's address space. */
    if (!install_page (spte-> page, frame, spte->writable)) 
    {
        palloc_free_page(frame);
        return false; 
    }
    spte-> is_loaded = true;

    //printf("new frame : %p , loaded page : %p \n", frame, spte->page);

    return true;
}

// bool is_loaded(void * page){
//    struct s_pte * spte = s_pte_search_by_vaddr(page); 
//     if(fte_search_by_spte(spte) == NULL)
//         return false;
    
//     return true;
// }

static bool
install_page (void *upage, void *kpage, bool writable)
{
  struct thread *t = thread_current ();
  /* Verify that there's not already a page at that virtual
     address, then map our page there. */
  return (pagedir_get_page (t->pagedir, upage) == NULL
          && pagedir_set_page (t->pagedir, upage, kpage, writable));
}

bool stack_growth(void* vaddr)
{
  //check the size of stack
  size_t new_size = PHYS_BASE - pg_round_down(vaddr);
  if(new_size > (1 << 23)) return false;

  struct s_pte* spte = s_pte_alloc(NULL, NULL, NULL, NULL,true, pg_round_down(vaddr));
  spte->is_loaded = true;

  uint8_t* frame = palloc_get_page(PAL_USER);
  if(frame == NULL)
  {
    free(spte);
    return false;
  }

  fte_search_by_frame(frame)->spte = spte;
  if(!install_page(spte->page, frame, spte->writable))
  {
    free(spte);
    palloc_free_page(frame);
    return false;
  }

  return true;

}




