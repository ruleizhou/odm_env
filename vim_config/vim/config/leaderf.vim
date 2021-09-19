""""""""""""""""""""""""""""""
"Leaderf settings
""""""""""""""""""""""""""""""
" 空格键
let mapleader = "\<space>"

" 显示链接目录下的文件
let g:Lf_UseVersionControlTool = 0
let g:Lf_FollowLinks = 1

" 不显示图标
let g:Lf_ShowDevIcons = 0

" 忽略.gitignore指定文件及查找链接目录
let g:Lf_RgConfig = [
        \ "--no-ignore",
        \ "--no-ignore-global",
        \ "--no-ignore-parent",
        \ "--no-messages",
        \ "--follow"
    \ ]

" 忽略
let g:Lf_WildIgnore = {
        \ 'dir': ['.svn','.git','.hg', '.mypy_cache'],
        \ 'file': ['*.sw?','*.bak','*.exe','*.o','*.so']
    \}

" Popup mode
"let g:Lf_WindowPosition = 'popup'
"let g:Lf_PreviewInPopup = 1

" Config Leaderf gtags
let g:Lf_GtagsAutoGenerate = 1
let g:Lf_Gtagslabel = 'native-pygments'
let g:Lf_GtagsHigherThan6_6_2 = 0

"文件搜索
nnoremap <silent> <Leader>f :Leaderf file --no-ignore<CR>

"文件搜索
nnoremap <silent> <Leader>q :LeaderfRgRecall<CR>

"历史打开过的文件
nnoremap <silent> <Leader>m :Leaderf mru<CR>
"Buffer
nnoremap <silent> <Leader>b :Leaderf buffer<CR>

"函数搜索（仅当前文件里）
nnoremap <silent> <Leader>l :Leaderf function<CR>

"模糊搜索，很强大的功能，迅速秒搜
nnoremap <silent> <Leader>rg :Leaderf rg --no-ignore<CR>

"关闭
nnoremap <silent> <Leader>z :q<CR>

noremap <leader>gr :<C-U><C-R>=printf("Leaderf! gtags -r %s --auto-jump", expand("<cword>"))<CR><CR>
noremap <leader>gd :<C-U><C-R>=printf("Leaderf! gtags -d %s --auto-jump", expand("<cword>"))<CR><CR>
noremap <leader>go :<C-U><C-R>=printf("Leaderf! gtags --recall %s", "")<CR><CR>
noremap <leader>gn :<C-U><C-R>=printf("Leaderf gtags --next %s", "")<CR><CR>
noremap <leader>gp :<C-U><C-R>=printf("Leaderf gtags --previous %s", "")<CR><CR>
