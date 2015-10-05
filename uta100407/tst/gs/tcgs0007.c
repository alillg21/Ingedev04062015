// tcgs0007
//
// author:      Gábor Sárospataki
// date:        31.01.2006
// description: test case to fmtIns,fmtDel functions

#include <string.h>
#include "pri.h"

#define CHK CHECK(ret>=0,lblKO)

int tcgs0007(void) {
    int ret;

    //char *ins="abcdefghij";
    //char *bgd="0123456789";
    char dst[prtW + 1];

    trcS("Tcgs0007 Begin:\n");

    ret = prtStart();
    CHK;

    //standard use (insert to the fifth character' place)
    memset(dst, 0, sizeof(dst));
    ret = prtS("The fmtIns function:");
    CHK;
    ret = fmtIns(dst, "abcdef", "ABCDEF", 3, 0);
    CHK;
    CHECK(strcmp(dst, "abcABCDEFdef") == 0, lblKO);
    ret = prtS(dst);
    CHK;

    //if you set a maximal length
    memset(dst, 0, sizeof(dst));
    ret = fmtIns(dst, "abcdefg", "ABCDEFG", 3, 6);
    CHK;
    CHECK(strcmp(dst, "abcABC") == 0, lblKO);
    ret = prtS(dst);
    CHK;

    ret = prtS(" ");
    CHK;
    ret = prtS("The fmtDel function:");
    CHK;

    //cut a two long part from fourth place
    memset(dst, 0, sizeof(dst));
    ret = fmtDel(dst, "StrXXXXing", 3, 4);
    CHECK(strcmp(dst, "String") == 0, lblKO);
    ret = prtS(dst);
    CHK;

    //whith big len (can handle it)
    memset(dst, 0, sizeof(dst));
    ret = fmtDel(dst, "StrXXXXing", 3, 2000);
    CHECK(strcmp(dst, "Str") == 0, lblKO);
    ret = prtS(dst);
    CHK;

    ret = 1;
    goto lblEnd;

  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    prtStop();
    return ret;
}
