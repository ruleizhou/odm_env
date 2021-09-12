#!/bin/bash

if [ -d ~/.vim ]; then
    echo "Already config vim"
else
    echo "Config vim "
    cp $install_dir/vim/vim ~/.vim -rf
    cp $install_dir/vim/vimrc ~/.vimrc
fi
