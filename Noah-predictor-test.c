//用来测试Noah预测器的程序
#include"Noah-predictor.h"
#include<stdlib.h>
#include<stdio.h>
#include<memory.h>

int main(){
    Noah_predictor_t noah;

    Noah_predictor_init(&noah, 1);

    //创建一个数组
    long* predictor_arr = (long *)malloc(sizeof(long)*10);

    int size = 0;

    noah_predictor(&noah, 1, predictor_arr, &size);

    printf("%d\n",size);
}