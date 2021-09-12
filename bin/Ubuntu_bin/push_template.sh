#!/bin/bash

devices=`adb devices | grep "\<device\>" | awk '{print $1}'`

count=0
for i in $devices
do
    let count++
done

if [ $count -gt 1 ];then
select device in $devices;
do
    break;
done
elif [ $count -eq 0 ];then
    device=""
else
    device=$devices
fi

if [ "x$device" == "x" ];then
    echo "device: $device not exist"
    exit -1
fi

adb -s $device wait-for-device && adb -s $device root && adb -s $device remount
adb -s $device wait-for-device && adb -s $device cmd




