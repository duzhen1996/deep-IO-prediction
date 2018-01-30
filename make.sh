#!/bin/sh
gcc select-useful-data.c -o select-useful-data

gcc Noah-predictor.c pageread-predictor.c readcache-with-LRU.c predict-manager.c -o predictor

#测试读缓存元数据
gcc readcache-with-LRU.c test-readcache-with-LRU.c -o test-readcache-with-LRU

#测试noah
gcc Noah-predictor.c Noah-predictor-test.c -o Noah-predictor-test