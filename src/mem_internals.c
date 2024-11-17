/******************************************************
 * Copyright Grégory Mounié 2018-2022                 *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include <sys/mman.h>
#include <assert.h>
#include <stdint.h>
#include "mem.h"
#include "mem_internals.h"
#include <string.h>

unsigned long knuth_mmix_one_round(unsigned long in)
{
    return in * 6364136223846793005UL % 1442695040888963407UL;
}


void *mark_memarea_and_get_user_ptr(void *ptr, unsigned long size, MemKind k)
{
    unsigned long valeur_magique = knuth_mmix_one_round((unsigned long)ptr);
    valeur_magique = (valeur_magique & ~(0b11UL)) | k;
    *((unsigned long *)ptr) = size;
    *((unsigned long *)(ptr + sizeof(unsigned long))) = valeur_magique;
    *((unsigned long *)(ptr + size - 2*sizeof(unsigned long))) = valeur_magique;
    *((unsigned long *)(ptr + size - sizeof(unsigned long))) = size; 

    return ptr + 2*sizeof(unsigned long);  
}


Alloc mark_check_and_get_alloc(void *ptr)
{
    Alloc a = {};
    unsigned long magic_init = *(unsigned long *)((uint8_t *) ptr - sizeof(unsigned long));
    unsigned long size_init = *(unsigned long *)((uint8_t *) ptr-2*sizeof(unsigned long));
    unsigned long magic_end = *(unsigned long *)((uint8_t *) ptr + size_init - 4*sizeof(unsigned long));
    unsigned long size_end = *(unsigned long *)((uint8_t *) ptr + size_init - 4*sizeof(unsigned long) + sizeof(unsigned long));
    assert(magic_init == magic_end);
    assert(size_init == size_end);
    MemKind memK = (MemKind) magic_init & 0b11UL;
    a.ptr = (void *) ((uintptr_t *)ptr-2);
    a.kind = memK;
    a.size = size_init;
    return a;

}

unsigned long
mem_realloc_small() {
    assert(arena.chunkpool == 0);
    unsigned long size = (FIRST_ALLOC_SMALL << arena.small_next_exponant);
    arena.chunkpool = mmap(0,
			   size,
			   PROT_READ | PROT_WRITE | PROT_EXEC,
			   MAP_PRIVATE | MAP_ANONYMOUS,
			   -1,
			   0);
    if (arena.chunkpool == MAP_FAILED)
	handle_fatalError("small realloc");
    arena.small_next_exponant++;
    return size;
}

unsigned long
mem_realloc_medium() {
    uint32_t indice = FIRST_ALLOC_MEDIUM_EXPOSANT + arena.medium_next_exponant;
    assert(arena.TZL[indice] == 0);
    unsigned long size = (FIRST_ALLOC_MEDIUM << arena.medium_next_exponant);
    assert( size == (1UL << indice));
    arena.TZL[indice] = mmap(0,
			     size*2, // twice the size to allign
			     PROT_READ | PROT_WRITE | PROT_EXEC,
			     MAP_PRIVATE | MAP_ANONYMOUS,
			     -1,
			     0);
    if (arena.TZL[indice] == MAP_FAILED)
	handle_fatalError("medium realloc");
    // align allocation to a multiple of the size
    // for buddy algo
    arena.TZL[indice] += (size - (((intptr_t)arena.TZL[indice]) % size));
    arena.medium_next_exponant++;
    return size; // lie on allocation size, but never free
}


// used for test in buddy algo
unsigned int
nb_TZL_entries() {
    int nb = 0;
    
    for(int i=0; i < TZL_SIZE; i++)
	if ( arena.TZL[i] )
	    nb ++;

    return nb;
}
