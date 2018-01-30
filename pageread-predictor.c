#include<memory.h>
#include<stdlib.h>
#include<stdio.h>
#include"pageread-predictor.h"

//这里是一个普通的段预读策略分别进行将这个函数每次传入一个当前的访问节点，然后自己维护一定数量的历史记录
//我们通过判断当前访问的节点在不在之前预测的块中来判断当前要预读的数量



/**
 *一个形参，第一个形参是返回值，是指向一个数组的指针
 *这个数组带外部就已经分配好空间，是一个20位的缓冲区
 *我们需要将预测序列填满，并且向外传出有效空间的大小
 **/


//这里是传统页预测器的初始化函数
void page_init(){
    printf("初始化页顺序预测器\n");
    memset(&history_arr,0,MAX_HISTORY_ARR_SIZE*sizeof(long));
}


void page_predictor(long now_access, long *predictor_arr, int *size){
    if(predictor_arr == NULL){
        printf("predictor_arr必须在外部分配好空间、\n");
        return;
    }
    
    printf("开始进行page顺序预测，now_access = %ld\n", now_access);
    //开始预测，看看之前的预测是不是对的
    //查看之前的预测结果

    int i;

    //要返回的块编号的数量
    int return_num = 3;

    for(i = 0; i < MAX_HISTORY_ARR_SIZE; i++){
        //开始检查上一次预测的记录
        if(history_arr[i] == now_access){
            //这里说明这个块是上次预测到的
            return_num = 6;
            break;
        }
    }

    //初始化历史函数，等待下一次使用
    memset(&history_arr,0,MAX_HISTORY_ARR_SIZE*sizeof(long));

    //如果没有预测到，那就依旧返回三个，如果预测到了那就返回6个
    for(i = 0; i < return_num; i++){
        predictor_arr[i] = now_access + i + 1;
        //重新定义历史预测函数
        history_arr[i] = now_access + i + 1;
    }
    
    *size = return_num; 
    
}