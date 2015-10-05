:..\Ttf2bdf.exe -r 100 -p 10 -c 1 -l '0_255'  cour.ttf> normal.bdf
..\Ttf2bdf.exe -r 100 -p 10 -l '0_255'  courbd.ttf> bold.bdf
..\Ttf2bdf.exe -r 128 -p 12 -l '0_255'  cour.ttf> big.bdf
: ..\TTf2bdf.exe -r 128 -p 12 -l '0_255'  cour.ttf> pnormal.bdf

..\bdf2fon -c -w8 -x0 -y0 -tISO1 -fnormal.bdf -s_dNORMAL_ cour.FON
..\bdf2fon -w8 -x0 -y0 -fbold.bdf -s_dBOLD_ cour.FON
..\bdf2fon -w16 -x0 -y0 -fbig.bdf -s_dBIG_ cour.FON
: ..\bdf2fon -w16 -x0 -y0 -fpnormal.bdf -s_pNORMAL_ cour.FON
..\bdf2fon -w16 -x0 -y0 -fpnormal.bdf -s_pNORMAL_ cour.FON
..\bdf2fon -w16 -x0 -y0 -fpbold.bdf -s_pBOLD_ cour.FON
..\bdf2fon -w16 -x0 -y0 -fphigh.bdf -s_pHIGH_ cour.FON
..\bdf2fon -w32 -x0 -y0 -fpbig.bdf -s_pBIG_ cour.FON
..\bdf2fon -w24 -x0 -y0 -fpmedium.bdf -s_pMEDIUM_ cour.FON
..\bdf2fon -w8 -x0 -fpcondensed.bdf -s_pCONDENSED_ cour.FON

..\bdf2xml normal.bdf normal.xml
..\bdf2xml bold.bdf bold.xml
..\bdf2xml big.bdf big.xml
..\bdf2xml pNormal.bdf pNormal.xml
..\bdf2xml pBold.bdf pBold.xml
..\bdf2xml pHigh.bdf pHigh.xml
..\bdf2xml pBig.bdf pBig.xml
..\bdf2xml pMedium.bdf pMedium.xml
..\bdf2xml pCondensed.bdf pCondensed.xml




