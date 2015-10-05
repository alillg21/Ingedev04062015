/// \file tcNS0035.c
/** Unitary UTA test case : MIFARE CARD READING
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
static TKey KR = { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11 };
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

#define lenAmt 10
#define lenBcdAmt 2
#define lenPan 16
#define lenBcdPan 8
#define lenYYMM 4
#define lenBcdYYMM 2
#define lenName 48
#define lenYYMMDD 6
#define lenBcdYYMMDD 3
#define lenAmt 10
#define lenStan 8
#define lenPmtStan 4
#define lenBcdPmtStan 2
#define lenBcdStan 4
#define lenYYMMDDhhmm 10
#define lenBcdYYMMDDhhmm 5
#define lenTid 8
#define lenBcdTid 4
#define lenBatNum 3
#define lenSeqNum 3

static char traPan[lenPan + 1] = "";
static char traExpDat[lenYYMM + 1] = "";
static byte traUsage = '0';
static byte traSecScheme = '0';
static char traName[lenName + 1] = "";
static char traRechargeDate[lenYYMMDD + 1] = "";
static char traRechargeAmt[lenAmt + 1] = "";
static card traAmt = 0;
static char traRechargePostId[lenTid + 1] = "";
static char traRechargeStan[lenStan + 1] = "";
static char traRechargeValDate[lenYYMMDD + 1] = "";
static card traCreditAmt = 0;
static card traDebitAmt = 0;
static card traCreditStan = 0;
static card traDebitStan = 0;
static char traPassDateTime[lenYYMMDDhhmm + 1] = "";
static byte traPassNum = 0;
static byte traBatchNum = 0;
static card traPmtStan = 0;
static char traBusId[lenTid + 1] = "";

static int persoGetSectorData(TKey K, byte SecKeyAut, char *buffer) {
    int ret = 0;
    byte sec = 0;
    byte typ = 0;

    trcS("persoGetSectorData Beg\n");

    memset(buffer, 0, sizeof(buffer));

    typ = (SecKeyAut & 0xF0);
    sec = (SecKeyAut & 0x0F);

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

    ret = mfcLoad(buffer + 0 * BLK_SIZE, (sec << 4) | 0, 'D');
    trcFN("mfcLoad ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    ret = mfcLoad(buffer + 1 * BLK_SIZE, (sec << 4) | 1, 'D');
    trcFN("mfcLoad ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    ret = mfcLoad(buffer + 2 * BLK_SIZE, (sec << 4) | 2, 'D');
    trcFN("mfcLoad ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    ret = mfcLoad(buffer + 3 * BLK_SIZE, (sec << 4) | 3, 'D');
    trcFN("mfcLoad ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    trcS("SECTOR DATA = {");
    trcBAN(buffer, sizeof(buffer));
    trcS("}\n");

    goto lblEnd;

  lblKO:
    trcS("persoGetSectorData ERROR\n");
    return -1;
  lblEnd:
    trcS("persoGetSectorData End\n");
    return 1;
}

static int persoGetCardIdentification(void) {
    int ret = 0;
    byte secbuf[4 * BLK_SIZE] = "";
    char pan[lenPan + 1] = "";
    char expDat[lenYYMM + 1] = "";
    byte usage = 0;
    byte secScheme = 0;

    trcS("persoGetCardIdentification Beg\n");

    //SECTOR 1 // key A = KF = READ KEY
    ret = persoGetSectorData(KF, 0xA1, secbuf);
    CHECK(ret >= 0, lblKO);

    /*
       SECTOR 1: CARD IDENTIFICATION

       This sector can be written only by PERSO application.
       The 48 bytes of the sector contain the following information:

       * bytes 0-7 (PAN): 16-digit logical card number. Each card has its logical number that is unique within the system.
       * bytes 8-9 (EXPIRY DATE): card expiry date in YYMM format. After achieving this date the card should be repersonalized.
       * byte 10 (CARD USAGE): 'p' for pass, 'a' for abonement, 'd' for driver, 'm' for manager
       * byte 11 (SECURITY SCHEME): the key set identifier
     */

    bin2hex(pan, secbuf, lenBcdPan);
    strcpy(traPan, pan);        //future MAPPUTSTR
    trcS("PAN = ");
    trcBAN(pan, lenPan);
    trcS("\n");

    bin2hex(expDat, secbuf + lenBcdPan, lenBcdYYMM);
    strcpy(traExpDat, expDat);  //future MAPPUTSTR
    trcS("EXP DATE = ");
    trcBAN(expDat, lenYYMM);
    trcS("\n");

    usage = secbuf[lenBcdPan + lenBcdYYMM];
    memcpy(&traUsage, &usage, 1);   //future MAPPUTBYTE
    trcS("USAGE = ");
    trcBAN(&usage, 1);
    trcS("\n");

    secScheme = secbuf[lenBcdPan + lenBcdYYMM + 1];
    memcpy(&traSecScheme, &secScheme, 1);   //future MAPPUTBYTE
    trcS("SCHEME = ");
    trcBAN(&secScheme, 1);
    trcS("\n");

    goto lblEnd;

  lblKO:
    trcS("persoGetCardIdentification ERROR\n");
    return -1;
  lblEnd:
    trcS("persoGetCardIdentification End\n");
    return 1;
}

static int persoGetCardHolderIdentification(void) {
    int ret = 0;
    byte secbuf[4 * BLK_SIZE] = "";

    trcS("persoGetCardHolderIdentification Beg\n");

    //SECTOR 2 // key A = KR = READ KEY
    ret = persoGetSectorData(KR, 0xA2, secbuf);
    CHECK(ret >= 0, lblKO);

    /*
       SECTOR 2: CARDHOLDER NAME
       This sector can be written only by PERSO application.
       The 48 bytes of the sector contain the name of the cardholder.
       This information is printed on the card and on RECHARGE and PERSO receipts.

       Example:
       0    1   2   3   4   5   6   7   8   9   10  11  12  13  14  15
       0  'M' 'R' ' ' 'C' 'A' 'R' 'D' 'H' 'O' 'L'   'D' 'E' 'R' ''' 's' ' '
       1  'L' 'A' 'S' 'T' 'N' 'A' 'M' 'E' ' ' 'A'   'N' 'D' ' ' 'H' 'I' 'S'
       2  ' ' 'F' 'I' 'R' 'S' 'T' 'N' 'A' 'M' 'E'   ' ' ' ' ' ' ' ' ' ' ' ' 
     */

    strcpy(traName, secbuf);    //future MAPPUTSTR
    trcS("NAME = ");
    trcBAN(secbuf, lenName);
    trcS("\n");

    goto lblEnd;

  lblKO:
    trcS("persoGetCardHolderIdentification ERROR\n");
    return -1;
  lblEnd:
    trcS("persoGetCardHolderIdentification End\n");
    return 1;
}
static int persoGetLastRechargeInfo(void) {
    int ret = 0;
    byte secbuf[4 * BLK_SIZE] = "";
    char amt[lenAmt + 1] = "";
    char rcgDate[lenYYMMDD + 1] = "";
    char valDate[lenYYMMDD + 1] = "";
    char post[lenTid + 1] = "";
    char stan[lenStan + 1] = "";
    card amtnum = 0;

    trcS("persoGetLastRechargeInfo Beg\n");

    //SECTOR 3 // key A = KR = READ KEY
    ret = persoGetSectorData(KR, 0xA3, secbuf);
    CHECK(ret >= 0, lblKO);
    /*  
       SECTOR 3: LAST RECHARGE INFO

       The sector contains the data of the last recharge.
       Usually it is written by RECHARGE application, but it is available for writing for PERSO application also.
       It contains the following data:

       * bytes 0-2 (RECHARGE DATE): the date of last recharge in YYMMDD format
       * bytes 3-4 (RECHARGE AMOUNT): the amount of last recharge
       * bytes 5-8 (RECHARGE POST): the terminal ID that performed the recharge
       * bytes 9-12 (SYSTEM TRACE AUDIT NUMBER=STAN): recharge operation sequential number assigned by the Front Office
       * bytes 16-18 (RECHARGE VALIDITY DATE): the date after which the card will be desactivated until a recharge operation
     */

    bin2hex(rcgDate, secbuf, lenBcdYYMMDD);
    strcpy(traRechargeDate, rcgDate);   //future MAPPUTSTR
    trcS("LAST RECHARGE DATE = ");
    trcBAN(rcgDate, lenYYMMDD);
    trcS("\n");

    bin2num(&amtnum, secbuf + lenBcdYYMMDD, lenBcdAmt);
    num2dec(amt, amtnum, lenAmt);
    strcpy(traRechargeAmt, amt);    //future MAPPUTSTR
    traAmt = amtnum;            //future MAPPUTCARD
    trcS("LAST RECHARGE AMOUNT = ");
    trcBAN(amt, lenAmt);
    trcS("\n");

    bin2hex(post, secbuf + lenBcdYYMMDD + lenBcdAmt, lenBcdTid);
    strcpy(traRechargePostId, post);    //future MAPPUTSTR
    trcS("RECHARGE POST ID = ");
    trcBAN(post, lenTid);
    trcS("\n");

    bin2hex(stan, secbuf + lenBcdYYMMDD + lenBcdAmt + lenBcdTid, lenBcdStan);
    strcpy(traRechargeStan, stan);  //future MAPPUTSTR
    trcS("RECHARGE STAN = ");
    trcBAN(stan, lenStan);
    trcS("\n");

    bin2hex(valDate, secbuf + lenBcdYYMMDD + lenBcdAmt + lenBcdTid + lenBcdStan,
            lenBcdYYMMDD);
    strcpy(traRechargeValDate, valDate);    //future MAPPUTSTR
    trcS("RECHARGE VALIDITY DATE = ");
    trcBAN(valDate, lenYYMMDD);
    trcS("\n");

    goto lblEnd;

  lblKO:
    trcS("persoGetLastRechargeInfo ERROR\n");
    return -1;
  lblEnd:
    trcS("persoGetLastRechargeInfo End\n");
    return 1;
}

static int persoGetCreditAmount(void) {
    int ret = 0;
    byte val[BLK_SIZE];
    card credAmt = 0;
    card credStan = 0;

    trcS("persoGetCreditAmount Beg\n");

    /*
       SECTOR 4: CREDIT AMOUNT
       Only RECHARGE and PERSO applications have a key that allows to write to this sector.

       The first block of the sector contains CREDIT AMOUNT.
       It is incremented by RECHARGE application during RECHARGE operation.

       The second block contains the CREDIT TRACE NUMER, i. e. the number of recharge operations performed with the card since its personalization.
       It is incremanted by one each time when a RECHARGE operation is performed.
     */

    ret = mfcAut(KR, 0xA4);     //SECTOR 4 // key A = KR = READ KEY
    trcFN("mfcAut ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    memset(val, 0, sizeof(val));

    ret = mfcLoad(val, 0x40, 'V');
    trcFN("mfcLoad ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    bin2num(&credAmt, val, sizeof(card));

    trcS("SECTOR 4: CREDIT AMOUNT\n");
    trcFN("AMOUNT = %d\n", credAmt);

    memset(val, 0, sizeof(val));

    ret = mfcLoad(val, 0x41, 'V');
    trcFN("mfcLoad ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    bin2num(&credStan, val, sizeof(card));

    trcFN("TRACE NUMBER = %d\n", credStan);

    traCreditAmt = credAmt;     // future MAPPUTCARD
    traCreditStan = credStan;   // future MAPPUTCARD

    goto lblEnd;

  lblKO:
    trcS("persoGetCreditAmount ERROR\n");
    return -1;
  lblEnd:
    trcS("persoGetCreditAmount End\n");
    return 1;
}

static int persoGetLastPaymentInfo(void) {
    int ret = 0;
    byte secbuf[4 * BLK_SIZE] = "";
    char pmtDateTime[lenYYMMDDhhmm + 1] = "";
    char busid[lenTid + 1] = "";
    card stan = 0;
    byte passNum = 0;
    byte batch = 0;

    trcS("persoGetLastPaymentInfo Beg\n");

    //SECTOR 5 // key A = KR = READ KEY
    ret = persoGetSectorData(KR, 0xA5, secbuf);
    CHECK(ret >= 0, lblKO);

    /*
       SECTOR 5: LAST PAYMENT INFO

       The sector contains the data of the lastpayment.
       It is modified at each PAYMENT operation by the BUS application.
       During RECHARGE operation it is reset to zero.
       It contains the following data:

       * bytes 0-4 (PASS TIMESTAMP): the timestamp of last payment in YYMMDDhhmm format
       * byte 5 (PASS NUMBER): number of passes
       * bytes 6-9 (TERMINAL ID): the terminal ID that performed the payment
       * byte 10: (BATCH NUMBER)
       * bytes 11-12 (PAYMENT SEQUENTIAL NUMBER): payment operation sequential number, unique per terminal per day
     */

    bin2hex(pmtDateTime, secbuf, lenBcdYYMMDDhhmm);
    strcpy(traPassDateTime, pmtDateTime);   //future MAPPUTSTR
    trcS("LAST PAYMENT DATETIME = ");
    trcBAN(pmtDateTime, lenYYMMDDhhmm);
    trcS("\n");

    passNum = secbuf[lenBcdYYMMDDhhmm];
    memcpy(&traPassNum, &passNum, 1);   //future MAPPUTBYTE
    trcS("NUMBER OF PASSES = ");
    trcBAN(&passNum, 1);
    trcS("\n");

    bin2hex(busid, secbuf + lenBcdYYMMDDhhmm + 1, lenBcdTid);
    strcpy(traBusId, busid);    //future MAPPUTSTR
    trcS("BUS ID = ");
    trcBAN(busid, lenTid);
    trcS("\n");

    batch = secbuf[lenBcdYYMMDDhhmm + 1 + lenBcdTid];
    memcpy(&traBatchNum, &batch, 1);    //future MAPPUTBYTE
    trcS("BATCH NUM = ");
    trcBAN(&batch, 1);
    trcS("\n");

    bin2num(&stan, secbuf + lenBcdYYMMDDhhmm + 1 + lenBcdTid + 1,
            lenBcdPmtStan);
    traPmtStan = stan;          //future MAPPUTSTR
    trcFN("RECHARGE STAN = %d \n", stan);

    goto lblEnd;

  lblKO:
    trcS("persoGetLastPaymentInfo ERROR\n");
    return -1;
  lblEnd:
    trcS("persoGetLastPaymentInfo End\n");
    return 1;
}

static int persoGetGreditAmount(void) {
    int ret = 0;
    byte val[BLK_SIZE];
    card dbtAmt = 0;
    card dbtStan = 0;

    trcS("persoGetGreditAmount Beg\n");

    /*
       SECTOR 6: DEBIT AMOUNT

       BUS application has a key that allows to write to this sector.

       The first block of the sector contains DEBIT AMOUNT.
       It is incremented by BUS application during PAYMENT operation.

       The second block contains the DEBIT TRACE NUMER, i. e. the number of payment operations performed with the card since its personalization.
       It is incremanted by one each time when a PAYMENT operation is performed.
     */

    ret = mfcAut(KR, 0xA6);     //SECTOR 6 // key A = KR = READ KEY
    trcFN("mfcAut ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    memset(val, 0, sizeof(val));

    ret = mfcLoad(val, 0x60, 'V');
    trcFN("mfcLoad ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    bin2num(&dbtAmt, val, sizeof(card));

    trcS("SECTOR 4: CREDIT AMOUNT\n");
    trcFN("AMOUNT = %d\n", dbtAmt);

    memset(val, 0, sizeof(val));

    ret = mfcLoad(val, 0x61, 'V');
    trcFN("mfcLoad ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    bin2num(&dbtStan, val, sizeof(card));

    trcFN("TRACE NUMBER = %d\n", dbtStan);

    traDebitAmt = dbtAmt;       // future MAPPUTCARD
    traDebitStan = dbtStan;     // future MAPPUTCARD

    goto lblEnd;

  lblKO:
    trcS("persoGetGreditAmount ERROR\n");
    return -1;
  lblEnd:
    trcS("persoGetGreditAmount End\n");
    return 1;
}

static int persoGetCardInfo(void) {
    int ret = 0;

    trcS("persoGetCardInfo Beg\n");

    ret = persoGetCardIdentification();
    CHECK(ret >= 0, lblKO);

    ret = persoGetCardHolderIdentification();
    CHECK(ret >= 0, lblKO);

    ret = persoGetLastRechargeInfo();
    CHECK(ret >= 0, lblKO);

    ret = persoGetCreditAmount();
    CHECK(ret >= 0, lblKO);

    ret = persoGetLastPaymentInfo();
    CHECK(ret >= 0, lblKO);

    ret = persoGetGreditAmount();
    CHECK(ret >= 0, lblKO);

    goto lblEnd;

  lblKO:
    trcS("persoGetCardInfo ERROR\n");
    return -1;
  lblEnd:
    trcS("persoGetCardInfo End\n");
    return 1;
}

void tcns0035(void) {
    int ret;

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = mfcStart();
    CHECK(ret >= 0, lblKO);

    ret = mfcDetect(60);
    trcFN("mfcDetect ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    prtS("tcns0035 GET INFO");

    ret = persoGetCardInfo();

    trcFN("persoGetCardInfo ret = %d\n", ret);
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
void tcns0035(void) {
}
#endif
