#!/bin/bash

if [ -d ~/.vim ]; then
    echo "Already config vim"
else
    echo "Config vim "
    cp $install_dir/vim_config/vim ~/.vim -rf
    cp $install_dir/vim_config/vimrc ~/.vimrc
fi
