
#include <stdio.h>
#include "pool_alloc.h"
#include <math.h>



void pool_test_simple_1(){

    printf("\n *** \t pool_test_simple \t ***\n");

    size_t str_arra_size[]={4};


    // for(int i=0;i<19;i++)str_arra_size[i]=4*(i+1);
    

    if(pool_init(str_arra_size,1))printf("pool int passed\n");
    else {
        printf("pool int faild\n");
        return;
    }

    printf("after init  :\n");
    // pool_print_linked_list(1);
    uint32_t * ptr[10];

    for(int i=0;i<8;i++){
        ptr[i]=(uint32_t *)pool_malloc(4);
        // printf("adr = %p\n",ptr[i]);
    }
    
    uint32_t count =0x55;
    for(int i=0;i<8;i++)ptr[i][0]=count++;



    
    for(int i=0;i<2;i++){
        pool_free(ptr[i]);
        printf("free adr = %p\n",ptr[i]);
    }

    

    for(int i=0;i<3;i++){
        ptr[i]=(uint32_t *)pool_malloc(4);
        if(!ptr[i])break;
        
    }


    // pool_free(ptr[7]);
    // // printf("after free 1  :\n");
    // pool_print_linked_list(1);

    pool_heap_dump();

 }


void pool_test_simple_2(){
    size_t tab[3]={8*4,4,9*4};
    if(pool_init(tab,3))printf("pool int passed\n");
    else {
        printf("pool int faild\n");
        return;
    }

    pool_heap_dump();


    // pool_malloc(8*sizeof(uint32_t));

    // for(int i=0;i<2;i++)pool_malloc(8*sizeof(uint32_t));
    pool_print_linked_list(1);
    // pool_heap_dump();
    // pool_malloc(8*sizeof(uint32_t));
    // pool_print_linked_list(1);
    // pool_malloc(8*sizeof(uint32_t));
    
}


 void pool_test_simple_3(){

    printf("\n *** \t pool_test_simple \t ***\n");

    size_t str_arra_size[]={sizeof(uint32_t),3*sizeof(uint32_t),5*sizeof(uint32_t)};

    if(pool_init(str_arra_size,3))printf("pool int passed\n");
    else {
        printf("pool int faild\n");
        return;
    }

    pool_malloc(3*sizeof(uint32_t));

    // //alloc
    // uint32_t* data[20];
    // for(int i=0 ;i<12;i++ ){
    //     data[i]= (uint32_t*)pool_malloc(sizeof(uint32_t));
    //     data[i][0]=5+i;
    // }
    // for(int i=0 ;i<12;i++ )printf("data[%d][0]=%u\n",i,data[i][0]);

    pool_print_linked_list(1);
    pool_print_linked_list(2);
    pool_print_linked_list(3);

    //heap dump
    // pool_heap_dump();

 }


typedef struct {
    uint32_t val[8];
}data_8_str;
void* data_8_ptr[3000];
size_t data_8_to_free_id[3000];
size_t data_8_ptr_offset=0;
void unit_test_data_8();


typedef struct {
    uint32_t val;
}data_1_str;
void* data_1_ptr[100];
size_t data_1_to_free_id[100];
size_t data_1_ptr_offset=0;
void unit_test_data_1();


typedef struct {
    uint32_t val[18];
}data_18_str;
void* data_18_ptr[100];
size_t data_18_to_free_id[100];
size_t data_18_ptr_offset=0;
void unit_test_data_18();


typedef struct {
    uint32_t val[5];
}data_5_str;
void* data_5_ptr[100];
size_t data_5_to_free_id[100];
size_t data_5_ptr_offset=0;
void unit_test_data_5();


typedef struct{ 
    uint32_t val[10];
}data_10_str;
void* data_10_ptr[100];
size_t data_10_to_free_id[100];
size_t data_10_ptr_offset=0;
void unit_test_data_10();


typedef struct {
    uint32_t val[90];
}data_90_str;
void* data_90_ptr[100];
size_t data_90_to_free_id[100];
size_t data_90_ptr_offset=0;
void unit_test_data_90();


#define NB_STUCT 6
size_t struct_array[NB_STUCT]={ sizeof(data_8_str),
                                sizeof(data_1_str),
                                sizeof(data_18_str),
                                sizeof(data_5_str),
                                sizeof(data_10_str),
                                sizeof(data_90_str)};


void test_alloc_data(void * data_ptr[],size_t data_size, size_t nb_alloc, char * struct_name){

    printf("\n\n *** \t test_alloc_data \t ***\n\n");
    printf("struct name :\t%s\n",struct_name);
    size_t data_size_32=data_size/4;
    if(data_size%4)data_size_32++;
    printf("data_size_32 \t= %zu\n",data_size_32);
    printf("nb_alloc \t= %zu\n",nb_alloc);
    
    
    static uint32_t data_counter;
    size_t alloc_count,i;
    if(!data_ptr){
        printf("invalid test : data_ptr is NULL");
        return;
    }

    for (alloc_count =0;alloc_count <nb_alloc;alloc_count++ ){

        /*
        malloc
        */
        data_ptr[alloc_count] = pool_malloc(data_size);

        //pool_malloc faild !
        if(data_ptr[alloc_count] == NULL){
            printf("No more free data , alloc_count =%zu \n",alloc_count);
        }

        //check the no null adr returned by pool_malloc
        if(data_ptr[alloc_count]){
            //pool_malloc passed !
            // else printf("passed\n");
        }
    }
    /*
    fill up the data
    */
    //use a marker to help bebugging : 

    /*
    the order from most signifiant byte to less signifiant byte :
    2 bytes : DA
    3 bytes : structer size
    3 bytes : global data counter
    */
    uint32_t data_mask = 0xDA000000 | (data_size_32<<12);
    uint32_t * struct_field_ptr;
    for(alloc_count=0;alloc_count<nb_alloc;alloc_count++){
        // printf("write str : alloc_count = %zu\n",alloc_count);
        if(data_ptr[alloc_count]){
            struct_field_ptr = (uint32_t*)data_ptr[alloc_count];
            for(i=0;i<data_size_32;i++){
                struct_field_ptr[i]=data_mask+data_counter++;
                // printf("write data = %4x\n",struct_field_ptr[i]);
            }
        }
        else printf("\t\t can't fill-up : invalid allocation alloc_count =%zu\n",alloc_count);
    }
}

void test_display_data(void * data_ptr[],size_t data_size, size_t nb_alloc, char * struct_name){
    
    printf("\n *** \t display_alloc_data \t ***\n");
    printf("struct name =  %s\n\n",struct_name);

    if(!data_ptr){
        printf("invalid test : data_ptr is NULL");
        return;
    }

    size_t data_size_32=data_size/4;
    if(data_size%4)data_size_32++;

    uint32_t * struct_field_ptr;
    size_t alloc_count,i;
    for(alloc_count=0;alloc_count<nb_alloc;alloc_count++){
        printf("read str : alloc_count = %zu\n",alloc_count);
        if(data_ptr[alloc_count]){
            struct_field_ptr = (uint32_t*)data_ptr[alloc_count];
            for(i=0;i<data_size_32;i++){
                printf("val read from heap adr = %p value = %x \n",
                        &struct_field_ptr[i],struct_field_ptr[i]);
            }
        }
        else printf("\t\tno data\n");
    printf("\n");
    }
}

void test_free_data(void * data_ptr[],size_t* id_data_free,size_t nb_free, char * struct_name){
    
    printf("\n *** \t free_alloc_data \t ***\n");
    printf("struct name =  %s\n\n",struct_name);

    for(size_t i=0;i<nb_free;i++){
        pool_free(data_ptr[id_data_free[i]]);
    }
}

void unit_test_data_8(){
        

    for(int i=0;i<90;i++)pool_malloc(sizeof(data_8_str));

        test_alloc_data(data_8_ptr+data_8_ptr_offset,sizeof(data_8_str),82,(char*)"data_8_str");
        data_8_ptr_offset+=90;

        //free the whole data
        data_8_to_free_id[0]=4;
        data_8_to_free_id[1]=0;
        data_8_to_free_id[2]=1;
        data_8_to_free_id[3]=3;   
        test_free_data(data_8_ptr,data_8_to_free_id,1,(char*)"data_8_str");


        test_alloc_data(data_8_ptr+data_8_ptr_offset,sizeof(data_8_str),7,(char*)"data_8_str");
        data_8_ptr_offset+=7;


        test_display_data(data_8_ptr,sizeof(data_8_str),2,(char*)"data_8_str");
        
        //reallocate one more time
        test_alloc_data(data_8_ptr+data_8_ptr_offset,sizeof(data_8_str),5,(char*)"data_8_str");
        data_8_ptr_offset+=5;
    }
    
void unit_test_data_1(){
        test_alloc_data(data_1_ptr+data_1_ptr_offset,sizeof(data_1_str),40,(char*)"data_1_str");
        data_1_ptr_offset+=40;

        // test_display_data(data_1_ptr,sizeof(data_1_str),2,(char*)"data_1_str");

        data_1_to_free_id[0]=2;
        data_1_to_free_id[1]=0;
        data_1_to_free_id[2]=10;
        data_1_to_free_id[3]=20;
        test_free_data(data_1_ptr,data_1_to_free_id,4,(char*)"data_1_str");

        test_alloc_data(data_1_ptr+data_1_ptr_offset,sizeof(data_1_str),17,(char*)"data_1_str");
        data_1_ptr_offset+=17;

        data_1_to_free_id[0]=17;
        test_free_data(data_1_ptr,data_1_to_free_id,1,(char*)"data_1_str");
    }

void unit_test_data_18(){
        test_alloc_data(data_18_ptr+data_18_ptr_offset,sizeof(data_18_str),3,(char*)"data_18_str");
        data_18_ptr_offset+=4;

        // test_display_data(data_18_ptr,sizeof(data_18_str),2,(char*)"data_18_str");

        data_18_to_free_id[0]=2;
        test_free_data(data_18_ptr,data_18_to_free_id,1,(char*)"data_18_str");

        test_alloc_data(data_18_ptr+data_18_ptr_offset,sizeof(data_18_str),27,(char*)"data_18_str");
        data_18_ptr_offset+=27;
    }
     
void unit_test_data_5(){
        test_alloc_data(data_5_ptr+data_5_ptr_offset,sizeof(data_5_str),4,(char*)"data_5_str");
        data_5_ptr_offset+=4;

        // test_display_data(data_5_ptr,sizeof(data_5_str),2,(char*)"data_5_str");

        data_5_to_free_id[0]=2;
        data_5_to_free_id[0]=0;
        test_free_data(data_5_ptr,data_5_to_free_id,2,(char*)"data_5_str");

        test_alloc_data(data_5_ptr+data_5_ptr_offset,sizeof(data_5_str),7,(char*)"data_5_str");
        data_5_ptr_offset+=7;

        data_5_to_free_id[0]=5;
        data_5_to_free_id[1]=15;
        data_5_to_free_id[2]=40;
        test_free_data(data_5_ptr,data_5_to_free_id,3,(char*)"data_5_str");
    }
  
void unit_test_data_10(){
        test_alloc_data(data_10_ptr+data_10_ptr_offset,sizeof(data_10_str),5,(char*)"data_10_str");
        data_10_ptr_offset+=5;

        data_10_to_free_id[0]=2;
        test_free_data(data_10_ptr,data_10_to_free_id,1,(char*)"data_10_str");

        test_alloc_data(data_10_ptr+data_10_ptr_offset,sizeof(data_10_str),5,(char*)"data_10_str");
        data_10_ptr_offset+=5;

    } 
     
void unit_test_data_90(){
        test_alloc_data(data_90_ptr+data_90_ptr_offset,sizeof(data_90_str),9,(char*)"data_90_str");
        data_90_ptr_offset+=9;

        // test_display_data(data_90_ptr,sizeof(data_90_str),2,(char*)"data_90_str");

        data_90_to_free_id[0]=2;
        test_free_data(data_90_ptr,data_90_to_free_id,1,(char*)"data_90_str");

        test_alloc_data(data_90_ptr+data_90_ptr_offset,sizeof(data_90_str),1,(char*)"data_90_str");
        data_90_ptr_offset+=1;
    }
 

void pool_test_advanced(){

printf("\n *** \t pool_test_advanced \t ***\n");

    if(pool_init(struct_array,NB_STUCT))printf("pool int passed\n");
    else {
        printf("pool int faild\n");
        return;
    }

unit_test_data_8();
unit_test_data_1();
unit_test_data_18();
unit_test_data_5();
unit_test_data_10();
unit_test_data_90();


test_display_data(data_8_ptr,sizeof(data_8_str),data_8_ptr_offset,(char*)"data_8_str");
test_display_data(data_1_ptr,sizeof(data_1_str),data_1_ptr_offset,(char*)"data_1_str");
test_display_data(data_18_ptr,sizeof(data_18_str),data_18_ptr_offset,(char*)"data_18_str");
test_display_data(data_5_ptr,sizeof(data_5_str),data_5_ptr_offset,(char*)"data_5_str");
test_display_data(data_10_ptr,sizeof(data_10_str),data_10_ptr_offset,(char*)"data_10_str");
test_display_data(data_90_ptr,sizeof(data_90_str),data_90_ptr_offset,(char*)"data_90_str");

}




