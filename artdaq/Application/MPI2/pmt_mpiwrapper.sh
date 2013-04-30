#!/bin/bash

hostname=`hostname`

echo "STARTING:$hostname:$1:${@:2}"
$1 -p ${@:2} #&> $1.$2
echo "EXITING:$hostname:$?:$1:${@:2}"