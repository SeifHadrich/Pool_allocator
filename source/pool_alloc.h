/**
 * @file pool_alloc.h

 _________________________________________________________________________________________________________________________________

 _________________________________________________________________________________________________________________________________

 Developped by Seif

      Contact  : seif.hadrich@gmail.com
      Date     : 08/16/2019

 Reviewed by -

      Contact  : -
      Date     : -

 _________________________________________________________________________________________________________________________________

 _________________________________________________________________________________________________________________________________

 */
#ifndef POOL_ALLOC_H
#define POOL_ALLOC_H

#include <stdio.h> 
#include <stdint.h> 
#include <stdbool.h>

//Enable the debugging functions
#define DEBUG_POOL_ALLOC

/*
________________________________________________________________________________________________________________________________
*/
// Initialize the pool allocator with a set of block sizes appropriate for this application.
// Returns true on success, false on failure.
bool pool_init(const size_t* block_sizes, size_t block_size_count);
// Allocate n bytes.
// Returns pointer to allocate memory on success, NULL on failure.
void* pool_malloc(size_t n);
// Release allocation pointed to by ptr.
void pool_free(void* ptr);

/*
________________________________________________________________________________________________________________________________
*/
//help debugging functions
#ifdef DEBUG_POOL_ALLOC

    //plot the whole heap
    void pool_heap_dump();
    //print the linked list of free chunks
    void pool_print_linked_list(uint8_t block_id);

//end : DEBUG_POOL_ALLOC
#endif

//end : POOL_ALLOC_H
#endif