#!/bin/bash
RED_COLOR='\E[1;31m'  #红
YELOW_COLOR='\E[1;33m' #黄
GREEN_COLOR='\E[1;35m' #黄
RES='\E[0m'

export han_build_dir=$han_work_path/han/Linux_env/linux/bin

project_name=(
            ocl
            ocl_jp
            ocm
            ocn
            miac
            mi6
            ime
            exodus
)

source_dir=`pwd`
cd $source_dir

while :
do
    clear
    echo -e "${YELOW_COLOR}build menu${RES}"
    for((i=0;i<${#project_name[@]};i++)) do
        echo -e "${YELOW_COLOR}[${i} ] ${project_name[i]}${RES}"
    done
    
    read -p "Select:" answer
    case ${answer} in
        [0-9] )
            echo -e "${YELOW_COLOR}start install xed${RES}"
            . $han_build_dir/project/${project_name[answer]}/build_menu.sh
            break
        ;;
        [0-9][0-9] )
            echo -e "${YELOW_COLOR}start install xed${RES}"
            . $han_build_dir/project/${project_name[answer]}/build_menu.sh
            break
        ;;
        e|E )
            break
        ;;
        * )
            echo -e "${RED_COLOR}===Invalid input operation===${RES}"
            continue
        ;;
    esac   
done

