#!/bin/bash
RED_COLOR='\E[1;31m'  #红
YELOW_COLOR='\E[1;33m' #黄
GREEN_COLOR='\E[1;35m' #黄
RES='\E[0m'

install_bin=false
install_vim=false
install_neovim=false

export install_dir=$PWD
echo install dir :$install_dir
#install odm env
while :
do
    clear -x
    # bin and sh
    if  [ $install_bin == false ]; then
        echo -e "${YELOW_COLOR}[1 ] install bin${RES}"
    else
        echo -e "${GREEN_COLOR}[1 ] install bin(ok)${RES}"
    fi
    # vim
    if  [ $install_vim == false ]; then
        echo -e "${YELOW_COLOR}[2 ] install vim${RES}"
    else
        echo -e "${GREEN_COLOR}[2 ] install vim(ok)${RES}"
    fi
    # neovim
    if  [ $install_neovim == false ]; then
        echo -e "${YELOW_COLOR}[3 ] install neovim${RES}"
    else
        echo -e "${GREEN_COLOR}[3 ] install neovim(ok)${RES}"
    fi
    
    echo -e "${YELOW_COLOR}[e] Exit${RES}"

    
    read -p "Select:" answer
    case ${answer} in
        1 )
            echo -e "${YELOW_COLOR}start install env${RES}"
            .  $install_dir/install_sh/install_bin.sh
            install_bin=true
            continue
        ;;
        2 )
            echo -e "${YELOW_COLOR}star tinstall copy${RES}"
            .  $install_dir/install_sh/install_vim.sh
            install_vim=true
            continue
        ;;
        3 )
            echo -e "${YELOW_COLOR}start install git${RES}"
            .  $install_dir/install_sh/install_neovim.sh
            install_neovim=true
            continue
        ;;
        e|E )
            exit 1
        ;;
        * )
            echo -e "${RED_COLOR}===Invalid input operation===${RES}"
            continue
        ;;
    esac
done
