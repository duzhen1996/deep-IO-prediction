#!/bin/sh
gcc select-useful-data.c -o select-useful-data

gcc pageread-predictor.c readcache-with-LRU.c predict-manager.c -o page_predictor

#测试读缓存元数据
gcc readcache-with-LRU.c test-readcache-with-LRU.c -o test-readcache-with-LRU