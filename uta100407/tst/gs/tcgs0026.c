// tcgs0026
//
// author:      Gábor Sárospataki
// date:        01.05.2007
// description: test case to hash funtions
#include "pri.h"
#include <string.h>
#include <stdio.h>
#include <unicapt.h>
static code const char *srcFile = __FILE__;

#define CHK CHECK(ret>=0,lblKO)
enum eLen {
    lenVal = sizeof(card)
};
enum eDim {
    dimHashBlock = 256,
    dimVal = 100
};
static tRecord valRec;
static word valLen = lenVal;
static word valMap;
static tTable valTab;

static tHash hsh;
static byte hshMem[hshMemCalc(lenVal, dimHashBlock)];

static int Index(void) {
    int ret;
    word idx;
    byte num[4];
    card szam;
    char buf[prtW + 1];

#ifdef __POINTER_DEBUG__
    idx = hshMemCalc(lenVal, dimHashBlock);
    sprintf(buf, "Cash size:%d", idx);
    prtS(buf);
    sprintf(buf, "Cash point.:%x", hshMem);
    prtS(buf);
#endif
    ret = dspClear();
    CHK;
    ret = dspLS(0, " Rand. the table");
    for (idx = 0; idx < dimVal; idx++) {
        sprintf(buf, "Index: %d", idx);
        ret = dspLS(1, buf);
        CHK;
        szam = sysRand();
        memcpy(num, &szam, 4);
        ret = recMove(&valRec, &valTab, idx);
        CHK;
        ret = recPut(&valRec, 0, num, sizeof(card));
        CHK;
    }
    ret = dspLS(1, "      Done      ");
    CHK;
    tmrPause(1);
    ret = dspLS(2, " Reset the hash ");
    ret = hshReset(&hsh);
    CHK;
    ret = dspLS(3, "      Done      ");
    CHK;
    tmrPause(1);
    ret = dspLS(2, "  Fill the hash ");
    for (idx = 0; idx < dimVal; idx++) {
        sprintf(buf, "Index: %d", idx);
        ret = dspLS(3, buf);
        CHK;
        ret = recMove(&valRec, &valTab, idx);
        CHK;
        ret = recGet(&valRec, 0, num, sizeof(card));
        CHK;
#ifdef __NO_COLLAPSE__
        tmrPause(1);
#endif
        ret = hshAdd(&hsh, num, idx);
        CHK;
    }
    ret = dspLS(3, "      Done      ");
    CHK;
    tmrPause(1);
    return 1;
  lblKO:
    return -1;
}

static int Search(void) {
    int ret;
    char number[dspW + 1];
    card idx;
    card num;
    char prt[prtW + 1];

    dspClear();
    memset(number, 0, sizeof(number));
    ret = dspLS(0, "Value?");
    CHK;
    ret = enterStr(1, number, sizeof(number));
    CHK;
    CHECK(ret == kbdVAL, lblEnd);
    CHECK(*number, lblEnd);
    dec2num(&num, number, 0);
    CHECK(num <= 0xFFFFFFFF && num, lblWrong);
    ret = hshSearch(&hsh, (byte *) & num);
    CHK;
    if(ret != 0xFFFF) {
        idx = ret;
        ret = recMove(&valRec, &valTab, (word) idx);
        CHK;
        ret = recGet(&valRec, 0, &num, lenVal);
        CHK;
        ret = prtS("Found the item");
        CHK;
        sprintf(prt, "Idx:%d Val:%d", idx, num);
        ret = prtS(prt);
    } else {
        sprintf(prt, "Not found the %d value", num);
        ret = prtS(prt);
    }
    prtS(" ");
    prtS(" ");
    prtS(" ");
    goto lblEnd;
  lblKO:
    ret = -1;
    goto lblEnd;
  lblWrong:
    dspLS(2, "Wrong value!");
    Beep();
    tmrPause(1);
    ret = 1;
    goto lblEnd;
  lblEnd:
    return ret;
}

static int Print(void) {
    int ret;
    char number[5 + 1];
    card idx;
    card num;
    char prt[prtW + 1];

    dspClear();
    memset(number, 0, sizeof(number));
    ret = dspLS(0, "Index number?");
    CHK;
    ret = enterStr(1, number, sizeof(number));
    CHK;
    CHECK(ret == kbdVAL, lblEnd);
    CHECK(*number, lblEnd);
    dec2num(&idx, number, 0);
    CHECK(idx < dimVal, lblWrong);
    ret = recMove(&valRec, &valTab, (word) idx);
    CHK;
    ret = recGet(&valRec, 0, &num, lenVal);
    CHK;
    ret = prtS("Print an item:");
    CHK;
    if(num)
        sprintf(prt, "Idx:%d Val:%d", idx, num);
    else
        sprintf(prt, "Empty index:%d", idx);
    ret = prtS(prt);
    CHK;
    prtS(" ");
    prtS(" ");
    prtS(" ");
    goto lblEnd;
  lblKO:
    ret = -1;
    goto lblEnd;
  lblWrong:
    dspLS(2, "Too big index!");
    Beep();
    tmrPause(1);
    ret = 1;
    goto lblEnd;
  lblEnd:
    return ret;
}

static int Delete(void) {
    int ret;
    char number[5 + 1];
    card idx;
    card num;
    char prt[prtW + 1];

    dspClear();
    memset(number, 0, sizeof(number));
    ret = dspLS(0, "Index number?");
    CHK;
    ret = enterStr(1, number, sizeof(number));
    CHK;
    CHECK(ret == kbdVAL, lblEnd);
    CHECK(*number, lblEnd);
    dec2num(&idx, number, 0);
    CHECK(idx < dimVal, lblWrong);
    ret = recMove(&valRec, &valTab, (word) idx);
    CHK;
    ret = recGet(&valRec, 0, &num, lenVal);
    CHK;
    ret = hshDel(&hsh, (byte *) & num);
    CHK;
    num = 0;
    ret = recPut(&valRec, 0, &num, lenVal);
    CHK;
    sprintf(prt, "Deleted index:%d", idx);
    ret = prtS("Delete an item:");
    CHK;
    ret = prtS(prt);
    CHK;
    prtS(" ");
    prtS(" ");
    prtS(" ");
    goto lblEnd;
  lblKO:
    ret = -1;
    goto lblEnd;
  lblWrong:
    dspLS(2, "Too big index!");
    Beep();
    tmrPause(1);
    ret = 1;
    goto lblEnd;
  lblEnd:
    return ret;
}

void tcgs0026(void) {
    int ret;
    char *Menu[] = {
        "Hash menu",
        "Indexing",
        "Print an index",
        "Search",
        "Delete an index",
        0
    };
    card ofs;
    byte sec;
    int sta;

    dspStart();
    nvmStart();
    prtStart();

    sec = 0;
    ofs = 0;
    //Initialization
    ret = recInit(&valRec, sec, ofs, 1, &valLen, &valMap);
    CHK;
    ret = tabInit(&valTab, sec, ofs, (word) recSize(&valRec), dimVal);
    CHK;
    ofs += ret;
    ret = hshInit(&hsh, sec, ofs, hshMem, lenVal, dimHashBlock);
    CHK;
    sta = 0;
    while(1) {
        ret = mnuSelect((Pchar *) Menu, sta, 0);
        CHK;
        sta = ret;
        if(!ret)
            break;
        switch (ret % MNUMAX) {
          case 1:
              ret = Index();
              CHK;
              break;
          case 2:
              ret = Print();
              CHK;
              break;
          case 3:
              ret = Search();
              CHK;
              break;
          case 4:
              ret = Delete();
              CHK;
              break;
        };
    };
    goto lblEnd;
  lblKO:
    dspClear();
    dspLS(0, "KO!");
    trcErr(ret);
    tmrPause(3);
  lblEnd:
    dspStop();
    prtStop();
}
