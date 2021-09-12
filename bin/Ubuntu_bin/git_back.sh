#!/bin/bash

if [ $# != 2 ];then
	echo "+++++++++++++++++++++++++++++++++++"
	echo "please use for this"
	echo "git_back.sh git_dir back_dir"
	echo "+++++++++++++++++++++++++++++++++++"
	exit 1;
fi

source_dir=`pwd`

git_dir=$1
back_name=$2
file=git_back.tar.gz
back_dir=$work_path/han/backup/file_backup/git_back/$back_name
#destination=$back_dir\/$file;
destination=$back_dir\/$file;
git_file="git.log";

mkdir -p $back_dir
cd $source_dir
git status -s $git_dir > $git_file

sed -i 's/^..//' $git_file
sed -i 's/git.log//' $git_file

if [ -f $git_file ]
	then
		echo "load $git_file";
	else
    	echo "Sorry,can not find config file $git_file";
    	exit 1;
fi
file_number=0;
exec < $git_file;
read file_name;
while [ $? -eq 0 ]  # read命令执行结果为0
	do
	    if [ -f $file_name -o -d $file_name ] #-o 表示or
	    then
	        file_list="$file_list "$file_name;
	    else
	        echo "$file_name does not exist";
	        echo "the number is $file_number";
	    fi
	    file_number=$[ $file_number + 1 ];
	    read file_name;
	done
tar -czPf $destination $file_list
echo "destination $destination"
echo "rm git_file $git_file"
rm $git_file 
cd $back_dir
echo "git push https://github.com/ruleizhou/Code_backup.git"
git add ../
git commit -m "Update $back_name"
git push origin main
echo "tar && rm git_back.tar.gz"
tar -xzf $file
rm $file
cd $source_dir

