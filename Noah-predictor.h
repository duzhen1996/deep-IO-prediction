#ifndef NOAH_PREDICTOR_H
#define NOAH_PREDICTOR_H

#define MAX_NOAH_RECORD_NUM 10000

//设计一个链表来获取历史后继记录
typedef struct _last_success_item{
    //有两个指针，一个指向前面，一个指向后面
    struct _last_success_item* front;
    struct _last_success_item* next;

    //这里记录块号
    long block_num;
    long success_num;
} last_success_item_t;

//历史后继记录表
typedef struct _last_success_records{
    //头部都一个空块
    last_success_item_t* head;
    //记录当前记录表的大小
    int size;
} last_success_records_t;

//这里放一个计数器，计数器本质上也是一个链表
typedef struct _counter_item{
    //指向链表收尾两个位置的指针
    struct _counter_item* front;
    struct _counter_item* next;

    //当前节点的块号
    long block_num;
    //相同后继计数
    int same_success_count;
} counter_item_t;

typedef struct _counter_record{
    //头部一个空块
    struct _counter_item* head;

    int size;
} counter_record_t;

//设计一个预测器，这个预测器包括一个历史后继的记录，稳定性，还有一个计数器这三个东西，计数器和历史后继记录都是链表
typedef struct _Noah_predictor{
    long last_access;
    int stability;
    last_success_records_t success_record;
    counter_record_t counter_record;
} Noah_predictor_t;

void Noah_predictor_init(Noah_predictor_t* input_predictor, int input_stability);

void noah_predictor(Noah_predictor_t* input_predictor, long now_access, long *predictor_arr, int *size);

long noah_find_last_succcess(last_success_records_t* success_record, long target_block);

void noah_add_time_of_counter(counter_record_t* counter_record, long target_block);

void noah_add_new_item_predictor_meta(Noah_predictor_t* input_predictor, long target_block, long last_success);

void noah_del_item_predictor_meta(Noah_predictor_t* input_predictor, long target_block);

#endif