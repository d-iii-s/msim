if &cp | set nocp | endif
let s:cpo_save=&cpo
set cpo&vim
map! <xHome> <Home>
map! <xEnd> <End>
map! <S-xF4> <S-F4>
map! <S-xF3> <S-F3>
map! <S-xF2> <S-F2>
map! <S-xF1> <S-F1>
map! <xF4> <F4>
map! <xF3> <F3>
map! <xF2> <F2>
map! <xF1> <F1>
vnoremap p :let current_reg = @"gvdi=current_reg
map <xHome> <Home>
map <xEnd> <End>
map <S-xF4> <S-F4>
map <S-xF3> <S-F3>
map <S-xF2> <S-F2>
map <S-xF1> <S-F1>
map <xF4> <F4>
map <xF3> <F3>
map <xF2> <F2>
map <xF1> <F1>
let &cpo=s:cpo_save
unlet s:cpo_save
set autoindent
set backspace=indent,eol,start
set helplang=en
set history=50
set printoptions=paper:a4
set ruler
set runtimepath=~/.vim,/etc/vim,/usr/share/vim/vimfiles,/usr/share/vim/addons,/usr/share/vim/vim63,/usr/share/vim/vimfiles,/usr/share/vim/addons/after,~/.vim/after
set suffixes=.bak,~,.swp,.o,.info,.aux,.log,.dvi,.bbl,.blg,.brf,.cb,.ind,.idx,.ilg,.inx,.out,.toc
set viminfo='20,\"50
