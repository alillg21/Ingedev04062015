#include <string.h>
#include "sys.h"
#include "tst.h"

//Unitary test case tcnl0022.
//For internal SRam
//ismLoad
//ismSet
//ismSave
//Functions testing:

void tcnl0022(void) {
    int ret;
    char chr[300 + 1];
    word ofs;
    struct {
        char r[100];
        char d[100];
        char t[100];
    } dat;

    trcS("tcnl0022 Beg\n");

    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    memcpy(dat.r,
           "B965C60459EFC3A69FC0276F6A86B1C9EA31C476C4746ECD6D7C873FAF4556DE55C32B6668EEDBDA45F3CD3844565E76754C",
           100);
    memcpy(dat.d,
           "3E1CE6A55205DE058AEDA42EABEDF49B7978C0B64A0B71D65A9F86D6136468A11D666A32B57E9A918E49B9572F57EDB54D08",
           100);
    memcpy(dat.t,
           "9C701663574F1CE41C95F144D60F0EA6A661700FD9BE8CD38746B04D42EABEDF49B7978C0B64A0B71D65A9F86D6136468A11",
           100);

    ofs = 0;

    ret = ismSet('X', 0, 512);  //set 512 byte of sram to 'X'
    CHECK(ret == 512, lblKO);

    ret = ismSave(&dat, ofs, sizeof(dat));  //save the structure into sram
    CHECK(ret == sizeof(dat), lblKO);
    ofs += ret;

    ret = ismLoad(chr, 0, 300); //load the structure from sram to bin
    CHECK(ret == ofs, lblKO);

    prtS(chr);
    prtS("sram processing OK");

    goto lblEnd;

  lblKO:
    trcErr(ret);
    prtS("KO!");
  lblEnd:
    prtStop();
    tmrPause(3);
    trcS("tcnl0022 End\n");
}
