" Specify a directory for plugins
" - For Neovim: stdpath('data') . '/plugged'
" - Avoid using standard Vim directory names like 'plugin'
call plug#begin('~/.config/nvim/plugged')

" Any valid git URL is allowed
Plug 'https://github.com/universal-ctags/ctags.git'
Plug 'https://github.com/ludovicchabant/vim-gutentags.git'
Plug 'https://github.com/BurntSushi/ripgrep.git'
Plug 'https://github.com/Yggdroot/LeaderF.git'
Plug 'https://github.com/preservim/nerdtree.git'
Plug 'https://github.com/preservim/tagbar.git'
Plug 'vim-airline/vim-airline'
Plug 'vim-airline/vim-airline-themes'
Plug 'jiangmiao/auto-pairs'
Plug 'luochen1990/rainbow'

" Initialize plugin system
call plug#end()
