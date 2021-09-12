#!/bin/bash

if [ -d ~/.tools ]; then
    echo "Already install Tools"
else
    echo "Intall Tools "
    mkdir ~/.tools
    cp $install_dir/bin/* ~/.tools/ -rf
    echo "export PATH=\$PATH:~/.tools/Ubuntu_bin"
fi
