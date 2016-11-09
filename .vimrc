" This is a project specific .vimrc

set tabstop=4
set softtabstop=4
set shiftwidth=4
set expandtab

" set filetype and better doxygen comments "
augroup project
    autocmd!
    autocmd BufRead,BufNewFile *.h,*.cpp set filetype=cpp.doxygen
augroup END

" Dont ask me every time you want to use you_complete_me plugin"
let g:ycm_confirm_extra_conf = 0

" set make as the :make command
set makeprg=make

" run make on F4, but don't show output (with !)
nnoremap <F6> :make!<cr>

" run you program "
nnoremap <F7> :!./my_great_program<cr>

" allow c-support plugin to use cmake and doxygen
let  g:C_UseTool_cmake   = 'yes'
let  g:C_UseTool_doxygen = 'yes'

" set up doxygen templates
call mmtemplates#config#Add ( 'C', '/home/artem/.vim/c-support/templates/doxygen.template', 'Doxygen', 'ntd' )

let g:DoxygenToolkit_blockHeader="-----------------------------------------------------------------------------"
let g:DoxygenToolkit_authorName="Artem Abramov"


" using GNU global source tagging system
set csprg=gtags-cscope
cs add GTAGS
" map so it works like tags
map g] :GtagsCursor<CR>


