#include "vm/page.h"
#include "userprog/process.h"

#define STACK_MAX (1 << 23) //256kb

static bool install_page (void *upage, void *kpage, bool writable);
 
static unsigned page_hash_func (const struct hash_elem *e, void *aux UNUSED)
{
  struct s_pte *spte = hash_entry(e, struct s_pte, elem);
  return hash_int((int) spte->vaddr);
}

static bool page_less_func (const struct hash_elem *a, const struct hash_elem *b, void *aux UNUSED)
{
  struct s_pte *spte_a = hash_entry(a, struct s_pte, elem);
  struct s_pte *spte_b = hash_entry(b, struct s_pte, elem);
  if (spte_a->vaddr < spte_b->vaddr)
      return true;

  return false;
}


void s_page_table_init(struct hash * s_page_table){
    hash_init (s_page_table, page_hash_func, page_less_func, NULL);

}

struct s_pte* s_pte_alloc(struct cur_file_info * cur_file_info, void * vaddr){
    struct s_pte * spte = (struct s_pte *) malloc(sizeof(struct s_pte));
    spte->cur_file_info =  cur_file_info;
    spte -> vaddr = vaddr;
    hash_insert(&thread_current()->s_page_table, &spte->elem);
    //printf("number of hash entry : %d \n", thread_current()->s_page_table.elem_cnt);
    return spte;
}

static void s_page_action_func (struct hash_elem *e, void *aux UNUSED)
{
    //printf("page free called \n");
  struct s_pte *spte = hash_entry(e, struct s_pte, elem);
  //if(is_loaded(spte->vaddr)){

      fte_free(fte_search_by_spte(spte)->frame);
      pagedir_clear_page(thread_current()->pagedir, spte->vaddr);
//  }
  free(spte);
}

void s_page_table_destroy(){
    hash_destroy (&thread_current()->s_page_table, s_page_action_func);
}


struct s_pte* s_pte_search_by_vaddr(void* vaddr){
    struct s_pte spte;
    spte.vaddr = pg_round_down(vaddr);
    struct hash_elem *e = hash_find(&thread_current()->s_page_table, &spte.elem);
    if (e == NULL)
        return NULL;

    return hash_entry (e, struct s_pte, elem);
}

bool load_page(void * vaddr){

    struct s_pte * spte = s_pte_search_by_vaddr(vaddr);
    enum palloc_flags flags = PAL_USER;
    if(spte == NULL){
        printf("error 0 \n");
        return false;
    }

    if (spte->cur_file_info-> page_read_bytes == 0)
    {
        flags == PAL_USER & PAL_ZERO;
    }

    void * frame = fte_alloc(flags, spte)->frame;
    printf("new frame %p and loaded page %p\n", frame, spte->vaddr);
    if(frame == NULL){
        printf("error 0 \n");
        frame_evict();
        frame = fte_alloc(flags, spte)->frame;
    }

    /* Load this page. */

    if (file_read (spte->cur_file_info->file, frame, spte->cur_file_info -> page_read_bytes) != (int) spte->cur_file_info -> page_read_bytes)
    {
        fte_free(frame);
        printf("error 1 \n");
        return false; 
    }

    memset (frame + spte->cur_file_info->page_read_bytes, 0, spte->cur_file_info-> page_zero_bytes);

    /* Add the page to the process's address space. */
    if (!install_page (spte-> vaddr, frame, spte->cur_file_info->writable)) 
    {
        fte_free (frame);
        printf("error 2 \n");

        return false; 
    }

    return true;

}

bool is_loaded(void * page){
   struct s_pte * spte = s_pte_search_by_vaddr(page); 
    if(fte_search_by_spte(spte) == NULL)
        return false;
    
    return true;
}

static bool
install_page (void *upage, void *kpage, bool writable)
{
  struct thread *t = thread_current ();

  /* Verify that there's not already a page at that virtual
     address, then map our page there. */
  return (pagedir_get_page (t->pagedir, upage) == NULL
          && pagedir_set_page (t->pagedir, upage, kpage, writable));
}