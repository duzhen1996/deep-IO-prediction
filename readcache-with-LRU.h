#ifndef READCACHE_WITH_LRU_H
#define READCACHE_WITH_LRU_H

//我们这里使用一个链表
//链表的一个元素
#define MAX_READ_CACHE 50000

#define MAX_READ_CACHE_SIZE 500
#define MAX_READ_CACHE_NUM 100

typedef struct _read_cache_meta{
    //指向前后两个索引的时针
    struct _read_cache_meta *next;
    struct _read_cache_meta *front;

    //仅仅为了方便测试而使用
    int have_accessed;

    //在缓存中的块号
    long block_num;
}read_cache_meta_t;

//这里需要一个头结点
typedef struct _read_cache{
    //有一个空节点，用来当做头结点
    read_cache_meta_t* head;
    //当前节点的大小
    int size;

    //设定两个计数变量，一个是判定有多少的元素从缓存中剔除，还有一个是判定有多少元素是从进入到剔除从来没有用过的
    int page_cache_del;
    int page_cache_have_not_access;

}read_cache_t;

//我们将每一个缓冲区当做一个桶，然后将所有的桶整合起来
typedef struct _all_read_cache{
    //这里使用一个数组来保存所有的读缓存
    read_cache_t read_cache_arr[MAX_READ_CACHE_NUM];
}all_read_cache_t;

void init_readcache_meta(all_read_cache_t* input_cache);

void add_cache_meta(all_read_cache_t* input_cache, long cache_block_num);

void add_item_head(read_cache_t* input_cache, read_cache_meta_t* insert_meta);

read_cache_meta_t* search_read_cache(read_cache_t* input_cache, long cache_block_num);

void del_read_cache(read_cache_t* input_cache, read_cache_meta_t* del_meta);

void del_read_cache_total(read_cache_t* input_cache, read_cache_meta_t* del_meta);

void print_read_cache(read_cache_t *read_cache);

void free_read_cache(read_cache_t* input_cache);

#endif