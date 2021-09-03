" copy function name to body
:g/nvg[A-Z]/yw$s override {"i(_vg);o}
" remove nvg prefix
:%s/nvg\([A-Z]\)\(\w\+(\)/\L\1\e\2(/g
" copy params without types
let @a='s/\w\+[^\w,)]*\ \(\w\+[,)]\)/\1/g'
let @q='f(lyi)j$bi, ":akb ljjj0'
