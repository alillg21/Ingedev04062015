/** \file
 * Unitary test case tcns0020.
 * Contactless TPASS: UTA VERSION
 * Load TPass library, detect, activate the MIFARE card, read/print the BLOCK0 
 * Start driver, authorization sector0, reading block 0, authorization to sec2, rewrite keys and conditions, authorisation with new key, read trailer ...
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

static TKey ReadKeyList[] = {
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    {0x11, 0x11, 0x11, 0x11, 0x11, 0x11},
    {0x22, 0x22, 0x22, 0x22, 0x22, 0x22},
    {0x33, 0x33, 0x33, 0x33, 0x33, 0x33},
    {0x44, 0x44, 0x44, 0x44, 0x44, 0x44},
    {0x55, 0x55, 0x55, 0x55, 0x55, 0x55},
    {0x66, 0x66, 0x66, 0x66, 0x66, 0x66}
};

static TKey WriteKeyList[] = {
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA},
    {0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB},
    {0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC},
    {0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD},
    {0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};

static TCondition accessConditionList[] = {
    {0xFF, 0x07, 0x80, 0xFF},   // INITIT CONDITIONS 
    {0x78, 0x77, 0x88, 0xFF},   // Trailer cond = 011, Data = 100 COND_1
    {0x08, 0x77, 0x8F, 0xFF}    // Trailer cond = 011, Data = 110 COND_2        
};

#define COND_1 1
#define COND_2 2
#define BLOCK_TRAILER 3

void tcns0020(void) {

    int ret;
    byte buf[16];
    char print[24];

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    prtS("tc0020 end\n");

    prtS("mfcStart");
    ret = mfcStart();
    trcFN("mfcStart ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    prtS("Detect card");
    ret = mfcDetect(60);
    trcFN("mfcDetect ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    prtS("Auth sector0 (def key)");
    ret = mfcAut(0, 0xA0);
    trcFN("mfcAut ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    prtS("Trailer data:");
    memset(buf, 0, sizeof(buf));
    ret = mfcLoad(buf, 0x03, 'D');
    trcFN("mfcLoad ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    memset(print, 0, sizeof(print));
    sprintf(print, "%02X %02X %02X %02X %02X %02X %02X %02X", buf[0], buf[1],
            buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
    prtS(print);

    memset(print, 0, sizeof(print));
    sprintf(print, "%02X %02X %02X %02X %02X %02X %02X %02X", buf[8], buf[9],
            buf[10], buf[11], buf[12], buf[13], buf[14], buf[15]);
    prtS(print);

    prtS("Auth sector7 (def key A)");
    ret = mfcAut(0, 0xA7);
    trcFN("mfcAut ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    prtS("Trailer data:");
    memset(buf, 0, sizeof(buf));
    ret = mfcLoad(buf, 0x73, 'D');
    trcFN("mfcLoad ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    memset(print, 0, sizeof(print));
    sprintf(print, "%02X %02X %02X %02X %02X %02X %02X %02X", buf[0], buf[1],
            buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
    prtS(print);

    memset(print, 0, sizeof(print));
    sprintf(print, "%02X %02X %02X %02X %02X %02X %02X %02X", buf[8], buf[9],
            buf[10], buf[11], buf[12], buf[13], buf[14], buf[15]);
    prtS(print);

    // build a buffer with keys and conditions
    // buf = newKeyA.conditions.newKeyB
    // conditions = 787788FF : 
    //      with key A: read trailer, read data
    //      with key B: read/write trailer, write key A, write key B, read/write data
    //      operations increment/decrement are forbidden for both keys

    memset(buf, 0, sizeof(buf));

    memcpy(buf, ReadKeyList[1], sizeof(ReadKeyList[1]));
    memcpy(buf + sizeof(ReadKeyList[1]), accessConditionList[COND_1],
           sizeof(accessConditionList[COND_1]));
    memcpy(buf + sizeof(ReadKeyList[1]) + sizeof(accessConditionList[COND_1]),
           WriteKeyList[1], sizeof(WriteKeyList[1]));

    prtS("Write trailer:");
    ret = mfcSave(buf, 0x73, 'D');
    trcFN("mfcLoad ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    prtS("Auth sector2 (new key A)");
    ret = mfcAut(ReadKeyList[1], 0xA7);
    trcFN("mfcAut ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    prtS("Read trailer:");
    memset(buf, 0, sizeof(buf));
    ret = mfcLoad(buf, 0x73, 'D');
    trcFN("mfcLoad ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    memset(print, 0, sizeof(print));
    sprintf(print, "%02X %02X %02X %02X %02X %02X %02X %02X", buf[0], buf[1],
            buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
    prtS(print);

    memset(print, 0, sizeof(print));
    sprintf(print, "%02X %02X %02X %02X %02X %02X %02X %02X", buf[8], buf[9],
            buf[10], buf[11], buf[12], buf[13], buf[14], buf[15]);
    prtS(print);

    prtS("Auth sector7 (new key B)");
    ret = mfcAut(WriteKeyList[1], 0xB7);
    trcFN("mfcAut ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    // build a buffer with keys and conditions
    // buf = newKeyA.conditions.newKeyB
    // conditions = FF0780FF - by default 

    memset(buf, 0, sizeof(buf));

    memcpy(buf, ReadKeyList[0], sizeof(ReadKeyList[0]));
    memcpy(buf + sizeof(ReadKeyList[0]), accessConditionList[0],
           sizeof(accessConditionList[0]));
    memcpy(buf + sizeof(ReadKeyList[0]) + sizeof(accessConditionList[0]),
           WriteKeyList[0], sizeof(WriteKeyList[0]));

    prtS("Write trailer:");
    ret = mfcSave(buf, 0x73, 'D');
    trcFN("mfcLoad ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    prtS("Auth sector2 (key A default)");
    ret = mfcAut(0, 0xA7);
    trcFN("mfcAut ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    prtS("Read trailer:");
    memset(buf, 0, sizeof(buf));
    ret = mfcLoad(buf, 0x73, 'D');
    trcFN("mfcLoad ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    memset(print, 0, sizeof(print));
    sprintf(print, "%02X %02X %02X %02X %02X %02X %02X %02X", buf[0], buf[1],
            buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
    prtS(print);

    memset(print, 0, sizeof(print));
    sprintf(print, "%02X %02X %02X %02X %02X %02X %02X %02X", buf[8], buf[9],
            buf[10], buf[11], buf[12], buf[13], buf[14], buf[15]);
    prtS(print);

    goto lblEnd;

  lblKO:
    prtS("tc0020 KO\n");

  lblEnd:
    prtS("--------");
    prtS("tc0020 end\n");
    mfcStop();
    prtStop();
    dspStop();

}
#else
void tcns0020(void) {
}
#endif
