
#include <stdio.h>
#include <math.h>

#include "pool_alloc.h"
#include "test_pool_alloc.h"

int main(int argc, char const *argv[]){

    printf("\n\n\n start program here \n");

    //run one basic example
    pool_test_simple_1();
    // pool_test_simple_2();

    // //run more adavnced test    
    // pool_test_advanced();    

    //heap dump display the whole heap
    // pool_heap_dump();

    
return 0;
}



