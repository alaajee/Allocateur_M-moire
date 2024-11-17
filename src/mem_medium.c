/******************************************************
 * Copyright Grégory Mounié 2018                      *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include <stdint.h>
#include <assert.h>
#include "mem.h"
#include "mem_internals.h"
#include <math.h>

unsigned int puiss2(unsigned long size) {
    unsigned int p=0;
    size = size -1; // allocation start in 0
    while(size) {  // get the largest bit
	p++;
	size >>= 1;
    }
    if (size > (1 << p))
	p++;
    return p;
}


void *emalloc_medium(unsigned long size)
{
    // Vérifier les bornes de la taille
    assert(size < LARGEALLOC); 
    assert(size > SMALLALLOC);
    size += 32;
    size += 32 - (size % 32);
    unsigned int k = puiss2(size);
    if  (arena.TZL[k] != NULL){
        goto final;
    }

    while (arena.TZL[k] == NULL )
    {
        if (k+1 == FIRST_ALLOC_MEDIUM_EXPOSANT + arena.medium_next_exponant)
        {
            int indice = k+1;
            unsigned long bloc_size = mem_realloc_medium();
            void *currentTZL = arena.TZL[indice];
            
            for (int i = 0; i < bloc_size - (1UL << indice); i += (1UL << indice)) {
                void* TZLsuiv = currentTZL + (1UL << indice);
                *((void **) currentTZL) = TZLsuiv;
                currentTZL = TZLsuiv;
            }
            *((void **) currentTZL) = NULL;

            goto div_adrss;
            break;  
        }
        else if (arena.TZL[k + 1] != NULL)
        {
            goto div_adrss;
            break;

        } 
        else {
            k++; 
        }
    }
div_adrss:
    for (int i = k + 1; i > puiss2(size); i--) {
                void *currentPtr = arena.TZL[i];
                arena.TZL[i] = *((void **)currentPtr);  
                unsigned long bloc_size = (1UL << (i - 1));
                void *buddyPtr = (void *)((unsigned long)currentPtr ^ bloc_size);
                void *nextHeadPtr = arena.TZL[i - 1];
                *((void **)buddyPtr) = nextHeadPtr;
                *((void **)currentPtr) = buddyPtr;
                arena.TZL[i - 1] = currentPtr;
            }
final:
    void *ptr = arena.TZL[puiss2(size)];
    arena.TZL[puiss2(size)] = *((void **)ptr);
    return mark_memarea_and_get_user_ptr(ptr, size, MEDIUM_KIND);
}

void efree_medium(Alloc a) {
    void *ptr = a.ptr; 
    unsigned long size = a.size; 
    unsigned int k = puiss2(size); 

    while (1) {
        void *address_buddy = (void *)((uintptr_t)ptr ^ (1 << k)); 

        void *address = arena.TZL[k];
        void *prev = NULL;

        int buddy_found = 0;
        while (address != NULL) {
            if (address == address_buddy) {
                if (prev == NULL) {
                    arena.TZL[k] = *((void **)address);
                } else {
                    *((void **)prev) = *((void **)address);
                }

                if (ptr > address_buddy) {
                    ptr = address_buddy; 
                }

                size *= 2;
                k++; 
                buddy_found = 1;  
                break;
            }
            prev = address;
            address = *((void **)address);
        }
        if (!buddy_found) {
            break;
        }
    }

    *((void **)ptr) = arena.TZL[k];  
    arena.TZL[k] = ptr;

    return;
}
