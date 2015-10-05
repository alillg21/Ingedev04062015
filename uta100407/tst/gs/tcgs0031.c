// tcgs0031
// Intop testcase of canvas and barcode
// author:      Gábor Sárospataki

#include <unicapt.h>
#include <string.h>
#include <stdio.h>
#include "sys.h"
static const char *srcFile = __FILE__;

#define CHK CHECK(ret>=0,lblKO)

void tcgs0031(void) {
    nvmStart();
    cnvStart();
    prtStart();
    bcrStart();
    prtS("TCGS0031");
    prtS("   ");
    prtS("   ");
    prtS("   ");
    tmrPause(1);
    cnvStop();
    bcrStop();
    prtStop();
}
