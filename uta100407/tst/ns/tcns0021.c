/** \file
 * Unitary test case tcns0020.
 * Contactless TPASS: UTA VERSION
 * Load TPass library, detect, activate the MIFARE card, read/print the BLOCK0 
 * PERSO
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

static char traPan[lenPan + 1] = "1234567890123456";
static char traExpDat[lenYYMM + 1] = "0801";
static byte traUsage = 'p';
static byte traSecScheme = '0';
static char traName[lenName + 1] = "MR CARDHOLDER LASTNAME AND HIS FIRSTNAME";
static char traRechargeDate[lenYYMMDD + 1] = "080102";
static card traAmt = 2345;
static char traRechargePostId[lenTid + 1] = "12345678";
static char traRechargeStan[lenStan + 1] = "17895697";
static char traRechargeValDate[lenYYMMDD + 1] = "080202";
static card traCreditAmt = 55555;   //D903
static card traDebitAmt = 44444;    //AD9C
static card traCreditStan = 222;    //DE
static card traDebitStan = 444; //1BC
static char traPassDateTime[lenYYMMDDhhmm + 1] = "0801222211";
static byte traPassNum = 4;
static byte traBatchNum = 24;
static card traPmtStan = 34567; //8707
static char traBusId[lenTid + 1] = "12345678";

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

static int persoCardIdentification(TKey A, TKey B, byte sec, byte condType) {
    int ret;
    byte bcdPan[lenBcdPan];
    byte bcdExpDate[lenBcdYYMM];
    byte buf[16];
    char pan[lenPan + 1] = "";
    char expDate[lenYYMM + 1] = "";
    byte usage = 0;
    byte scheme = 0;

    /*
       SECTOR 1: CARD IDENTIFICATION

       This sector can be written only by PERSO application.
       The 48 bytes of the sector contain the following information:

       * bytes 0-7 (PAN): 16-digit logical card number. Each card has its logical number that is unique within the system.
       * bytes 8-9 (EXPIRY DATE): card expiry date in YYMM format. After achieving this date the card should be repersonalized.
       * byte 10 (CARD USAGE): 'p' for pass, 'a' for abonement, 'd' for driver, 'm' for manager
       * byte 11 (SECURITY SCHEME): the key set identifier
     */
    trcS("persoCardIdentification Beg\n");

    ret = mfcAut(0, 0xA0 | sec);
    trcFN("mfcAut ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    memset(buf, 0, sizeof(buf));

    // PAN
    memcpy(pan, traPan, lenPan);    // future MAPGET

    hex2bin(bcdPan, pan, lenPan);
    memcpy(buf, bcdPan, lenBcdPan);
    trcS("PAN = ");
    trcBAN(bcdPan, lenBcdPan);
    trcS("\n");

    // EXP DATE
    memcpy(expDate, traExpDat, lenYYMM);    // future MAPGET

    hex2bin(bcdExpDate, expDate, lenYYMM);
    memcpy(buf + lenBcdPan, bcdExpDate, lenBcdYYMM);
    trcS("EXP DATE = ");
    trcBAN(bcdExpDate, lenBcdYYMM);
    trcS("\n");

    //USAGE
    memcpy(&usage, &traUsage, 1);   // future MAPGET

    memcpy(buf + lenBcdPan + lenBcdYYMM, &usage, 1);
    trcS("USAGE = ");
    trcBAN(&usage, 1);
    trcS("\n");

    //SEC SCHEME
    memcpy(&scheme, &traSecScheme, 1);  // future MAPGET

    memcpy(buf + lenBcdPan + lenBcdYYMM + 1, &scheme, 1);
    trcS("SCHEME = ");
    trcBAN(&scheme, 1);
    trcS("\n");

    // Write Data to sector 1 block 0
    ret = mfcSave(buf, (sec << 4) | 0x00, 'D');
    trcFN("mfcSave ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    trcS("\tSECTOR 1: CARD IDENTIFICATION\n");
    trcS("\tBLOCK = ");
    trcBAN(buf, sizeof(buf));
    trcS("\n");

    // Trailer setting

    // build a buffer with keys and conditions
    // buf = newKeyA.conditions.newKeyB
    // conditions = 787788FF : 
    //      with key A: read trailer, read data
    //      with key B: read/write trailer, write key A, write key B, read/write data
    //      operations increment/decrement are forbidden for both keys

    persoWriteTrailer(A, B, sec, condType);

    goto lblEnd;

  lblKO:
    trcS("persoCardIdentification ERROR\n");
    return -1;
  lblEnd:
    trcS("persoCardIdentification End\n");
    return 1;
}

static int persoCardHolderIdentification(TKey A, TKey B, byte sec,
                                         byte condType) {
    int ret;
    byte buf[16];
    byte name[lenName + 1];

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

    trcS("persoCardHolderIdentification Beg\n");

    ret = mfcAut(0, 0xA0 | sec);
    trcFN("mfcAut ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    memcpy(name, traName, lenName); // future MAPGET  

    // content of the block 0 of the sector
    memset(buf, 0, sizeof(buf));
    memcpy(buf, name, BLK_SIZE);

    // Write Data to block 0 of the sector
    ret = mfcSave(buf, (sec << 4) | 0x00, 'D');
    trcFN("mfcSave ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    trcS("Name BLOCK0 = ");
    trcBAN(buf, BLK_SIZE);
    trcS("\n");

    // content of the block 1 of the sector
    memset(buf, 0, sizeof(buf));
    memcpy(buf, name + BLK_SIZE, BLK_SIZE);

    // Write Data to block 0 of the sector
    ret = mfcSave(buf, (sec << 4) | 0x01, 'D');
    trcFN("mfcSave ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    trcS("Name BLOCK1 = ");
    trcBAN(buf, BLK_SIZE);
    trcS("\n");

    // content of the block 2 of the sector
    memset(buf, 0, sizeof(buf));
    memcpy(buf, name + 2 * BLK_SIZE, BLK_SIZE);

    // Write Data to block 0 of the sector
    ret = mfcSave(buf, (sec << 4) | 0x02, 'D');
    trcFN("mfcSave ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    trcS("Name BLOCK2 = ");
    trcBAN(buf, BLK_SIZE);
    trcS("\n");

    trcS("SECTOR 2: CARDHOLDER NAME\n");
    trcS("SECTOR = ");
    trcBAN(name, lenName);
    trcS("\n");

    // Trailer setting

    // build a buffer with keys and conditions
    // buf = newKeyA.conditions.newKeyB
    // conditions = 787788FF : 
    //      with key A: read trailer, read data
    //      with key B: read/write trailer, write key A, write key B, read/write data
    //      operations increment/decrement are forbidden for both keys

    persoWriteTrailer(A, B, sec, condType);

    goto lblEnd;

  lblKO:
    trcS("persoCardHolderIdentification ERROR\n");
    return -1;
  lblEnd:
    trcS("persoCardHolderIdentification End\n");
    return 1;
}

static int persoLastRechargeInfo(TKey A, TKey B, byte sec, byte condType) {
    int ret;
    byte buf[16];
    byte bcdRechargeDate[lenBcdYYMMDD]; //bytes 0-2 (RECHARGE DATE): the date of last recharge in YYMMDD format
    byte bcdRechargeAmt[lenBcdAmt]; //bytes 3-4 (RECHARGE AMOUNT): the amount of last recharge
    byte bcdRechargePost[lenBcdTid];    //bytes 5-8 (RECHARGE POST): the terminal ID that performed the recharge
    byte bcdRechargeStan[lenBcdStan];   //bytes 9-12 (SYSTEM TRACE AUDIT NUMBER=STAN): recharge operation sequential number assigned by the Front Office
    byte bcdRechargeValDate[lenBcdYYMMDD];  //bytes 16-18 (RECHARGE VALIDITY DATE): the date after which the card will be desactivated until a recharge operation

    char stan[lenStan + 1] = "";
    char valDate[lenYYMMDD + 1] = "";
    char post[lenTid + 1] = "";
    char rcgDate[lenYYMMDD + 1] = "";
    card amt = 0;

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

    trcS("persoLastRechargeInfo Beg\n");

    ret = mfcAut(0, 0xA0 | sec);
    trcFN("mfcAut ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    memset(buf, 0, sizeof(buf));

    //LAST RECHARGE DATE
    memcpy(rcgDate, traRechargeDate, lenYYMMDD);    // future MAPGET 

    hex2bin(bcdRechargeDate, rcgDate, lenYYMMDD);
    memcpy(buf, bcdRechargeDate, lenBcdYYMMDD);
    trcS("LAST RECHARGE DATE = ");
    trcBAN(bcdRechargeDate, lenBcdYYMMDD);
    trcS("\n");

    //LAST RECHARGE AMOUNT
    amt = traAmt;               // future MAPGET 

    num2bin(bcdRechargeAmt, amt, lenBcdAmt);
    memcpy(buf + lenBcdYYMMDD, bcdRechargeAmt, lenBcdAmt);
    trcS("RECHARGE AMOUNT = ");
    trcBAN(bcdRechargeAmt, lenBcdAmt);
    trcS("\n");

    //RECHARGE POST
    memcpy(post, traRechargeStan, lenStan); // future MAPGET  

    hex2bin(bcdRechargePost, post, lenBcdTid);
    memcpy(buf + lenBcdYYMMDD + lenBcdAmt, bcdRechargePost, lenBcdTid);
    trcS("RECHARGE POST = ");
    trcBAN(bcdRechargePost, lenBcdTid);
    trcS("\n");

    //RECHARGE STAN
    memcpy(stan, traRechargePostId, lenTid);    // future MAPGET 

    hex2bin(bcdRechargeStan, stan, lenBcdStan);
    memcpy(buf + lenBcdYYMMDD + lenBcdAmt + lenBcdTid, bcdRechargeStan,
           lenBcdStan);
    trcS("RECHARGE STAN = ");
    trcBAN(bcdRechargeStan, lenBcdStan);
    trcS("\n");

    //RECHARGE VALIDITY DATE
    memcpy(valDate, traRechargeValDate, lenYYMMDD); // future MAPGET  

    hex2bin(bcdRechargeValDate, valDate, lenBcdYYMMDD);
    memcpy(buf + lenBcdYYMMDD + lenBcdAmt + lenBcdTid + lenBcdStan,
           bcdRechargeValDate, lenBcdYYMMDD);
    trcS("RECHARGE VAL DATE = ");
    trcBAN(bcdRechargeValDate, lenBcdYYMMDD);
    trcS("\n");

    // Write Data to block 0
    ret = mfcSave(buf, (sec << 4) | 0x00, 'D');
    trcFN("mfcSave ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    trcS("SECTOR 3: LAST RECHARGE INFO\n");
    trcS("BLOCK = ");
    trcBAN(buf, sizeof(buf));
    trcS("\n");

    // Trailer setting

    // build a buffer with keys and conditions
    // buf = newKeyA.conditions.newKeyB
    // conditions = 787788FF : 
    //      with key A: read trailer, read data
    //      with key B: read/write trailer, write key A, write key B, read/write data
    //      operations increment/decrement are forbidden for both keys

    persoWriteTrailer(A, B, sec, condType);

    goto lblEnd;

  lblKO:
    trcS("persoLastRechargeInfo ERROR\n");
    return -1;
  lblEnd:
    trcS("persoLastRechargeInfo End\n");
    return 1;
}

static int persoCreditAmount(TKey A, TKey B, byte sec, byte condType) {
    int ret;
    byte amt[4];
    byte trc[4];
    card credAmt = 0;
    card credStan = 0;

    /*
       SECTOR 4: CREDIT AMOUNT
       Only RECHARGE and PERSO applications have a key that allows to write to this sector.

       The first block of the sector contains CREDIT AMOUNT.
       It is incremented by RECHARGE application during RECHARGE operation.

       The second block contains the CREDIT TRACE NUMER, i. e. the number of recharge operations performed with the card since its personalization.
       It is incremanted by one each time when a RECHARGE operation is performed.
     */

    trcS("persoCreditAmount Beg\n");

    ret = mfcAut(0, 0xA0 | sec);
    trcFN("mfcAut ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    memset(amt, 0, sizeof(amt));
    memset(trc, 0, sizeof(trc));
    credAmt = traCreditAmt;     // future MAPGETCARD
    credStan = traCreditStan;   // future MAPGETCARD

    num2bin(amt, credAmt, sizeof(amt));
    num2bin(trc, credStan, sizeof(trc));

    // Write VALUE to block 0
    ret = mfcSave(amt, (sec << 4) | 0x00, 'V');
    trcFN("mfcSave ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    trcS("SECTOR 4: CREDIT AMOUNT\n");
    trcS("AMOUNT = ");
    trcBAN(amt, sizeof(amt));
    trcS("\n");

    // Write VALUE to block 1
    ret = mfcSave(trc, (sec << 4) | 0x01, 'V');
    trcFN("mfcSave ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    trcS("TRACE NUM = ");
    trcBAN(trc, sizeof(trc));
    trcS("\n");

    // Trailer setting
    // build a buffer with keys and conditions
    // buf = newKeyA.conditions.newKeyB
    // conditions =  08778FFF : 
    //      with key A: read trailer, read data,decrement
    //      with key B: read/write trailer, write key A, write key B, read/write data, increment/decrement
    //       

    persoWriteTrailer(A, B, sec, condType);

    goto lblEnd;

  lblKO:
    trcS("persoCreditAmount ERROR\n");
    return -1;
  lblEnd:
    trcS("persoCreditAmount End\n");
    return 1;
}

static int persoLastPaymentInfo(TKey A, TKey B, byte sec, byte condType) {
    int ret;
    byte buf[16];
    byte bcdPassTimestamp[lenBcdYYMMDDhhmm];    //bytes 0-4 (PASS TIMESTAMP): the timestamp of last payment in YYMMDDhhmm format
    byte bcdBusId[lenBcdTid];   //bytes 6-9 (TERMINAL ID): the terminal ID that performed the payment
    byte bcdPmtStan[lenBcdPmtStan]; //bytes 11-12 (PAYMENT SEQUENTIAL NUMBER): payment operation sequential number, unique per terminal per day

    char datetime[lenYYMMDDhhmm + 1] = "";
    char busid[lenTid + 1] = "";
    byte passNum = 0;
    byte batNum = 0;
    card pmtStan = 0;

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

    trcS("persoLastPaymentInfo Beg\n");

    ret = mfcAut(0, 0xA0 | sec);
    trcFN("mfcAut ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    memset(buf, 0, sizeof(buf));

    //LAST PAYMENT DATETIME
    memcpy(datetime, traPassDateTime, lenYYMMDDhhmm);   // future MAPGET    

    hex2bin(bcdPassTimestamp, datetime, lenBcdYYMMDDhhmm);
    memcpy(buf, bcdPassTimestamp, lenBcdYYMMDDhhmm);
    trcS("PASS TIMESTAMP = ");
    trcBAN(bcdPassTimestamp, lenBcdYYMMDDhhmm);
    trcS("\n");

    //NUMBER OF PASSES
    passNum = traPassNum;       // future MAPGET  

    memcpy(buf + lenBcdYYMMDDhhmm, &passNum, 1);
    trcS("PASS NUM = ");
    trcBAN(&passNum, 1);
    trcS("\n");

    //BUS ID
    memcpy(busid, traBusId, lenTid);    // future MAPGET 

    hex2bin(bcdBusId, busid, lenBcdTid);
    memcpy(buf + lenBcdYYMMDDhhmm + 1, bcdBusId, lenBcdTid);
    trcS("PASS BUS ID = ");
    trcBAN(bcdBusId, lenBcdTid);
    trcS("\n");

    //BATCH NUM
    batNum = traBatchNum;       // future MAPGET  

    memcpy(buf + lenBcdYYMMDDhhmm + 1 + lenBcdTid, &batNum, 1);
    trcS("PASS BATCH NUM = ");
    trcBAN(&batNum, 1);
    trcS("\n");

    //PAYMENT STAN
    pmtStan = traPmtStan;       // future MAPGET       
    num2bin(bcdPmtStan, pmtStan, lenBcdPmtStan);
    memcpy(buf + lenBcdYYMMDDhhmm + 1 + lenBcdTid + 1, bcdPmtStan,
           lenBcdPmtStan);
    trcS("PASS BATCH NUM = ");
    trcBAN(bcdPmtStan, lenBcdPmtStan);
    trcS("\n");

    // Write Data to block 0
    ret = mfcSave(buf, (sec << 4) | 0x00, 'D');
    trcFN("mfcSave ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    trcS("SECTOR 5: LAST PAYMENT INFO\n");
    trcS("BLOCK = ");
    trcBAN(buf, sizeof(buf));
    trcS("\n");

    // Trailer setting

    // build a buffer with keys and conditions
    // buf = newKeyA.conditions.newKeyB
    // conditions = 787788FF : 
    //      with key A: read trailer, read data
    //      with key B: read/write trailer, write key A, write key B, read/write data
    //      operations increment/decrement are forbidden for both keys

    persoWriteTrailer(A, B, sec, condType);

    goto lblEnd;

  lblKO:
    trcS("persoLastPaymentInfo ERROR\n");
    return -1;
  lblEnd:
    trcS("persoLastPaymentInfo End\n");
    return 1;
}

static int persoDebitAmount(TKey A, TKey B, byte sec, byte condType) {
    int ret;
    byte amt[4];
    byte trc[4];
    card dbtAmt = 0;
    card dbtStan = 0;

    /*
       SECTOR 6: DEBIT AMOUNT

       BUS application has a key that allows to write to this sector.

       The first block of the sector contains DEBIT AMOUNT.
       It is incremented by BUS application during PAYMENT operation.

       The second block contains the DEBIT TRACE NUMER, i. e. the number of payment operations performed with the card since its personalization.
       It is incremanted by one each time when a PAYMENT operation is performed.
     */

    trcS("persoDebitAmount Beg\n");

    ret = mfcAut(0, 0xA0 | sec);
    trcFN("mfcAut ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    memset(amt, 0, sizeof(amt));
    memset(trc, 0, sizeof(trc));
    dbtAmt = traDebitAmt;       // future MAPGETCARD
    dbtStan = traDebitStan;     // future MAPGETCARD

    num2bin(amt, dbtAmt, sizeof(amt));
    num2bin(trc, dbtStan, sizeof(trc));

    // Write VALUE to block 0
    ret = mfcSave(amt, (sec << 4) | 0x00, 'V');
    trcFN("mfcSave ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    trcS("SECTOR 6: DEBIT AMOUNT\n");
    trcS("AMOUNT = ");
    trcBAN(amt, sizeof(amt));
    trcS("\n");

    // Write VALUE to block 1
    ret = mfcSave(amt, (sec << 4) | 0x01, 'V');
    trcFN("mfcSave ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    trcS("TRACE NUM = ");
    trcBAN(trc, sizeof(trc));
    trcS("\n");

    // Trailer setting
    // build a buffer with keys and conditions
    // buf = newKeyA.conditions.newKeyB
    // conditions =  08778FFF : 
    //      with key A: read trailer, read data,decrement
    //      with key B: read/write trailer, write key A, write key B, read/write data, increment/decrement
    //       

    persoWriteTrailer(A, B, sec, condType);

    goto lblEnd;

  lblKO:
    trcS("persoDebitAmount ERROR\n");
    return -1;
  lblEnd:
    trcS("persoDebitAmount End\n");
    return 1;
}

void tcns0021(void) {
    int ret;
    byte SECTOR = 0;

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    prtS("tc0021 PERSO\n");

    ret = mfcStart();
    trcFN("mfcStart ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    ret = mfcDetect(60);
    trcFN("mfcDetect ret = %d\n", ret);
    CHECK(ret >= 0, lblKO);

    // personalize sector 1, rewrite trailer with KeyA = KF, ConditionsType = COND_1, KeyB = KP 
    SECTOR = 1;
    ret = persoCardIdentification(KF, KP, SECTOR, COND_1);
    CHECK(ret >= 0, lblKO);
    prtS("persoCardIdentification");

    //2
    SECTOR = 2;
    ret = persoCardHolderIdentification(KR, KP, SECTOR, COND_1);
    CHECK(ret >= 0, lblKO);
    prtS("persoCardHolderIdentification");

    //3
    SECTOR = 3;
    ret = persoLastRechargeInfo(KR, KC, SECTOR, COND_1);
    CHECK(ret >= 0, lblKO);
    prtS("persoLastRechargeInfo");

    //4
    SECTOR = 4;
    ret = persoCreditAmount(KR, KC, SECTOR, COND_2);
    CHECK(ret >= 0, lblKO);
    prtS("persoCreditAmount");

    //5
    SECTOR = 5;
    ret = persoLastPaymentInfo(KR, KB, SECTOR, COND_1);
    CHECK(ret >= 0, lblKO);
    prtS("persoLastPaymentInfo");

    //6
    SECTOR = 6;
    ret = persoDebitAmount(KR, KB, SECTOR, COND_2);
    CHECK(ret >= 0, lblKO);
    prtS("persoDebitAmount");

    goto lblEnd;

  lblKO:
    prtS("tc0021 KO\n");

  lblEnd:
    prtS("--------");
    prtS("tc0021 end\n");
    mfcStop();
    prtStop();
    dspStop();

}
#else
void tcns0021(void) {
}
#endif
