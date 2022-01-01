" Specify a directory for plugins
" - For Neovim: stdpath('data') . '/plugged'
" - Avoid using standard Vim directory names like 'plugin'
call plug#begin('~/.vim/plugged')

" Any valid git URL is allowed
Plug 'universal-ctags/ctags.git'
Plug 'ludovicchabant/vim-gutentags.git'
Plug 'skywind3000/gutentags_plus.git'
Plug 'BurntSushi/ripgrep.git'
Plug 'Yggdroot/LeaderF.git'
Plug 'preservim/nerdtree.git'
Plug 'https://github.com/preservim/tagbar.git'
Plug 'vim-airline/vim-airline'
Plug 'vim-airline/vim-airline-themes'
Plug 'jiangmiao/auto-pairs'
Plug 'luochen1990/rainbow'
Plug 'inkarkat/vim-mark'
Plug 'inkarkat/vim-ingo-library'

" Initialize plugin system
call plug#end()
