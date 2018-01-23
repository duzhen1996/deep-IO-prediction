#ifndef PAGEREAD_PREDICTOR_H
#define PAGEREAD_PREDICTOR_H


//保存着上次预测段的数组，用来判断是不是需要读取更多的段
#define MAX_HISTORY_ARR_SIZE 10
long history_arr[MAX_HISTORY_ARR_SIZE];

void page_init();
void page_predictor(long now_access, long *predictor_arr, int *size);


#endif