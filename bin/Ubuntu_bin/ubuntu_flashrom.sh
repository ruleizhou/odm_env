#!/bin/bash

current_path=`pwd`
#export PATH=$current_path:$PATH
echo "$current_path"
adb reboot bootloader

fastboot flash xbl xbl.elf
fastboot flash xblbak xbl.elf
fastboot flash tz tz.mbn
fastboot flash tzbak tz.mbn
fastboot flash rpm rpm.mbn
fastboot flash rpmbak rpm.mbn
fastboot flash hyp hyp.mbn
fastboot flash hypbak hyp.mbn
fastboot flash pmic pmic.elf
fastboot flash pmicbak pmic.elf

fastboot flash keymaster keymaster64.mbn
fastboot flash keymasterbak keymaster64.mbn
fastboot flash cmnlib cmnlib.mbn
fastboot flash cmnlibbak cmnlib.mbn
fastboot flash cmnlib64 cmnlib64.mbn
fastboot flash cmnlib64bak cmnlib64.mbn
fastboot flash mdtpsecapp mdtpsecapp.mbn
fastboot flash mdtpsecappbak mdtpsecapp.mbn
fastboot flash modem NON-HLOS.bin
fastboot flash dsp dspso.bin
fastboot flash abl abl.elf
fastboot flash ablbak abl.elf
fastboot flash bluetooth BTFM.bin
fastboot flash bluetoothbak BTFM.bin
fastboot flash storsec storsec.mbn
fastboot flash devcfg devcfg.mbn
fastboot flash devcfgbak devcfg.mbn
fastboot flash devinfo devinfo.bin
fastboot flash devinfo devinfo.bin
fastboot flash apppreload apppreload.img
fastboot flash cota cota.img

fastboot flash boot boot.img
fastboot flash recovery recovery.img
fastboot flash system system.img
fastboot flash vendor vendor.img
fastboot flash mdtp mdtp.img
fastboot flash mdtpbak mdtp.img
fastboot flash cache cache.img
fastboot flash userdata userdata.img
fastboot flash splash splash.img

echo
echo @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
echo Flash success!
echo Press any key to restart the device!
echo @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
read -n 1

fastboot reboot

