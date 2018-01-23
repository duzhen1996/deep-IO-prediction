#!/bin/sh
gcc select-useful-data.c -o select-useful-data

gcc pageread-predictor.c predict-manager.c -o page_predictor
