/**
 * @file pool_alloc.cpp

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
#include <stdio.h> 
#include <stdint.h> 
#include <stdbool.h>
#include "pool_alloc.h"

/*
________________________________________________________________________________________________________________________________
*/
//Params :
#define Param_Heap_Size_8b          65536   // 64 KB RAM < MAX_HEAP_SIZE_8b6
#define Param_Coef                  8       // < MAX_COEF
#define Param_Max_Nb_Chunk          6       // < MAX_NB_CHUNK
#define Param_Max_Chunk_Size_8b     1024    // < MAX_CHUNK_SIZE_8b
#define Param_Min_Chunk_Size_8b     4       // > MIN_CHUNK_SIZE_8b

/*
________________________________________________________________________________________________________________________________
*/
/*
absolute max values please don't change these values
*/
#define MAX_HEAP_SIZE_8b            0x00FFFFFF 
#define MAX_COEF                    0xFFFF
#define MAX_NB_CHUNK                0b00111111
#define MAX_CHUNK_SIZE_8b           0x000FFFFF
#define MIN_CHUNK_SIZE_8b           1

/*
________________________________________________________________________________________________________________________________
*/
/*
program defines
*/
#define DEADBEEF                    0xDEADBEEF//mark allignement bytes
#define NOT_USED_YET                0xEEAAEEBB//mark the not used yet data
#define HEADER_SIZE_32              1 // 4 bytes 1 byte for chunk_id and 3bytes for the next chunk position
#define NULL_POS                    0 //no next chunk
#define BLOCK_ID                    0 //block id = 0 and chunk id is > 0

/*
private variables could only be used on pool_alloc.cpp
*/
//params
static uint32_t     g_param_heap_size_32b       = 0;
static uint32_t     g_param_coef                = 0;
static uint32_t     g_param_max_nb_chunk        = 0;
static uint32_t     g_param_max_chunk_size_32b  = 0;
static uint32_t     g_param_min_chunk_size_32b  = 0;

//private global variables
#define HEAP_SIZE_8b (Param_Heap_Size_8b<MAX_HEAP_SIZE_8b ? Param_Heap_Size_8b :MAX_HEAP_SIZE_8b)
#define HEAP_SIZE_32b (HEAP_SIZE_8b/4)
static uint8_t      g_pool_heap[HEAP_SIZE_8b];//the heap if possible use malloc to allocate the heap instead of this
static uint32_t*    g_pool_heap_32b = (uint32_t*)g_pool_heap;

static uint32_t     g_nb_chunk                  = 0;
static uint32_t     g_total_block_size_32b          = 0;
static bool         g_pool_is_init                  = false;

static uint32_t*    g_chunk_size_32b           = NULL;
static uint32_t*    g_free_chunk_linked_list   = NULL; 

static uint32_t     g_heap_next_pos             = 0;
static uint32_t     g_heap_first_pos            = 0;
static uint32_t     g_heap_last_pos             = 0;


//private functions
bool make_new_chunks(uint32_t chunk_id);
bool check_params();

/*
________________________________________________________________________________________________________________________________
*/
/*
* pool_init
* Initialize pool allocator
* IN  -> array of sizes and number of sizes
* OUT <- true on success
*/
bool pool_init(const size_t* user_chunk_sizes, size_t nb_chunk_input) {
    
    if(g_pool_is_init){
        #ifdef DEBUG_POOL_ALLOC
            printf("\n\n[pool_debug] > fn: pool_init() pool is already initialized !\n");
        #endif
        return false;
    }

    size_t i;
    //set pool initilized flag to false
    g_pool_is_init = false;
    //mark all data as unsed data (help debuging a RAM dump)
    for(i=0;i<HEAP_SIZE_32b;i++)g_pool_heap_32b[i]=NOT_USED_YET;

    //read and check params
    if(!check_params()){
        #ifdef DEBUG_POOL_ALLOC
            printf("\n\n[pool_debug] > fn: pool_init() invalid param\n");
        #endif
        return false;
    }
    
    if(!user_chunk_sizes || !nb_chunk_input){
       #ifdef DEBUG_POOL_ALLOC
            printf("\n\n[pool_debug] > fn: pool_init() no sizes available\n");
        #endif
        return false;
   }

    //the g_chunk_size_32b array is actually a top piece of the heap
    g_chunk_size_32b=g_pool_heap_32b;

    //check the number of chunks set 
    if(nb_chunk_input > Param_Max_Nb_Chunk) {
        #ifdef DEBUG_POOL_ALLOC
            printf("\n\n[pool_debug] > fn: pool_init() Error nb_chunk_input > Param_Max_Nb_Chunk \n");
        #endif
        return false;
    }
    
    size_t  user_chunk_max_size_32b=0;
    size_t  user_chunk_min_size_32b=-1;//the max value for an unsigned
    uint8_t chunk_id;
    
    uint32_t user_size_32b=0;

    //set the nb_chunk
    g_nb_chunk =nb_chunk_input;

    /*
    process the chunk sizes input array
    */
    for(chunk_id = 1;chunk_id<=g_nb_chunk;chunk_id++){
        
        //convert 8 bits size to 32bits size
        user_size_32b=user_chunk_sizes[chunk_id-1]/4;

        //make sure that this size is 4 bytes aligned
        if(user_chunk_sizes[chunk_id-1]%4){
            #ifdef DEBUG_POOL_ALLOC
                printf("\n\n[pool_debug] > fn: pool_init() Error size = %zu is not 4 bytes alligned\n",
                    user_chunk_sizes[chunk_id-1]);
            #endif

            return false;
        }

        //make sure that this size is not already exist 
        for(i=0;i<chunk_id-1;i++){
            if(g_chunk_size_32b[i]==user_size_32b){

                #ifdef DEBUG_POOL_ALLOC
                    printf("\n\n[pool_debug] > fn: pool_init() Error size = %u has been enter twice\n",
                        user_size_32b);
                #endif

                return false;
            }
        }

        //saved the new chunk size
        g_chunk_size_32b[chunk_id-1]=user_size_32b;
        
        //keep tracking of the max and min sizes
        if(user_size_32b>user_chunk_max_size_32b)user_chunk_max_size_32b=user_size_32b;
        if(user_size_32b<user_chunk_min_size_32b)user_chunk_min_size_32b=user_size_32b;   

    }
    
    //check the min and max size (if min and max are with in the normal range then all size are also within the range)
    if(user_chunk_max_size_32b > g_param_max_chunk_size_32b || user_chunk_min_size_32b < g_param_min_chunk_size_32b) {

        #ifdef DEBUG_POOL_ALLOC
            if(user_chunk_max_size_32b > g_param_max_chunk_size_32b)
                printf("\n\n[pool_debug] > fn: pool_init() Error user_chunk_max_size_32b =%zu > g_param_max_chunk_size_32b =%u\n",
                        user_chunk_max_size_32b,g_param_max_chunk_size_32b);

            if(user_chunk_min_size_32b < g_param_min_chunk_size_32b)
                printf("\n\n[pool_debug] > fn: pool_init() Error user_chunk_min_size_32b =%zu < g_param_min_chunk_size_32b =%zu\n",
                        user_chunk_min_size_32b,(size_t)g_param_min_chunk_size_32b);
        #endif

        return false;
    }
     
    g_total_block_size_32b = (user_chunk_max_size_32b+HEADER_SIZE_32)*g_param_coef ;
  
    /*
    set g_free_chunk_linked_list :

    the g_free_chunk_linked_list array stored in the heap just after g_chunk_size_32b array
    */
    g_free_chunk_linked_list=&g_chunk_size_32b[g_nb_chunk];

    //set the linked list root as empty
    for(i=0;i<g_nb_chunk;i++)
        g_free_chunk_linked_list[i]=((i+1)<<24)|NULL_POS;

    /*
    set heap_first_pos and heap_last_pos:
    the pool heap starts just after the g_free_chunk_linked_list
    */
    g_heap_first_pos    = g_nb_chunk *2 ; //First heap position come after the chunk sizes array + linked list array
    g_heap_last_pos     = g_param_heap_size_32b;
    
    //set the position of the next available position
    g_heap_next_pos     = g_heap_first_pos;
    
    //making all chunks
    for(i=0;i<g_nb_chunk;i++){
        //if making new chunk failed
        if(!make_new_chunks(i+1)){

            #ifdef DEBUG_POOL_ALLOC
                printf("\n\n[pool_debug] > fn: pool_init() heap full : cannot make a new chun chunk_id =%zu\n",i+1);
            #endif

            return false;
        }
    }

    g_pool_is_init      = true; //the heap in now initilized
    return true;
}


/*
* pool_malloc
* allocate a dynamic memory 
* IN  -> the size (it should be one of the initilized sizes setted with pool_init() )
* OUT <- -
*/
void* pool_malloc(size_t n) {

    size_t i;
    size_t user_size_32b = n/4;

    //the ret value
    void *ret_chunk_adr =NULL;

    if(!g_pool_is_init){
            #ifdef DEBUG_POOL_ALLOC
                printf("\n\n[pool_debug] > fn: pool_malloc() pool is not initialized !\n");
            #endif
            return NULL;
        }

    uint8_t chunk_id=0;
    //find the size of the chunk
    for(i=0;i<g_nb_chunk;i++){
        // printf("g_input_size[%d]= %d , n= %lu \n",i,g_input_size[i],n);
        if(g_chunk_size_32b[i]==user_size_32b){
            chunk_id=i+1;
            break;
        }
    }

    //can't find the chunkd id
    if(!chunk_id){
        #ifdef DEBUG_POOL_ALLOC
            printf("\n\n[pool_debug] > fn: pool_malloc() unknown size !\n");
        #endif
        return NULL;
    }   

    /*
    if no chunk is available in the linked list, we try to 
    allocate a new block from the heap and then transform it to many chunks.
    */
    if(g_free_chunk_linked_list[chunk_id-1]==NULL_POS){
        //can't allocate a free block
        if(!make_new_chunks(chunk_id)){

            #ifdef DEBUG_POOL_ALLOC
                printf("\n\n[pool_debug] > fn: pool_malloc() no more chunk is available!\n");
            #endif
            
            return NULL;
        }
    }
        
    /*
    at least one free chunk is available
    */

    uint32_t chunk_position= g_free_chunk_linked_list[chunk_id-1] & 0x00FFFFFF ;

    //safety check
        if(chunk_position>g_heap_last_pos || chunk_position<g_heap_first_pos ){

            #ifdef DEBUG_POOL_ALLOC
                printf("\n\n[pool_debug] > fn: pool_malloc() chunk_position invalid : corrupted memory !\n");
            #endif

            return NULL;
        }
    //convert a chunk position to the absolute adr
    ret_chunk_adr = &g_pool_heap_32b[ chunk_position + HEADER_SIZE_32];
    
    /*
    detach the chunk from the linked list
    */
    g_free_chunk_linked_list[chunk_id-1] = g_pool_heap_32b[chunk_position] & 0x00FFFFFF;
          
    return ret_chunk_adr;
}


/*
* pool_free
* free a chunk
* IN  -> chunk adr
* OUT <- -
*/
void pool_free(void* ptr) {
    
    if(!g_pool_is_init){

        #ifdef DEBUG_POOL_ALLOC
            printf("\n\n[pool_debug] > fn: pool_free() pool is not initialized !\n");
        #endif

        return;
    }
    
    //convert ptr adr to a relative pos
    uint32_t block_header_pos = ( ((size_t)ptr - (size_t)g_pool_heap_32b) /4 ) - HEADER_SIZE_32;

    //safety check
    if( (((size_t)ptr - (size_t)g_pool_heap_32b) > 0x00FFFFFF)  || //the relative pos didn't fit in 3 bytes
         (block_header_pos < g_heap_first_pos) ||
         (block_header_pos > g_heap_last_pos)){  //the realative position it's out of the heap
        #ifdef DEBUG_POOL_ALLOC
                printf("\n\n[pool_debug] > fn: pool_free() Error no valid free pointer \n");
        #endif
        return;
        }

    //get chunk id
    uint8_t chunk_id =  (g_pool_heap_32b[block_header_pos] & 0xFF000000)>> 24 ;

    //safety check
    if(chunk_id<1 || chunk_id>g_nb_chunk){
        #ifdef DEBUG_POOL_ALLOC
                printf("\n\n[pool_debug] > fn: pool_free() Error no valid chunk_id \n");
        #endif
        return;
    }
    
    /*
    insert a new chunk to the head of the linked list (FILO)
    */
    //the linked list is not empty
    if((g_free_chunk_linked_list[chunk_id-1]&0x00FFFFFF) != NULL_POS){
        //new chunk -> next = root
        g_pool_heap_32b[block_header_pos]=g_free_chunk_linked_list[chunk_id-1];
    }
    //empty linked list
    else {
        //new chunk -> next = NULL_POS
        g_pool_heap_32b[block_header_pos]= (chunk_id<<24) | NULL_POS;
    }

    //root = the new free chunk
    g_free_chunk_linked_list[chunk_id-1] = (chunk_id<<24) |block_header_pos ;
    
    return ;
}

/*
________________________________________________________________________________________________________________________________
*/
/*
* make_new_chunks
* create a new chunks and add them to the corresponding linked list
* IN  -> chunk id
* OUT <- true on success
*/
bool make_new_chunks(uint32_t chunk_id){

    //check if there is more space in the heap to allocate a new block
    if((g_heap_next_pos + g_total_block_size_32b ) >  g_heap_last_pos){

        #ifdef DEBUG_POOL_ALLOC
            printf("\n\n[pool_debug] > fn: make_new_chunks() pool no more block are availble !\n");
        #endif

        return false;
    }
    
    //compute the total size of the chunk
    uint32_t chunk_total_size = g_chunk_size_32b[chunk_id-1] + HEADER_SIZE_32 ;

    //chunk_header_position
    uint32_t chunk_header_position  = g_heap_next_pos;
    uint32_t chunk_header_next_pos = chunk_header_position ;

    //update next available position in the heap
    g_heap_next_pos = g_heap_next_pos + g_total_block_size_32b;

    /*
    split the block into chunks
    and add those chunks to the linked list
    */
    uint32_t nb_chunks = g_total_block_size_32b/chunk_total_size;
    uint32_t i;
    g_free_chunk_linked_list[chunk_id-1]= (chunk_id<<24) |chunk_header_position;

    for (i=0;i<nb_chunks;i++){
        //check if there will be a next chunk
        if((chunk_header_position + chunk_total_size) < g_heap_next_pos ){
            chunk_header_next_pos = chunk_header_position + chunk_total_size;
            g_pool_heap_32b[chunk_header_next_pos]= (chunk_id<<24)  | NULL_POS;
        }
        //set the current chunk as the last one.
        else chunk_header_next_pos = NULL_POS;

        //attach the chunk to the tail of the linked list
        g_pool_heap_32b[chunk_header_position]= (chunk_id<<24) | chunk_header_next_pos;
        
        //move to the next chunk
        chunk_header_position += chunk_total_size;
    }

    //the rest data is useless so we can mark it as DEADBEEF
    for(i=chunk_header_position+1;i<g_heap_next_pos;i++)
        g_pool_heap_32b[i]=DEADBEEF;
    
    return true;
}


/*
* check_params
* check pool parameters
* IN  -> 
* OUT <- true on success
*/
bool check_params(){
    if(Param_Heap_Size_8b <= MAX_HEAP_SIZE_8b){
        g_param_heap_size_32b=((uint32_t)Param_Heap_Size_8b)/4;
        if(Param_Coef <= MAX_COEF){
            g_param_coef=(uint32_t)Param_Coef;
            if(Param_Max_Nb_Chunk <= MAX_NB_CHUNK){
                g_param_max_nb_chunk=(uint32_t)Param_Max_Nb_Chunk;
                if(Param_Max_Chunk_Size_8b <= MAX_CHUNK_SIZE_8b){
                        g_param_max_chunk_size_32b=(uint32_t)Param_Max_Chunk_Size_8b/4;
                        if(Param_Min_Chunk_Size_8b >= MIN_CHUNK_SIZE_8b &&
                            Param_Min_Chunk_Size_8b <= Param_Min_Chunk_Size_8b){
                                g_param_min_chunk_size_32b=(uint32_t)Param_Min_Chunk_Size_8b/4;
                                return true;
                            }
                        }
                }
            }
        }
    return false;
    }

/*
________________________________________________________________________________________________________________________________
*/
//theses functions only compiled when DEBUG_POOL_ALLOC is defined
#ifdef DEBUG_POOL_ALLOC 

    /*
    * pool_heap_dump
    * plot the whole heap
    * IN  -> -
    * OUT <- -
    */
    void pool_heap_dump(){
        uint32_t i;
        if(!g_pool_is_init){
            printf("\n\n[pool_debug] > fn: pool_heap_dump() pool is not initialized !\n");
            return;
        }

        printf("\n***\t _pool_heap_print_ \t***\n");
        printf("heap size_32b \t= %u \n",HEAP_SIZE_32b);
        printf("First adr \t= %p\n",&g_pool_heap_32b[0]);
        printf("heap first adr \t= %p\n",&g_pool_heap_32b[g_heap_first_pos]);
        printf("heap last adr \t= %p\n",&g_pool_heap_32b[g_heap_last_pos]);
        printf("Last adr \t= %p\n",&g_pool_heap_32b[HEAP_SIZE_32b-1]);
        printf("g_nb_chunk              = %u\n" ,g_nb_chunk);
        printf("g_total_block_size_32b  = %u\n" ,g_total_block_size_32b);
        printf("g_heap_first_pos        = %u\n" ,g_heap_first_pos);
        printf("g_heap_last_pos         = %u\n" ,g_heap_last_pos);
        printf("g_heap_next_pos         = %u\n" ,g_heap_next_pos);
        printf("***\t***\t***\n\n");

        //print the size chunks array
        printf("print the linked list roots array\n");
        printf(" ------------\t------------\t------------\t------------\n");
        for(i=0;i<g_nb_chunk;i++)
            printf("size[%3x] adr = %p\t val = %x\n",i,&g_chunk_size_32b[i],g_chunk_size_32b[i]);
        printf(" ------------\t------------\t------------\t------------\n\n");
        
        //print the linked list roots array
        printf("print the linked list roots array\n");
        printf(" ------------\t------------\t------------\t------------\n");
        for(i=0;i<g_nb_chunk;i++)
            printf("ll[%3x] adr = %p\t val = %x\n",i,&g_free_chunk_linked_list[i],g_free_chunk_linked_list[i]);
        printf(" ------------\t------------\t------------\t------------\n\n");
        
        //dump the heap
        printf("\n***\t heap dump \t***\n");
        printf("heap first pos \t= %u\n",g_heap_first_pos);
        printf("heap last pos \t= %u\n",g_heap_last_pos);
        printf("g_total_block_size_32b = %u  \n",g_total_block_size_32b);
        
        for (i =g_heap_first_pos; i<= g_heap_last_pos ;i++){
            if(!((i-g_heap_first_pos)%g_total_block_size_32b))
                    printf(" ------------\t------------\t------------\t------------\n");
            printf("[%3x] adr = %p\t val = %x\n",i,&g_pool_heap_32b[i],g_pool_heap_32b[i]);
            if(!((i-g_heap_first_pos)%g_total_block_size_32b))
                    printf(" ------------\t------------\t------------\t------------\n");
        }

        printf("\n***\t print linked list \t***\n");
        for(uint8_t chunk_id =1;chunk_id<=g_nb_chunk;chunk_id++){
            pool_print_linked_list(chunk_id);
        }
    }


    /*
    * pool_print_linked_list
    * print the linked list of free chunks
    * IN  -> chunk_id
    * OUT <- -
    */
    void pool_print_linked_list(uint8_t chunk_id){

        if(!g_pool_is_init){
            printf("\n\n[pool_debug] > fn: pool_print_linked_list() pool is not initialized !\n");
            return;
        }

        if(chunk_id < 1){
            printf("error chunk_id =%u is not valid\n",chunk_id);
            return;
        }

        printf("****\tchunk_id = %d\t********\n",chunk_id);
        
        if(chunk_id>g_nb_chunk){
            printf("error chunk_id =%u > g_nb_chunk=%u\n",chunk_id,g_nb_chunk);
            return;
        }

        uint32_t chunk_header = 0;
        uint32_t chunk_adr =  g_free_chunk_linked_list[chunk_id-1] &0x00FFFFFF;
        
        printf("************\n");
        while(chunk_adr != NULL_POS){
            if(chunk_adr < g_heap_first_pos || chunk_adr > g_heap_last_pos){
                printf("error chunk_adr =%u out of the heap\n",chunk_adr);
                return;
            }
            printf("%4x -> ",chunk_adr);
            chunk_header=g_pool_heap_32b[chunk_adr];
            chunk_adr=chunk_header&0x00FFFFFF;
            chunk_id =chunk_header>>24;
        }
        printf("END\n");
        printf("************\n");
    }
//end : DEBUG_POOL_ALLOC
#endif