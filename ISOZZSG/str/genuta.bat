cls
set utaYYMMDD=uta100407
xsltproc ..\..\..\%utaYYMMDD%\xsl\keyh.xsl dbs.xml > key.h
xsltproc ..\..\..\%utaYYMMDD%\xsl\maph.xsl dbs.xml > map.h
xsltproc ..\..\..\%utaYYMMDD%\xsl\dfth.xsl dbs.xml > dft.h
xsltproc ..\..\..\%utaYYMMDD%\xsl\mapc.xsl dbs.xml > map.c
xsltproc ..\..\..\%utaYYMMDD%\xsl\dftc.xsl dbs.xml > dft.c
xsltproc ..\..\..\%utaYYMMDD%\xsl\rptc.xsl dbs.xml > rpt.c
 pause