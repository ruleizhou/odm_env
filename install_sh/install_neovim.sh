#!/bin/bash

if [ -d ~/.config/nvim ]; then
    echo "Already config neovim"
else
    echo "Config neovim "
    cp $install_dir/nvim_config ~/.config/nvim -rf
fi
