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

    //这里为每一个桶都创建空间
    for(int i = 0; i < MAX_NOAH_BARREL_NUM; i++){
        // printf("初始化%d\n", i);
        //创建两个链表
        input_predictor->success_records[i].head = (last_success_item_t *)malloc(sizeof(last_success_item_t));
        //初始化
        memset(input_predictor->success_records[i].head, 0, sizeof(last_success_item_t));

        //后继记录链表头尾连一下
        input_predictor->success_records[i].head->front = input_predictor->success_records[i].head;
        input_predictor->success_records[i].head->next = input_predictor->success_records[i].head;
        input_predictor->success_records[i].head->block_num = -1;
        input_predictor->success_records[i].head->success_num = -1;
        input_predictor->success_records[i].head->same_success_count = -1;
    }

    // printf("初始化完毕\n");
}

//传入一个当前访问块，然后返回一个预测序列，如果不稳定或者没法预测，那就范围一个空序列
//对于历史后继和后继稳定性的计数也不能永无止境地存下去，因为总会有存不下的时候，我们为两个记录池选定10000
//记录在宏定义中
//一口气预测出6位，如果某一位之后开始预测不稳定或者预测不出来，那就在那一位之后开始返回-1
//所以短期预测和长期预测我们采用不同的策略。
void noah_predictor(Noah_predictor_t* input_predictor, long now_access, long *predictor_arr, int *size){
    //predictor是有实际空间的，从外部传入
    *size = 0;
    
    //我们首先需要知道上一次访问的是哪个数据块
    //如果上一次不是-1，那就进行记录
    long old_success;
    int barrel_num;
    printf("开始进行noah预测，noah_access=%ld\n", now_access);
    if(input_predictor->last_access != -1){
        //我们通过last_access来更新后继

        //先看看之前的后继是什么
        // printf("根据历史节点重新修订访问后继\n");
        //首先计算是在哪个桶，然后在传入函数
        barrel_num = (input_predictor->last_access) % MAX_NOAH_BARREL_NUM;
        old_success = noah_find_last_succcess(&(input_predictor->success_records[barrel_num]), input_predictor->last_access);

        //如果后继是当前节点，那就更新计时器
        if(old_success == now_access){
            // printf("和上次后继一致，更新计数\n");
            //更新计时器要将新修改的记录中心放到链表头部
            barrel_num = (input_predictor->last_access) % MAX_NOAH_BARREL_NUM;
            noah_add_time_of_counter(&(input_predictor->success_records[barrel_num]), input_predictor->last_access);
        } else if(old_success == -1){
            //到达这里说明上一次后继还不在
            //向两个record中添加之前不存在的新元素
            // printf("找不到对应元素后继，直接加入新的元数据\n");
            noah_add_new_item_predictor_meta(input_predictor, input_predictor->last_access, now_access);
        } else if(old_success != now_access){
            //这里说明我们要更换后继，先删除然后添加
            noah_del_item_predictor_meta(input_predictor, input_predictor->last_access);
            noah_add_new_item_predictor_meta(input_predictor, input_predictor->last_access, now_access);
        }

        //元数据更新完毕
    }

    //很据元数据选择后继
    //首先找到后继，开始针对当前节点
    long posible_successor;
    int  posible_count;
    long predictor_block = now_access;
    // printf("开始进行预测\n");
    barrel_num = predictor_block % MAX_NOAH_BARREL_NUM;
    while((posible_successor = noah_find_last_succcess(&(input_predictor->success_records[barrel_num]), predictor_block)) != -1){
        
        //如果预测超过6个就不再预测
        if(*size >= 6){
            break;
        }
        
        //这里可以找到后继
        //查看后继是不是到达了一定的稳定性
        // printf("处理稳定性\n");
        barrel_num = predictor_block % MAX_NOAH_BARREL_NUM;
        posible_count = noah_find_counter(&(input_predictor->success_records[barrel_num]), predictor_block);

        if(posible_count == -1){
            //不可能找不到
            // printf("不可能找不到对应的频次记录\n");
            break;
        }
        
        // printf("检查稳定性\n");
        if(posible_count > input_predictor->stability){
            // printf("开始进行预测第%d个元素\n", (*size)+1);
            //这里说明可以预测
            // printf("可以预测\n");
            predictor_arr[*size] = posible_successor;
            (*size)++;
            predictor_block = posible_successor;
            barrel_num = predictor_block % MAX_NOAH_BARREL_NUM;
            continue;
        }
        
        //到这里说明稳定性不足，不能预测
        break;
    }

    //最后重新更新上次访问的节点
    input_predictor->last_access = now_access;
}


int noah_find_counter(last_success_records_t* success_record, long target_block){
    last_success_item_t* scan = success_record->head->next;
    last_success_item_t* scan_front;
    last_success_item_t* scan_next;
    last_success_item_t* head_next;

    
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

            return scan->same_success_count;
        }

        scan = scan->next;
    }

    //这里说明没找到
    return -1;
}


long noah_find_last_succcess(last_success_records_t* success_record, long target_block){
    //遍历历史记录表，然后找到后继
    last_success_item_t* scan = success_record->head->next;
    last_success_item_t* scan_front;
    last_success_item_t* scan_next;
    last_success_item_t* head_next;

    //如果回到头部就停止遍历
    // printf("查找对应的节点的后继\n");
    while(scan != success_record->head){
        // printf("进入一次循\n");
        if(scan->block_num == target_block){
            // printf("找到了\n");
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

        // printf("向下走一位\n");

        scan = scan->next;
    }

    //到这里就说明没找到
    return -1;
}

void noah_add_time_of_counter(last_success_records_t* success_record, long target_block){
    //如果一个元素在last_success_record里面有，那么在counter_record里面一定也有
    last_success_item_t* scan = success_record->head->next;
    last_success_item_t* scan_front;
    last_success_item_t* scan_next;
    last_success_item_t* head_next;

    //遍历整个表，然后找到对应的节点
    while(scan != success_record->head){
        //看看是不是需要更改块号的
        if(scan->block_num == target_block){
            //找到了
            scan->same_success_count++;
            
            //挪到首位
            scan_front = scan->front;
            scan_next = scan->next;

            //重新拼接
            scan_front->next = scan_next;
            scan_next->front = scan_front;

            //将取出来的东西放在头部
            head_next = success_record->head->next;
            head_next->front = scan;
            scan->next = head_next;

            success_record->head->next = scan;
            scan->front = success_record->head;
            return;
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
    // printf("申请新元素的空间\n");
    int barrel_num = target_block % MAX_NOAH_BARREL_NUM;
    last_success_item_t* add_item_last_record = (last_success_item_t *)malloc(sizeof(last_success_item_t));
    last_success_item_t* last_success_head_next = (input_predictor->success_records[barrel_num]).head->next;
    //初始化
    memset(add_item_last_record, 0, sizeof(last_success_item_t));
    add_item_last_record->block_num = target_block;
    add_item_last_record->success_num = last_success;
    add_item_last_record->same_success_count = 1;

    //插入到链表头部
    input_predictor->success_records[barrel_num].head->next = add_item_last_record;
    add_item_last_record->front = input_predictor->success_records[barrel_num].head;

    last_success_head_next->front = add_item_last_record;
    add_item_last_record->next = last_success_head_next;

    //容量增加
    (input_predictor->success_records[barrel_num].size)++;

    //容量超标则剔除末尾元素
    if(input_predictor->success_records[barrel_num].size > MAX_NOAH_BARREL_SIZE){
        //容量超标
        // printf("容量超标，开始删除\n");
        last_success_item_t* last_success_head_front = input_predictor->success_records[barrel_num].head->front;
        
        //缝合一下
        last_success_head_front->front->next = last_success_head_front->next;
        last_success_head_front->next->front = last_success_head_front->front;
        
        free(last_success_head_front);

        (input_predictor->success_records[barrel_num].size)--;
    }

    // printf("在successor_record中插入成功\n");
}


//往预测器的记录中删除元素
void noah_del_item_predictor_meta(Noah_predictor_t* input_predictor, long target_block){
    //从两个元数据中删除数据
    //按道理来讲，两个记录里面的顺序是一样的
    int barrel_num = target_block % MAX_NOAH_BARREL_NUM;
    last_success_item_t* scan_last_success = input_predictor->success_records[barrel_num].head->next;

    //开始删除元素
    while(scan_last_success != input_predictor->success_records[barrel_num].head){
        if(scan_last_success->block_num == target_block){
            //同时删除两个节点里面的元素，因为顺序是一样的
            scan_last_success->front->next = scan_last_success->next;
            scan_last_success->next->front = scan_last_success->front;

            free(scan_last_success);
            (input_predictor->success_records[barrel_num].size)--;
            
            return;
        }

        scan_last_success = scan_last_success->next;
    }
}


