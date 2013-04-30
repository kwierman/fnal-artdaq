#!/bin/bash

ps -A aww -o pid,cmd --no-header | grep "$1 -p ${@:2}" | grep -v grep | sed "s/^[ ]*//" | cut -d" " -f1 | xargs kill
sleep 3
ps -A aww -o pid,cmd --no-header | grep "$1 -p ${@:2}" | grep -v grep | sed "s/^[ ]*//" | cut -d" " -f1 | xargs kill -9
