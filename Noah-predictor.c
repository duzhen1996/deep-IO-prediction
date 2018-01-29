#include<stdlib.h>
#include<stdio.h>
#include<memory.h>
#include"Noah-predictor.h"


//这里进行Noah预测器的初始化
//在外部传入一个预测器的指针
void Noah_predictor_init(Noah_predictor_t* input_predictor, int input_stability){
    printf("Noah预测器初始化\n");
    
    input_predictor->stability = input_stability;
    input_predictor->last_access = -1;

    //创建两个链表
    input_predictor->success_record.head = (last_success_item_t *)malloc(sizeof(last_success_item_t));
    //初始化
    memset(input_predictor->success_record.head, 0, sizeof(last_success_item_t));

    //后继记录链表头尾连一下
    input_predictor->success_record.head->front = input_predictor->success_record.head;
    input_predictor->success_record.head->next = input_predictor->success_record.head;
    input_predictor->success_record.head->block_num = -1;
    input_predictor->success_record.head->success_num = -1;

    input_predictor->counter_record.head = (counter_item_t *)malloc(sizeof(counter_item_t));
    memset(input_predictor->counter_record.head, 0, sizeof(counter_record_t));

    input_predictor->counter_record.head->front = input_predictor->counter_record.head;
    input_predictor->counter_record.head->next = input_predictor->counter_record.head;
    input_predictor->counter_record.head->block_num = -1;
    input_predictor->counter_record.head->same_success_count = -1;
}

//传入一个当前访问块，然后返回一个预测序列，如果不稳定或者没法预测，那就范围一个空序列
//对于历史后继和后继稳定性的计数也不能永无止境地存下去，因为总会有存不下的时候，我们为两个记录池选定10000
//记录在宏定义中
//一口气预测出6位，如果某一位之后开始预测不稳定或者预测不出来，那就在那一位之后开始返回-1
//所以短期预测和长期预测我们采用不同的策略。
void noah_predictor(Noah_predictor_t* input_predictor, long now_access, long *predictor_arr, int *size){
    //我们首先需要知道上一次访问的是哪个数据块
    //如果上一次不是-1，那就进行记录
    long old_success;

    if(input_predictor->last_access != -1){
        //我们通过last_access来更新后继

        //先看看之前的后继是什么
        old_success = noah_find_last_succcess(&input_predictor->success_record, input_predictor->last_access);

        //如果后继是当前节点，那就更新计时器
        if(old_success == now_access){
            //更新计时器要将新修改的记录中心放到链表头部
            noah_add_time_of_counter(&input_predictor->counter_record, input_predictor->last_access);
        } else if(old_success == -1){
            //到达这里说明上一次后继还不在
            //向两个record中添加之前不存在的新元素
            noah_add_new_item_predictor_meta(&input_predictor, input_predictor->last_access, now_access);
        } else if(old_success != now_access){
            //这里说明我们要更换后继，先删除然后添加
            noah_del_item_predictor_meta(&input_predictor, input_predictor->last_access);
            noah_add_new_item_predictor_meta(&input_predictor, input_predictor->last_access, now_access);
        }

        //元数据更新完毕
    }

    //很据元数据选择后继
    




    //最后重新更新上次访问的节点
    input_predictor->last_access = now_access;
}


long noah_find_last_succcess(last_success_records_t* success_record, long target_block){
    //遍历历史记录表，然后找到后继
    last_success_item_t* scan = success_record->head->next;
    last_success_item_t* scan_front;
    last_success_item_t* scan_next;
    last_success_item_t* head_next;

    //如果回到头部就停止遍历
    while(scan != success_record->head){
        if(scan->block_num == target_block){
            //这就就说明找到了
            //如果找到了，那就把这个节点放到头部，使链表中的记录新的节点放到前面
            scan_front = scan->front;
            scan_next = scan->next;

            //将前后两个位置封上
            scan_front->next = scan_next;
            scan_next->front = scan_front;

            //然后将scan指针插到链表头部
            head_next = success_record->head->next;

            head_next->front = scan;
            scan->next = head_next;

            success_record->head->next = scan;
            scan->front = success_record->head;

            return scan->success_num;
        }

        scan = scan->next;
    }

    //到这里就说明没找到
    return -1;
}

void noah_add_time_of_counter(counter_record_t* counter_record, long target_block){
    //如果一个元素在last_success_record里面有，那么在counter_record里面一定也有
    counter_item_t* scan = counter_record->head->next;
    counter_item_t* scan_front;
    counter_item_t* scan_next;
    counter_item_t* head_next;

    //遍历整个表，然后找到对应的节点
    while(scan != counter_record->head){
        //看看是不是需要更改块号的
        if(scan->block_num == target_block){
            //找到了
            scan->same_success_count++;
            
            //挪到首位
            scan_front = scan->front;
            scan_next = scan->next;

            //重新拼接
            scan_front->next = scan;
            scan_next->front = scan;

            //将取出来的东西放在头部
            head_next = counter_record->head->next;
            head_next->front = scan;
            scan->next = head_next;

            counter_record->head->next = scan;
            scan->front = counter_record->head;
            break;
        }
        scan = scan->next;
    }

    //不可能到达这里
    printf("没有在counter_record中找到对应记录，这是不应该的\n");
}

//这里在两个record中添加新的元素
void noah_add_new_item_predictor_meta(Noah_predictor_t* input_predictor, long target_block, long last_success){
    //这里向记录中添加新元素
    //首先先添加last_success_record
    last_success_item_t* add_item_last_record = (last_success_item_t *)malloc(sizeof(last_success_item_t));
    last_success_item_t* last_success_head_next = input_predictor->success_record.head->next;
    //初始化
    memset(add_item_last_record, 0, sizeof(last_success_item_t));
    add_item_last_record->block_num = target_block;
    add_item_last_record->success_num = last_success;

    //插入到链表头部
    input_predictor->success_record.head->next = add_item_last_record;
    add_item_last_record->front = input_predictor->success_record.head;

    //容量增加
    input_predictor->success_record.size++;

    //容量超标则剔除末尾元素
    if(input_predictor->success_record.size > MAX_NOAH_RECORD_NUM){
        last_success_item_t* last_success_head_front = input_predictor->success_record.head->front;
        
        //缝合一下
        input_predictor->success_record.head->front = last_success_head_front->front;
        last_success_head_front->next = input_predictor->success_record.head;
        
        free(last_success_head_front);

        input_predictor->success_record.size--;
    }

    //然后在counter_record中添加元素
    counter_item_t* add_item_counter_record = (counter_item_t *)malloc(sizeof(counter_item_t));
    counter_item_t* counter_head_next = input_predictor->counter_record.head->next;
    //初始化
    memset(add_item_counter_record, 0, sizeof(counter_item_t));
    //插入这个表肯定已经有一次后继访问了
    add_item_counter_record->block_num = target_block;
    add_item_counter_record->same_success_count = 1;

    //插入到头部
    input_predictor->counter_record.head->next = add_item_counter_record;
    add_item_counter_record->front = input_predictor->counter_record.head;

    counter_head_next->front = add_item_counter_record;
    add_item_counter_record->next = counter_head_next;

    input_predictor->counter_record.size++;

    if(input_predictor->counter_record.size > MAX_NOAH_RECORD_NUM){
        counter_item_t* counter_head_front = input_predictor->counter_record.head->front;
        
        //缝合一下
        input_predictor->counter_record.head->front = counter_head_front->front;
        counter_head_front->front->next = input_predictor->counter_record.head;

        free(counter_head_front);

        input_predictor->counter_record.size--;
    }
}


//往预测器的记录中删除元素
void noah_del_item_predictor_meta(Noah_predictor_t* input_predictor, long target_block){
    //从两个元数据中删除数据
    //按道理来讲，两个记录里面的顺序是一样的
    last_success_item_t* scan_last_success = input_predictor->success_record.head->next;
    counter_item_t* scan_counter = input_predictor->counter_record.head->next;

    //开始删除元素
    while(scan_last_success != input_predictor->success_record.head){
        if(scan_last_success->block_num == target_block){
            //同时删除两个节点里面的元素，因为顺序是一样的
            scan_last_success->front->next = scan_last_success->next;
            scan_last_success->next->front = scan_last_success->front;

            free(scan_last_success);
            input_predictor->success_record.size--;
            
            scan_counter->front->next = scan_counter->next;
            scan_counter->next->front = scan_counter->front;

            if(scan_counter->block_num ！= target_block){
                printf("Noah的两个record竟然没有对齐，不存在的\n");
            }

            free(scan_counter);
            input_predictor->counter_record.size--;
            break;
        }

        scan_last_success = scan_last_success->next;
        scan_counter = scan_counter->next;
    }
}
