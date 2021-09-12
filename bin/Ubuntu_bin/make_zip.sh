#!/bin/bash

date=`date +%y%m%d`
zip_dir=/home/han/system_zip
pwd_dir=`pwd`
project_name=$1
mkdir $zip_dir/$date -p

cd $pwd_dir

echo "copy img"
cp *.img android-info*.txt $zip_dir/$date
echo "zip img"
cd $zip_dir/$date
echo "name $project_name"
zip -r $project_name\_$date.zip ./*
mv $project_name\_$date.zip ../
cd -
