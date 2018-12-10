#include "frame.h"

void frame_table_init(){
    list_init(&frame_table);
}

struct fte* fte_alloc(enum palloc_flags flags, struct s_pte * spte){
    
    struct fte* fte = (struct fte *)malloc(sizeof(struct fte));
    void * frame  = palloc_get_page(flags);

    if(frame == NULL){
        frame_evict();
        frame = palloc_get_page(flags);
    }
    
    fte -> frame = frame;
    fte -> spte = spte;
    fte -> thread = thread_current();
    fte -> allocatable = true;
    list_push_back(&frame_table, &fte->elem);
    return fte;
}

void fte_free(void * frame){

    lock_acquire(&frame_lock);
    struct fte * fte = fte_search_by_frame(frame);
   // printf("frame free called \n");
    if(fte == NULL)
        return;
    list_remove(&fte->elem);
    palloc_free_page(fte->frame);
    free(fte);
    lock_release(&frame_lock);
}

struct fte* fte_search_by_frame(void * frame){
    struct list_elem *e;

    for(e = list_begin(&frame_table) ; e != list_end(&frame_table) ; e = list_next(e)){
        struct fte * fte = list_entry(e, struct fte, elem);
        if(fte->frame == frame)
            return fte;
    }
    return NULL;
}

struct fte* fte_search_by_spte(struct s_pte * spte){
    struct list_elem *e;

    for(e = list_begin(&frame_table) ; e != list_end(&frame_table) ; e = list_next(e)){
        struct fte * fte = list_entry(e, struct fte, elem);
        if(fte->spte == spte)
            return fte;
    }
    return NULL;
}

void frame_evict(){
    //use clock algorithm
    //now FIFO
    
    struct list_elem * e;

    e = list_begin(&frame_table);

    struct fte* fte = list_entry(e, struct fte, elem);
    void * frame = fte-> frame;
    
    fte_free(frame);
}