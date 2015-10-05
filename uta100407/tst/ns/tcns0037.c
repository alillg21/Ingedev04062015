/// \file tcNS0037.c
/** Unitary UTA test case : Card repare
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

static TKey KF = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
//static TKey KR = { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11 };
static TKey KP = { 0x22, 0x22, 0x22, 0x22, 0x22, 0x22 };
static TKey KC = { 0x33, 0x33, 0x33, 0x33, 0x33, 0x33 };
static TKey KB = { 0x44, 0x44, 0x44, 0x44, 0x44, 0x44 };

static TCondition accessConditionList[] = {
    {0xFF, 0x07, 0x80, 0xFF},   // INITIT CONDITIONS 
    {0x78, 0x77, 0x88, 0xFF},   // Trailer cond = 011, Data = 100 COND_1
    {0x08, 0x77, 0x8F, 0xFF}    // Trailer cond = 011, Data = 110 COND_2        
};

#define COND_1 1
#define COND_2 2
#define COND_INIT 0
#define BLOCK_TRAILER 3
#define BLK_SIZE 16

static int persoWriteTrailer(TKey A, TKey B, byte sec, byte condType) {
    int ret = 0;
    byte SB = 0;
    byte buf[16];

    // not initialize the sector 0
    VERIFY(sec);

    trcS("persoWriteTrailer Beg\n");

    memset(buf, 0, sizeof(buf));

    // Trailer setting
    // build a buffer with keys and conditions
    // buf = newKeyA.conditions.newKeyB
    // conditions = FF0780FF - by default 

    memcpy(buf, A, sizeof(TKey));
    memcpy(buf + sizeof(TKey), accessConditionList[condType],
           sizeof(TCondition));
    memcpy(buf + sizeof(TKey) + sizeof(TCondition), B, sizeof(TKey));

    SB = (sec << 4) | 0x03;
    ret = mfcSave(buf, SB, 'D');
    trcFN("mfcSave SB = %x\n", SB);
    trcFN(", ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    trcS("\tTRAILER = [");
    trcBAN(buf, sizeof(buf));
    trcS("]\n");

    goto lblEnd;

  lblKO:
    trcS("persoWriteTrailer ERROR\n");
    return -1;
  lblEnd:
    trcS("persoWriteTrailer End\n");
    return 1;
}

static int persoCleanBlock(byte SB) {
    int ret = 0;
    byte buf[16];

    trcS("persoCleanBlock Beg\n");

    memset(buf, 0, sizeof(buf));

    ret = mfcSave(buf, SB, 'D');
    trcFN("mfcSave SB = %x\n", SB);
    trcFN(", ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    trcS("\tData = [");
    trcBAN(buf, sizeof(buf));
    trcS("]\n");

    goto lblEnd;

  lblKO:
    trcS("persoCleanBlock ERROR\n");
    return -1;
  lblEnd:
    trcS("persoCleanBlock End\n");
    return 1;
}

// ReInitialization of the card's sector
// input: actual key K for writing data and trailer, SecKeyAut - byte with sector number and the key type
// key type is given by the caller according to the actual condition matrix of the sector
// the key K (of type A or B according to the SecKeyAut) which has been given as the input parameters
// should be authorized for WRITING either datablocks or trailer
static int persoInitializeSector(TKey K, byte SecKeyAut) {
    int ret = 0;
    byte sec = 0;
    byte typ = 0;

    trcS("persoInitializeSector Beg\n");

    typ = (SecKeyAut & 0xF0);
    sec = (SecKeyAut & 0x0F);
    // not initialize the sector 0
    CHECK(sec > 0, lblKO);

    switch (typ) {
      case 0xA0:
      case 0xB0:
          ret = mfcAut(K, SecKeyAut);
          break;
      default:
          ret = -1;
          VERIFY(0);
          break;
    };
    trcFN("mfcAut %x\n", SecKeyAut);
    CHECK(ret >= 0, lblKO);

    // CLEAN the block 0, 1, 2 of the sector

    ret = persoCleanBlock((sec << 4) | 0x00);
    CHECK(ret >= 0, lblKO);

    ret = persoCleanBlock((sec << 4) | 0x01);
    CHECK(ret >= 0, lblKO);

    ret = persoCleanBlock((sec << 4) | 0x02);
    CHECK(ret >= 0, lblKO);

    // REINITIALIZATION of the trailer block

    persoWriteTrailer(KF, KF, sec, COND_INIT);

    goto lblEnd;

  lblKO:
    trcS("persoInitializeSector ERROR\n");
    return -1;
  lblEnd:
    trcS("persoInitializeSector End\n");
    return 1;
}

static int persoCardRepare(void) {
    int ret = 0;
    byte SECTOR = 0;

    trcS("persoCardRepare Beg\n");
    //1
    SECTOR = 1;
    ret = persoInitializeSector(KP, 0xB0 | SECTOR);
    CHECK(ret >= 0, lblKO);

    prtS("SEC1 UNDONE");

    //2
    SECTOR = 2;
    ret = persoInitializeSector(KP, 0xB0 | SECTOR);
    CHECK(ret >= 0, lblKO);

    prtS("SEC2 UNDONE");

    //3
    SECTOR = 3;
    ret = persoInitializeSector(KC, 0xB0 | SECTOR);
    CHECK(ret >= 0, lblKO);

    prtS("SEC3 UNDONE");

    //4
    SECTOR = 4;
    ret = persoInitializeSector(KC, 0xB0 | SECTOR);
    CHECK(ret >= 0, lblKO);

    prtS("SEC4 UNDONE");

    //5
    SECTOR = 5;
    ret = persoInitializeSector(KB, 0xB0 | SECTOR);
    CHECK(ret >= 0, lblKO);

    prtS("SEC5 UNDONE");

    //6
    SECTOR = 6;
    ret = persoInitializeSector(KB, 0xB0 | SECTOR);
    CHECK(ret >= 0, lblKO);

    prtS("SEC6 UNDONE");

    goto lblEnd;

  lblKO:
    trcS("persoCardRepare ERROR\n");
    return -1;
  lblEnd:
    trcS("persoCardRepare End\n");
    return 1;
}

void tcns0037(void) {
    int ret;

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    prtS("tc0037 REPARE\n");

    ret = mfcStart();
    trcFN("mfcStart ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    ret = mfcDetect(60);
    trcFN("mfcDetect ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    ret = persoCardRepare();
    trcFN("persoCardRepare ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    trcErr(ret);
    dspLS(1, "Test Failed");
    goto lblEnd;
  lblEnd:
    prtS("tcns0037 done");
    prtS("--------");
    mfcStop();
    dspStop();
    prtStop();

}
#else
void tcns0037(void) {
}
#endif
