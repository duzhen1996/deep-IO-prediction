//这个函数是一个调度器，这个调度器接受一系列的预测器，并且接受一段的历史输入序列，从而可以得到预测的结果

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pageread-predictor.h"

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
    int size;
    int i;

    //初始化一个预测序列
    predict_seq = (long*)malloc(MAX_PREDICT_DEEP*sizeof(long));

    printf("开始运行预测\n");

    if((fp_read = fopen("block_count_hm.csv", "r"))!=NULL){
        printf("开始读取访问的历史记录\n");
        
        while(fgets(line, MAX_LINE_SIZE, fp_read)){
            //将一行的内容转化为块号传入
            access_block_count = atol(line);
            //这里创建一个数组来存储
            page_init();

            //在外部将传入的预测序列初始化
            memset(predict_seq, 0, MAX_PREDICT_DEEP*sizeof(long));
            
            page_predictor(access_block_count, predict_seq, &size);

            //打印一下预测到的东西
            // if(size == 6){
                printf("预测:");
                for(i = 0;  i < size; i++){
                    printf(" %ld", predict_seq[i]);
                }

                printf("\n");
            // }
            
        }
        
    }else{
        printf("文件开启失败\n");
        return;
    }

    //关闭
    printf("读取完毕\n");
    fclose(fp_read);
    fp_read = NULL;
}

int main(){
    predictor_manager();
}