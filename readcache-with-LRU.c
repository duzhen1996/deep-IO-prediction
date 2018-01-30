//这里实现一个附带LRU算法的读缓存
//我们使用一种比较简单但是不见得高效的方案：https://blog.minhow.com/2017/01/06/algorithm/lru-algorithm/
#include<stdlib.h>
#include<stdio.h>
#include<memory.h>
#include"readcache-with-LRU.h"


//这里需要初始化一个读缓存元数据链表
void init_readcache_meta(all_read_cache_t* input_cache_arr){
    printf("初始化读缓存元数据\n");

    // read_cache_meta_t *head_node;
    //用一个循环来初始化所有缓冲区
    for(int i = 0; i < MAX_READ_CACHE_NUM; i++){
        //初始化读缓存
        input_cache_arr->read_cache_arr[i].size = 0;
        
        //创建一个头结点
        read_cache_meta_t *head_node = (read_cache_meta_t *)malloc(sizeof(read_cache_meta_t));

        // printf("head_node\n");

        //定位一下指针
        input_cache_arr->read_cache_arr[i].head = head_node;


        //将头尾两个指针指回自己
        input_cache_arr->read_cache_arr[i].head->next = input_cache_arr->read_cache_arr[i].head;
        input_cache_arr->read_cache_arr[i].head->front = input_cache_arr->read_cache_arr[i].head;
        input_cache_arr->read_cache_arr[i].head->block_num = -1;

        //初始化换出块数量和未经使用的换出块数量
        input_cache_arr->read_cache_arr[i].page_cache_del = 0;
        input_cache_arr->read_cache_arr[i].page_cache_have_not_access = 0;
    }
    
}


//向缓存中添加一个元素的元数据
void add_cache_meta(all_read_cache_t* input_cache_arr, long cache_block_num){
    //看看是不是元素已经在缓冲区了
    int this_have_accessed = 0;
    int barrel_num = cache_block_num % MAX_READ_CACHE_NUM;
    
    //首先找一下看看有没有这个元素
    read_cache_meta_t* search_block = search_read_cache(&(input_cache_arr->read_cache_arr[barrel_num]), cache_block_num);

    if(search_block != NULL){
        //如果找到了这个block，那就直接删掉
        del_read_cache(&(input_cache_arr->read_cache_arr[barrel_num]), search_block);
        this_have_accessed = 1;
    }

    //创建一个节点
    read_cache_meta_t* insert_block = (read_cache_meta_t *)malloc(sizeof(read_cache_meta_t));

    //这里初始化新建的节点
    insert_block->block_num = cache_block_num;
    insert_block->front = NULL;
    insert_block->next = NULL;

    insert_block->have_accessed = this_have_accessed;


    //查看大小，如果大小已经满了，那就从末尾删除元素
    if(input_cache_arr->read_cache_arr[barrel_num].size >= MAX_READ_CACHE_SIZE){
        del_read_cache_total(&(input_cache_arr->read_cache_arr[barrel_num]), input_cache_arr->read_cache_arr[barrel_num].head->front);
    }

    //到这里至少保证已经没有这个元素了，将元素添加到读缓存中
    add_item_head(&(input_cache_arr->read_cache_arr[barrel_num]), insert_block);
}


//向链表中添加一个元素在头部在头部
void add_item_head(read_cache_t* input_cache, read_cache_meta_t* insert_meta){
    //插入在空白头结点和第一个元素之间
    read_cache_meta_t* next_node = input_cache->head->next;

    //然后将新的节点插入在头部
    //和头部节点连接一下
    input_cache->head->next = insert_meta;
    insert_meta->front = input_cache->head;

    //和第一个节点连接一下
    next_node->front = insert_meta;
    insert_meta->next = next_node;

    input_cache->size++;
}

//在读缓存链表中搜索一个元素，返回这个元素的指针
read_cache_meta_t* search_read_cache(read_cache_t* input_cache, long cache_block_num){
    
    //用一个指针来不断搜索
    
    read_cache_meta_t* search_pointer = input_cache->head->next;

    //只要没有在头部就不断搜索
    while(search_pointer != input_cache->head){
        // printf("在读缓存中搜索\n");
        //看看是不是要求的元素
        if(search_pointer->block_num==cache_block_num){
            //传出这个指针的值
            return search_pointer;
        }
        //指针向下走一个
        search_pointer = search_pointer->next;
    }

    //如果找不到就返回空指针
    return NULL;
}

//从读缓存链表中删除一个元素，将要删除的元素的指针传入
void del_read_cache(read_cache_t* input_cache, read_cache_meta_t* del_meta){
    
    //这里重新连接前面和后面的节点
    read_cache_meta_t* del_front = del_meta->front;
    read_cache_meta_t* del_next = del_meta->next;

    //前后重新连接一下
    del_front->next = del_next;
    del_next->front = del_front;

    input_cache->size--;

    //释放当前节点
    free(del_meta);
}

//这个函数是在缓冲区满了之后为了腾空间释放缓冲区用的，所以这里我们需要记录一下预测的错误率
void del_read_cache_total(read_cache_t* input_cache, read_cache_meta_t* del_meta){
    input_cache->page_cache_del++;
    
    if(del_meta->have_accessed == 0){
        input_cache->page_cache_have_not_access++;
    }

    //这里重新连接前面和后面的节点
    read_cache_meta_t* del_front = del_meta->front;
    read_cache_meta_t* del_next = del_meta->next;

    //前后重新连接一下
    del_front->next = del_next;
    del_next->front = del_front;

    input_cache->size--;

    //释放当前节点
    free(del_meta);
}

//打印所有的元数据
void print_read_cache(read_cache_t *read_cache){
    //打印所有数据
    //用一个指针来不断搜索
    read_cache_meta_t* search_pointer = read_cache->head->next;

    //只要没有在头部就不断搜索
    while(search_pointer != read_cache->head){
        //打印
        printf("%ld, ", search_pointer->block_num);
        //指针向下走一个
        search_pointer = search_pointer->next;
    }

    printf("\n");
}

//释放读缓存中的所有元素
void free_read_cache(read_cache_t* input_cache){
    
}

