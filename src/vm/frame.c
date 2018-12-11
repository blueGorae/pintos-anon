#include "frame.h"
#include "threads/malloc.h"

void frame_table_init(){
    list_init(&frame_table);
}

struct fte* fte_alloc(void * page){
    lock_acquire(&frame_lock);
    struct fte* fte = (struct fte *)malloc(sizeof(struct fte));
    printf("frame alloc %p \n", page);
    fte -> frame = page;
    fte -> thread = thread_current();
    fte -> allocatable = true;
    list_push_back(&frame_table, &fte->elem);
    lock_release(&frame_lock);
    return fte;
}
void fte_free(void * frame){

    printf("fte free called %p \n", frame);
    lock_acquire(&frame_lock);
    struct fte * fte = fte_search_by_frame(frame);
    
    fte -> spte ->is_loaded = false;

    list_remove(&fte->elem);
    free(fte);
    lock_release(&frame_lock);
}

struct fte* fte_search_by_frame(void * frame){
    struct list_elem *e;

    for(e = list_begin(&frame_table) ; e != list_end(&frame_table) ; e = list_next(e)){
    if(e == NULL)
        return NULL;
	struct fte * fte = list_entry(e, struct fte, elem);
    
	if(fte-> frame == frame)
	    return fte;
    }
    return NULL;
}

struct fte* fte_search_by_spte(struct s_pte * spte){
   
    struct list_elem *e;

    for(e = list_begin(&frame_table) ; e != list_end(&frame_table) ; e = list_next(e)){
	if(e == NULL)
        return NULL;
    struct fte * fte = list_entry(e, struct fte, elem);
	if(fte-> spte == spte)
	    return fte;
    }
    return NULL;
}


void frame_evict(){
    //use clock algorithm
    //now FIFO
    
    lock_acquire(&frame_lock);
    struct list_elem * e;

    e = list_begin(&frame_table);

    struct fte* fte = list_entry(e, struct fte, elem);
    void * frame = fte-> frame;
    
    palloc_free_page(frame);
    lock_release(&frame_lock);
}