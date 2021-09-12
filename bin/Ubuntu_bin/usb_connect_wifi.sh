#!/bin/bash

if [ "x$1" == "xf" ];then
# 有时在wifi断了后， 又重新连上了，可以直接使用./wifi.sh f
    phone_ip=`cat /tmp/phone_ip.1234`
    if [ "x$phone_ip" != "x" ];then
        echo "connect to $phone_ip"
        adb -s $phone_ip connect $phone_ip
    fi
    exit 0
fi

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

#port=`adb -s $device wait-for-device && adb -s $device shell "getprop service.adb.tcp.port"`
#if [ "x$port" != "x5555" ];then
adb -s $device wait-for-device && adb -s $device root
adb -s $device wait-for-device && adb -s $device shell "setprop service.adb.tcp.port 5555"
#adb -s $device wait-for-device && adb -s $device shell "start adbd"
adb -s $device wait-for-device && adb -s $device tcpip 5555
sleep 1
#fi
tmp=`adb -s $device wait-for-device && adb -s $device shell ifconfig |  grep "10.33" | awk -F ':' '{print $2}' | awk '{print $1}'`
if [ "x$tmp" == "x" ];then
    echo "ip is none"
    exit -1
fi
echo "connect to $tmp"
echo "$tmp:5555" > /tmp/phone_ip.1234
adb -s $tmp:5555 connect $tmp