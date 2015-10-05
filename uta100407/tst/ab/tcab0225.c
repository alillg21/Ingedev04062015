/** \file
 * Implementation of MenuLib functions
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0225.c $
 *
 * $Id: tcab0225.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include "pri.h"
#include "tst.h"
#include <string.h>

static char mnuBdy[1 + __MNUMAX__ + 1][dspW + 1];   //title+body+terminator
static word mnuDly = 0;
static byte mnuPos = 0;

static int utaMnuReset(card hmiHandle, card touchHandle) {
    //hmiHandle, touchHandle are ignored: no need for UTA
    trcS("utaMnuReset\n");
    memset(mnuBdy, 0, sizeof(mnuBdy));
    mnuPos = 1;
    return sizeof(mnuBdy);
}

static int utaMnuSetTitle(const char *title) {
    VERIFY(title);
    VERIFY(strlen(title) <= dspW);
    trcFS("utaMnuSetTitle title=%s\n", title);
    strcpy(mnuBdy[0], title);
    return strlen(title);
}

static int utaMnuSetTimeout(card timeout) {
    trcFN("utaMnuSetTimeout timeout=%d\n", timeout);
    mnuDly = timeout / 100;     //mnuDly is in seconds
    return mnuDly;
}

static int utaMnuAddItem(const char *caption, byte enabled, word value) {
    VERIFY(mnuPos < __MNUMAX__);
    VERIFY(caption);
    VERIFY(strlen(caption) <= dspW);
    VERIFY(value == mnuPos);    //why do it in another way?

    trcFS("utaMnuAddItem caption=%s\n", caption);
    if(!enabled)
        return 0;
    strcpy(mnuBdy[mnuPos++], caption);
    return strlen(caption);
}

static int utaMnuRun(word startingSelection) {
    char *ptr[MNUMAX];          //array of pointers to mnu items
    byte idx = 0;

    trcFN("utaMnuRun startingSelection=%d\n", startingSelection);
    memset(ptr, 0, sizeof(ptr));
    while(*mnuBdy[idx]) {
        ptr[idx] = mnuBdy[idx];
        trcFS("%s\n", ptr[idx]);
        idx++;
    }
    return mnuSelect((Pchar *) ptr, startingSelection, mnuDly);
}

static void makeMenu(const char *mnu[]) {
    int i;

    utaMnuReset(0, 0);
    utaMnuSetTitle(mnu[0]);
    utaMnuSetTimeout(60 * 100);
    i = 1;
    while(mnu[i]) {
        utaMnuAddItem(mnu[i], 1, i);
        i++;
    }
}

void tcab0225(void) {
    int ret;
    int i, j;
    const char *mnu[] = {
        "BANANA",
        "LALALA",
        "LULULU",
        "TITITI",
        "TATATA",
        "BYEBYE",
        0
    };
    const char *ptr;

    trcS("tcab0225 Beg\n");
    dspStart();
    i = 0;
    while(42) {
        makeMenu(mnu);
        ret = utaMnuRun(i);     //return (top,cur) where top is the number of top line displayed, cur is cursor position
        i = ret;
        j = ret % MNUMAX;       //
        ptr = mnu[j];
        dspClear();
        switch (j) {
          case 1:
          case 2:
          case 3:
          case 4:
              dspLS(1, ptr);
              break;
          default:
              dspLS(1, ptr);
              j = 0;            //quit the loop
              break;
        }
        tmrPause(3);
        if(!j)
            break;
    }

    goto lblEnd;
    //lblKO:
    ;
  lblEnd:
    dspStop();
    trcS("tcab0225 End\n");
}
