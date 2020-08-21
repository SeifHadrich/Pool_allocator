
#ifndef TEST_POOL_ALLOC_H
#define TEST_POOL_ALLOC_H

#include <stdio.h> 
#include <stdint.h> 
#include <stdbool.h>
#include <string.h> 

void test_alloc_data(void * data_ptr[],size_t data_size, size_t nb_alloc, char * struct_name);
void test_display_data(void * data_ptr[],size_t data_size, size_t nb_alloc, char * struct_name);
void test_free_data(void * data_ptr[],size_t* id_data_free,size_t nb_free, char * struct_name);
void pool_test_advanced();

void pool_test_simple_1();//allocate one chunk
void pool_test_simple_2();//allocate 2 chunks


#endif