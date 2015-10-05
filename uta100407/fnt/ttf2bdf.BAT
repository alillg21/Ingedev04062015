: Usage: ttf2bdf.bat <ttf-file> <bdf-file> <point-size>
:  - <ttf-file>: input ttf file
:  - <bdf-file>: output bdf file
:  - <point-size> point size
: Example: ttf2bdf.bat cour.ttf cour.bdf 16
otf2bdf -r 75 -p %3 -o %2 %1
