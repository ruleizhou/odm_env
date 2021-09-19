"去掉vi的一致性"
set nocompatible
"显示行号"
set number
" 隐藏滚动条"    
set guioptions-=r 
set guioptions-=L
set guioptions-=b
"隐藏顶部标签栏"
set showtabline=0
syntax on   "开启语法高亮"
set nowrap  "设置不折行"
set fileformat=unix "设置以unix的格式保存文件"
set cindent     "设置C样式的缩进格式"
set ts=4
set tabstop=4   "设置table长度"
set shiftwidth=4        "同上"
set expandtab
set autoindent
set showmatch   "显示匹配的括号"
set scrolloff=5     "距离顶部和底部5行"
set laststatus=2    "命令行为两行"
set fenc=utf-8      "文件编码"
set backspace=2
set mouse=nh     "启用鼠标 n：普通模式 v：可视模式 i：插入模式 c：命令行模式 h：帮助文档"
set selection=exclusive
set selectmode=mouse,key
set matchtime=5
set ignorecase      "忽略大小写"
set incsearch
set hlsearch        "高亮搜索项"
set whichwrap+=<,>,h,l
set autoread
set cursorline      "突出显示当前行"
set clipboard=unnamed
" 自动删除行尾空格
autocmd BufWritePre *.c :%s/\s\+$//e
