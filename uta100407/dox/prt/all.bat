copy ..\*.gif html
copy ..\*.png html
doxygen prt.dox 
:> out 2>err
hhc html/index.hhp 
move html\index.chm prt.chm
:pause 1