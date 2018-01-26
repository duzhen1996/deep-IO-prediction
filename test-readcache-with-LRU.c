//这个文件是用来测试读缓存元数据的，我们主要就是测试初始化函数和插入函数
#include "readcache-with-LRU.h"


int main(){
    //创建一个缓冲区
    read_cache_t read_cache;

    //初始化缓冲区
    init_readcache_meta(&read_cache);

    //这里插入一个元素
    add_cache_meta(&read_cache, 1);

    //打印看看有没有插入的元素
    //打印一下看看插入的结果
    print_read_cache(&read_cache);
    

    add_cache_meta(&read_cache, 2);
    add_cache_meta(&read_cache, 3);

    add_cache_meta(&read_cache, 1);

    //打印一下看看插入的结果
    print_read_cache(&read_cache);
    
}
