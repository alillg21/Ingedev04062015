copy ..\*.gif html
copy ..\*.png html
doxygen pru.dox
hhc html/index.hhp 
move html\index.chm pru.chm
:pause 1