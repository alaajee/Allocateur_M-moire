/******************************************************
 * Copyright Grégory Mounié 2018                      *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include <assert.h>
#include "mem.h"
#include "mem_internals.h"


void *
emalloc_small(unsigned long size)
{
    if (arena.chunkpool == NULL){
        unsigned long new_block_size = mem_realloc_small();
        void *new_chunk = arena.chunkpool;
        void **next_chunk;
        while (new_block_size >= CHUNKSIZE){
            next_chunk = (void **)(new_chunk);
            new_block_size -= CHUNKSIZE;
            if (new_block_size >= CHUNKSIZE){
                *next_chunk = (void *)(new_chunk + CHUNKSIZE);
            } else {
                *next_chunk = NULL;
            }
            new_chunk = (void *)(new_chunk + CHUNKSIZE);
        }
    }

    void *chunk = arena.chunkpool;
    arena.chunkpool = *(void **)chunk;
    return mark_memarea_and_get_user_ptr(chunk, CHUNKSIZE, SMALL_KIND);
}


void efree_small(Alloc a) {
    void *chunk = a.ptr;
    *(void **) chunk = arena.chunkpool;
    arena.chunkpool = chunk;
}
