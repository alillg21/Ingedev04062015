/// \file tcNS0035.c
/** Unitary UTA test case : BUS P A S S 
 * \sa
 */
#include "ctx.h"
#ifdef __MFC__
#include <string.h>
#include <stdio.h>
#include "sys.h"
#include "tst.h"

typedef const byte TKey[6];
typedef const byte TCondition[4];

//static TKey KF = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
//static TKey KR = { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11 };
//static TKey KP = { 0x22, 0x22, 0x22, 0x22, 0x22, 0x22 };
//static TKey KC = { 0x33, 0x33, 0x33, 0x33, 0x33, 0x33 };
//static TKey KB = { 0x44, 0x44, 0x44, 0x44, 0x44, 0x44 };

/*
static TCondition accessConditionList[] = {
    {0xFF, 0x07, 0x80, 0xFF},   // INITIT CONDITIONS 
    {0x78, 0x77, 0x88, 0xFF},   // Trailer cond = 011, Data = 100 COND_1
    {0x08, 0x77, 0x8F, 0xFF}    // Trailer cond = 011, Data = 110 COND_2        
};
*/

#define COND_1 1
#define COND_2 2
#define COND_INIT 0
#define BLOCK_TRAILER 3
#define BLK_SIZE 16

void tcns0036(void) {
    int ret;

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = mfcStart();
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    trcErr(ret);
    dspLS(1, "Test Fail");
    goto lblEnd;
  lblEnd:
    prtS("tcns0035 done");
    prtS("--------");
    mfcStop();
    dspStop();
    prtStop();

}
#else
void tcns0036(void) {
}
#endif
