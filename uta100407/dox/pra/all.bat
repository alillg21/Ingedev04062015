copy ..\*.gif html
copy ..\*.png html
doxygen pra.dox
hhc html/index.hhp 
move html\index.chm pra.chm
:pause 1