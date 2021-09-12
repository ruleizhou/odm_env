#!/bin/bash

if [ $# != 1 ];then
	echo "+++++++++++++++++++++++++++++++++++"
	echo "please use for this"
	echo "han_dtb2dts.sh dtb_name "
	echo "+++++++++++++++++++++++++++++++++++"
	exit 1;
fi

source_dir=`pwd`

./out/target/product/$TARGET_PRODUCT/obj/KERNEL_OBJ/scripts/dtc/dtc -I dtb -O dts -o 

