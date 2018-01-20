//这个函数是原始数据集要经过的第一个函数
//这个函数所做的工作就是将所有读的请求筛选出来，并且将块号存储按照顺序存储下来，放到一个csv文件中
//csv的文件结构非常简单，当做文本文件来处理，主要就是下面的这个结构：

/**
128166372010810581,mds,0,Write,2654453760,4096,54086
128166372011594132,mds,0,Write,3154132992,4096,51785
128166372011600556,mds,0,Write,3201662976,20480,45361
128166372011606862,mds,0,Write,3154124800,4096,39055
128166372017062367,mds,0,Write,57819136,4096,52301
128166372017074528,mds,0,Write,57823232,4096,40140
128166372017085606,mds,0,Write,57806848,4096,29061
128166372017220587,mds,0,Write,3154137088,4096,50331
128166372017230760,mds,0,Write,57810944,4096,40158
128166372020823372,mds,0,Write,156237824,4096,41296
128166372052829342,mds,0,Write,3201683456,4096,66576
128166372052830076,mds,0,Write,1509126144,512,65841
128166372053007505,mds,0,Write,3240710144,4096,44663
128166372053019821,mds,0,Write,3233218560,4096,32346
128166372053023423,mds,0,Write,3154132992,4096,28745
128166372053122921,mds,0,Write,1253568512,1536,85496
128166372053136474,mds,0,Write,1214406656,4096,71944
128166372053150930,mds,0,Write,1509126656,1024,57488
128166372053168971,mds,0,Write,1509127680,2560,39446
**/

//所以我们一行行读出来，然后使用逗号分隔开
//将块大小设定为4K
#define PAGE_SIZE 4096

//每一行的最大字节数，同时也是行缓冲区的大小
#define MAX_LINE_SIZE 1024

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(void){
    FILE *fp_read = NULL;
    FILE *fp_write = NULL;
    char line[MAX_LINE_SIZE];
    
    printf("开始运行\n");

    if((fp_read = fopen("resource/MSR-Cambridge/mds_0.csv","r")) != NULL)
    {   
        printf("开始读取\n");

        //这里开始写文件
        if((fp_write = fopen("block_count.csv", "w+")) == NULL){
            printf("文件写开启失败\n");
            fclose(fp_read);
            fp_read = NULL;
            return -1;
        }

        char delims[] = ",";
        char *result = NULL;

        while (fgets(line, MAX_LINE_SIZE, fp_read)){
            //首先看看是不是read，如果里面有read再打印
            //strstr是用来匹配子串的函数
            if(strstr(line, "Write") != NULL){
                continue;
            }

            //这里是个分割的元素
            result = strtok( line, delims );

            // printf("%s", result);

            //这里要进行计数，只保留第5列
            int i = 1;

            //只要还有可以分的，那result不会是NULL
            while((result = strtok( NULL, delims))){
                i++;
                if(i == 5){
                    //将字符串换成数字
                    long block_count = atol(result) / PAGE_SIZE;

                    //这里将块访问记录导入到一个新的csv文件中
                    fprintf(fp_write, "%ld\n", block_count);
                    printf("%ld\n", block_count);
                }
            }         
        }

        printf("读取完毕\n");
        fclose(fp_read);
        fclose(fp_write);
        fp_read = NULL;
        fp_write = NULL;
    }
}