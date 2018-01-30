//这个函数是一个调度器，这个调度器接受一系列的预测器，并且接受一段的历史输入序列，从而可以得到预测的结果

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pageread-predictor.h"
#include "readcache-with-LRU.h"
#include "Noah-predictor.h"

//每一行的最大字节数，同时也是行缓冲区的大小
#define MAX_LINE_SIZE 1024

//预测深度的最大值
#define MAX_PREDICT_DEEP 20


//这里创建一个预测的管理器，我们使用这个管理器来进行预测
void predictor_manager(){
    //从文件中一个个读入
    FILE *fp_read = NULL;
    char line[MAX_LINE_SIZE];
    long access_block_count;
    long* predict_seq;
    long* predict_seq2;
    int size;
    int size2;
    int i;

    //这里加入一个读缓存
    read_cache_t read_cache;
    //初始化读缓存
    init_readcache_meta(&read_cache);

    //初始化一个函数来看看缓存命中率
    long all_count_of_block = 0;
    long hit_count_of_block = 0;

    //初始化一个预测序列
    predict_seq = (long*)malloc(MAX_PREDICT_DEEP*sizeof(long));
    //初始化给noah用的第二个序列
    predict_seq2 = (long*)malloc(MAX_PREDICT_DEEP*sizeof(long));
    
    printf("开始运行预测\n");

    if((fp_read = fopen("block_count_hm.csv", "r"))!=NULL){
        printf("开始读取访问的历史记录\n");
        
        //这里创建一个数组来存储
        page_init();

        //初始化noah
        Noah_predictor_t noah;
        Noah_predictor_init(&noah, 1);

        while(fgets(line, MAX_LINE_SIZE, fp_read)){
            //记录读进来的块
            all_count_of_block++;

            //将一行的内容转化为块号传入
            access_block_count = atol(line);

            //看看这个块是不是命中的
            if(search_read_cache(&read_cache, access_block_count) != NULL){
                hit_count_of_block++;
            }
            
            //在外部将传入的预测序列初始化
            memset(predict_seq, 0, MAX_PREDICT_DEEP*sizeof(long));
            memset(predict_seq2, 0 , MAX_PREDICT_DEEP*sizeof(long));

            page_predictor(access_block_count, predict_seq, &size);
            noah_predictor(&noah, access_block_count, predict_seq2, &size2);

            //我们将新的东西放到读缓存
            for(i = 0;  i < size2; i++){
                //当两个预测器预测地一样的时候才会写入
                add_cache_meta(&read_cache, predict_seq2[i]);
            }

            //打印一下预测到的东西
            // printf("预测:");
            // for(i = 0;  i < size; i++){
            //     printf(" %ld", predict_seq[i]);
            // }

            // printf("\n");
            
        }
        
    }else{
        printf("文件开启失败\n");
        return;
    }

    //关闭
    printf("读取完毕\n");
    fclose(fp_read);
    fp_read = NULL;

    //计算缓存命中率
    int hit_rate = hit_count_of_block  * 100 / all_count_of_block;

    //预测错误率
    int err_rate = read_cache.page_cache_have_not_access * 100 / read_cache.page_cache_del;

    printf("缓冲命中率:%d%%\n", hit_rate);
    printf("预测错误率:%d%%\n", err_rate);
}

int main(){
    predictor_manager();
}