//这里实现一个附带LRU算法的读缓存
//我们使用一种比较简单但是不见得高效的方案：https://blog.minhow.com/2017/01/06/algorithm/lru-algorithm/
#include<stdlib.h>


//我们这里使用一个链表
//链表的一个元素

typedef struct _read_cache_meta{
    //指向前后两个索引的时针
    read_cache_meta_t *next;
    read_cache_meta_t *front;
    
}read_cache_meta_t;


