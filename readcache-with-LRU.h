#ifndef READCACHE_WITH_LRU_H
#define READCACHE_WITH_LRU_H

//我们这里使用一个链表
//链表的一个元素
#define MAX_READ_CACHE 100


typedef struct _read_cache_meta{
    //指向前后两个索引的时针
    struct _read_cache_meta *next;
    struct _read_cache_meta *front;
    
    //在缓存中的块号
    long block_num;
}read_cache_meta_t;

//这里需要一个头结点
typedef struct _read_cache{
    //有一个空节点，用来当做头结点
    read_cache_meta_t* head;
    //当前节点的大小
    int size;
}read_cache_t;

void init_readcache_meta(read_cache_t* input_cache);

void add_cache_meta(read_cache_t* input_cache, int cache_block_num);

void add_item_head(read_cache_t* input_cache, read_cache_meta_t* insert_meta);

read_cache_meta_t* search_read_cache(read_cache_t* input_cache, int cache_block_num);

void del_read_cache(read_cache_t* input_cache, read_cache_meta_t* del_meta);

void print_read_cache(read_cache_t *read_cache);

void free_read_cache(read_cache_t* input_cache);

#endif