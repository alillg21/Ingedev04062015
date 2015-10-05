// tcgs0030
// Checking of the file size
// author:      Gábor Sárospataki

#include <unicapt.h>
#include <string.h>
#include <stdio.h>
#include "sys.h"

#define CHK CHECK(ret>=0,lblKO)

void tcgs0030(void) {
    int ret;
    byte logic;
    byte sec = 0;

    dspStart();
    nvmStart();
    nvmRemove(sec);
    dspLS(0, "Exp. size: 0");
    logic = (nvmSize(sec) == 0);
    if(logic)
        dspLS(1, "Correct");
    else
        dspLS(1, "Incorrect");
    tmrPause(3);

    dspLS(0, "Create file");
    dspLS(1, "100 byte");
    ret = nvmSet(sec, ' ', 0, 100);
    CHK;
    tmrPause(1);

    dspLS(0, "Exp. size: 100");
    logic = (nvmSize(sec) == 100);
    if(logic)
        dspLS(1, "Correct");
    else
        dspLS(1, "Incorrect");
    tmrPause(3);

    goto lblEnd;
  lblKO:
    dspLS(0, "KO!");
    tmrPause(3);
  lblEnd:
    dspStop();
}
