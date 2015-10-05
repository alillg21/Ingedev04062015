// tcgs0028
// Global files
// author:      Gábor Sárospataki

#include <unicapt.h>
#include <string.h>
#include <stdio.h>
#include "sys.h"
static code const char *srcFile = __FILE__;

#define CHK CHECK(ret>=0,lblKO)

void tcgs0029(void) {
    int ret;
    uint32 hdl;
    char buf[1024];
    char prt[dspW + 1];
    int get, time;
    int idx;

    memset(buf, 0, sizeof(buf));
    prtStart();
    nvmStart();

    for (idx = 0; idx < sizeof(buf); idx++)
        buf[idx] = idx % 0xff;

    prtS("Global file speed (1024 byte)");
    tmrStart(1, 60);
    get = tmrGet(1);
    ret = nvmSave(0xf1, buf, 0, sizeof(buf));
    CHK;
    time = -tmrGet(1) + get;
    tmrStop(1);
    sprintf(prt, "Save: %d", time);
    prtS(prt);

    tmrStart(1, 60);
    get = tmrGet(1);
    ret = nvmLoad(0xf1, buf, 0, sizeof(buf));
    CHK;
    time = -tmrGet(1) + get;
    tmrStop(1);
    sprintf(prt, "Load: %d", time);
    prtS(prt);

    prtS("Local file speed (1024 byte)");
    tmrStart(1, 60);
    get = tmrGet(1);
    ret = nvmSave(1, buf, 0, sizeof(buf));
    CHK;
    time = -tmrGet(1) + get;
    tmrStop(1);
    sprintf(prt, "Save: %d", time);
    prtS(prt);

    tmrStart(1, 60);
    get = tmrGet(1);
    ret = nvmLoad(1, buf, 0, sizeof(buf));
    CHK;
    time = -tmrGet(1) + get;
    tmrStop(1);
    sprintf(prt, "Load: %d", time);
    prtS(prt);
    goto lblEnd;
  lblKO:
    prtS("KO");
  lblEnd:
    prtS(" ");
    prtS(" ");
    prtS(" ");
    prtStop();
}
