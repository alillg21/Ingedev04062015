/** \file
 * Unitary test case tcab0032.
 * Testing converters.
 * \sa
 *  - cvtInit()
 *  - cvtGet()
 *  - cvtPut()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0032.c $
 *
 * $Id: tcab0032.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "pri.h"
#include "tst.h"

enum eRec {                     //record fields
    recBeg,                     //start sentinel
    recNum,
    recDec,
    recHex,
    recBin,
    recEnd                      //end sentinel
};

typedef struct sSample {
    word num;
    char dec[5];
    char hex[4];
    char bin[3];
} tSample;

static int fillRec(tRecord * rec) {
    int ret;

    ret = prtS("FillRecord...");
    CHECK(ret >= 0, lblKO);
    ret = recPutWord(rec, recNum, 0x1234);
    CHECK(ret >= 0, lblKO);
    ret = prtS("Num = 1234");
    CHECK(ret >= 0, lblKO);
    ret = recPutStr(rec, recDec, "23456");
    CHECK(ret >= 0, lblKO);
    ret = prtS("Dec = \"23456\"");
    CHECK(ret >= 0, lblKO);
    ret = recPutStr(rec, recHex, "ABCD");
    CHECK(ret >= 0, lblKO);
    ret = prtS("Hex = \"ABCD\"");
    CHECK(ret >= 0, lblKO);
    ret = recPut(rec, recBin, "\x78\x9A\xBC", 3);
    CHECK(ret >= 0, lblKO);
    ret = prtS("Bin = \"\\x78\\x9A\\xBC\"");
    CHECK(ret >= 0, lblKO);
    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

int testNumDec(const tContainer * cnt) {
    int ret;
    tConvert cvt;
    char tmp[dspW + 1];
    word buf;

    VERIFY(cnt);

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "NumDec...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("======== NumDec ========");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "cvtInit...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("cvtInit Num2Dec (str5)");
    CHECK(ret >= 0, lblKO);
    ret = cvtInit(&cvt, cnt, recNum, fmtNum, fmtDec, 5);    //5= result length
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspLS(2, "cvtGet...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("cvtGet...");
    CHECK(ret >= 0, lblKO);
    ret = cvtGet(&cvt, recNum, tmp, dspW + 1);  //last parameter= destination buffer length
    CHECK(ret >= 0, lblKO);
    ret = dspLS(3, tmp);
    CHECK(ret >= 0, lblKO);
    CHECK(strcmp(tmp, "04660") == 0, lblKO);    //0x1234==4660
    ret = prtS("Dec = \"04660\"");
    CHECK(ret >= 0, lblKO);

    tmrPause(1);

    ret = dspLS(1, "cvtPut...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("cvtPut...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("Dec = \"22222\"");
    CHECK(ret >= 0, lblKO);
    ret = cvtPut(&cvt, recNum, "22222", 0); //length calculated as strlen
    CHECK(ret >= 0, lblKO);
    ret = cvtPut(&cvt, NULLKEY, "22222", 0);    //the result should be the same
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspLS(2, "cntGetWord...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("cntGetWord...");
    CHECK(ret >= 0, lblKO);
    ret = cntGetWord(cnt, recNum, buf);
    CHECK(ret >= 0, lblKO);
    CHECK(buf == 22222, lblKO);
    ret = dspLS(3, "22222");
    CHECK(ret >= 0, lblKO);
    ret = prtS("Num = 22222");
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

int testNumHex(const tContainer * cnt) {
    int ret;
    tConvert cvt;
    char tmp[dspW + 1];
    word buf;

    VERIFY(cnt);

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "NumHex...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("======== NumHex ========");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "cvtInit...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("cvtInit Num2Hex (str4)");
    CHECK(ret >= 0, lblKO);
    ret = cvtInit(&cvt, cnt, recNum, fmtNum, fmtHex, 4);    //4= length of the result
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspLS(2, "cvtGet...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("cvtGet...");
    CHECK(ret >= 0, lblKO);
    ret = cvtGet(&cvt, recNum, tmp, dspW + 1);  //in get functions the last parameter is buffer size
    CHECK(ret >= 0, lblKO);
    ret = dspLS(3, tmp);
    CHECK(ret >= 0, lblKO);
    CHECK(strcmp(tmp, "1234") == 0, lblKO);
    ret = prtS("Hex = \"04D2\"");
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspLS(1, "cvtPut...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("cvtPut...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("Hex = \"ABCD\"");
    CHECK(ret >= 0, lblKO);
    ret = cvtPut(&cvt, recNum, "ABCD", 0);  //length calculated as strlen
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspLS(2, "cntGetWord...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("cntGetWord...");
    CHECK(ret >= 0, lblKO);
    ret = cntGetWord(cnt, recNum, buf);
    CHECK(ret >= 0, lblKO);
    CHECK(buf == 0xABCD, lblKO);
    ret = dspLS(3, "ABCD");
    CHECK(ret >= 0, lblKO);
    ret = prtS("Num = 43981");
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

int testNumBin(const tContainer * cnt) {
    int ret;
    tConvert cvt;
    byte tmp[4];
    word buf;

    VERIFY(cnt);

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "NumBin...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("======== NumBin ========");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "cvtInit...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("cvtInit Num2Bin (str3)");
    CHECK(ret >= 0, lblKO);
    ret = cvtInit(&cvt, cnt, recNum, fmtNum, fmtBin, 3);    //3= the outside length
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspLS(2, "cvtGet...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("cvtGet...");
    CHECK(ret >= 0, lblKO);
    ret = cvtGet(&cvt, recNum, tmp, 4); //4= size of buffer
    CHECK(ret >= 0, lblKO);
    CHECK(memcmp(tmp, "\x00\x12\x34", 3) == 0, lblKO);  //retrieved in direct order
    ret = dspLS(3, "001234");
    CHECK(ret >= 0, lblKO);
    ret = prtS("Bin = \"\\x00\\x12\\x34\"");
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspLS(1, "cvtPut...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("cvtPut...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("Bin = \"\\x00\\xAB\\xCD\"");
    CHECK(ret >= 0, lblKO);
    ret = cvtPut(&cvt, recNum, "\x00\xAB\xCD", 3);  //3= source length, should be equal to outside length
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspLS(2, "cntGetWord...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("cntGetWord...");
    CHECK(ret >= 0, lblKO);
    ret = cntGetWord(cnt, recNum, buf);
    CHECK(ret >= 0, lblKO);
    CHECK(buf == 0xABCD, lblKO);    //saved in deirect order
    ret = dspLS(3, "ABCD");
    ret = prtS("Num = 43981");
    CHECK(ret >= 0, lblKO);
    CHECK(ret >= 0, lblKO);

    tmrPause(1);

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

int testDecNum(const tContainer * cnt) {
    int ret;
    tConvert cvt;
    char tmp[dspW + 1];
    word buf;

    VERIFY(cnt);

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "DecNum...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("======== DecNum ========");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "cvtInit...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("cvtInit Dec2Num (word)");
    CHECK(ret >= 0, lblKO);
    ret = cvtInit(&cvt, cnt, recDec, fmtDec, fmtNum, 2);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspLS(2, "cvtGet...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("cvtGet...");
    CHECK(ret >= 0, lblKO);
    ret = cvtGet(&cvt, recDec, &buf, sizeof(word));
    CHECK(ret >= 0, lblKO);
    CHECK(buf == 23456, lblKO);
    ret = dspLS(3, "23456");
    CHECK(ret >= 0, lblKO);
    ret = prtS("Num = 23456");
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspLS(1, "cvtPut...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("cvtPut...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("Num = 22222");
    CHECK(ret >= 0, lblKO);
    ret = cvtPut(&cvt, recDec, wPtr(22222), sizeof(word));
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspLS(2, "cntGetStr...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("cntGetStr...");
    CHECK(ret >= 0, lblKO);
    ret = cntGetStr(cnt, recDec, tmp);
    CHECK(ret >= 0, lblKO);
    ret = dspLS(3, tmp);
    CHECK(ret >= 0, lblKO);
    CHECK(strcmp(tmp, "22222") == 0, lblKO);
    ret = prtS("Dec = \"22222\"");
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

int testHexNum(const tContainer * cnt) {
    int ret;
    tConvert cvt;
    char tmp[dspW + 1];
    word buf;

    VERIFY(cnt);

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "HexNum...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("======== HexNum ========");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "cvtInit...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("cvtInit Hex2Num (word)");
    CHECK(ret >= 0, lblKO);
    ret = cvtInit(&cvt, cnt, recHex, fmtHex, fmtNum, 2);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspLS(2, "cvtGet...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("cvtGet...");
    CHECK(ret >= 0, lblKO);
    ret = cvtGet(&cvt, recHex, &buf, sizeof(word));
    CHECK(ret >= 0, lblKO);
    CHECK(buf == 0xABCD, lblKO);
    ret = dspLS(3, "ABCD");
    CHECK(ret >= 0, lblKO);
    ret = prtS("Num = ABCD");
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspLS(1, "cvtPut...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("cvtPut...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("Num = 22222");
    CHECK(ret >= 0, lblKO);
    ret = cvtPut(&cvt, recHex, wPtr(0xAAAA), sizeof(word));
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspLS(2, "cntGetStr...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("cntGetStr...");
    CHECK(ret >= 0, lblKO);
    ret = cntGetStr(cnt, recHex, tmp);
    CHECK(ret >= 0, lblKO);
    ret = dspLS(3, tmp);
    CHECK(ret >= 0, lblKO);
    CHECK(strcmp(tmp, "AAAA") == 0, lblKO);
    ret = prtS("Hex = \"AAAA\"");
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

int testBinNum(const tContainer * cnt) {
    int ret;
    tConvert cvt;
    char tmp[dspW + 1];
    card buf;

    VERIFY(cnt);

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "BinNum...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("======== BinNum ========");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "cvtInit...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("cvtInit Bin2Num (card)");
    CHECK(ret >= 0, lblKO);
    ret = cvtInit(&cvt, cnt, recBin, fmtBin, fmtNum, sizeof(card)); //external view: card
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspLS(2, "cvtGet...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("cvtGet...");
    CHECK(ret >= 0, lblKO);
    ret = cvtGet(&cvt, recBin, &buf, sizeof(card)); //last parameter= size of destination buffer
    CHECK(ret >= 0, lblKO);
    CHECK(buf == 0x789ABC, lblKO);
    ret = dspLS(3, "789ABC");
    CHECK(ret >= 0, lblKO);
    ret = prtS("Num = 7903932");
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspLS(1, "cvtPut...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("cvtPut...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("Num = 42422");
    CHECK(ret >= 0, lblKO);
    ret = cvtPut(&cvt, recBin, cPtr(0xA5B6), sizeof(card)); //last parameter= length of source buffer, should be equal to external view size
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspLS(2, "cntGetStr...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("cntGetStr...");
    CHECK(ret >= 0, lblKO);
    ret = cntGet(cnt, recBin, tmp, dspW + 1);
    CHECK(ret >= 0, lblKO);
    CHECK(memcmp(tmp, "\x00\xA5\xB6", 3) == 0, lblKO);
    ret = dspLS(3, "00A5B6");
    CHECK(ret >= 0, lblKO);
    ret = prtS("Bin = \"\\x00\\xA5\\xB6\"");
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

int testBinDec(const tContainer * cnt) {
    int ret;
    tConvert cvt;
    char tmp[dspW + 1];

    VERIFY(cnt);

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "BinDec...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("======== BinDec ========");
    CHECK(ret >= 0, lblKO);

    ret = cntPutStr(cnt, recBin, "\x78\x9A\xBC");
    CHECK(ret >= 0, lblKO);     //only decimal digits

    ret = dspLS(1, "cvtInit...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("cvtInit Bin2Dec (str6)");
    CHECK(ret >= 0, lblKO);
    ret = cvtInit(&cvt, cnt, recBin, fmtBin, fmtDec, 7);    //7= length of external representation including ending zero
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspLS(2, "cvtGet...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("cvtGet...");
    CHECK(ret >= 0, lblKO);
    ret = cvtGet(&cvt, recBin, tmp, dspW + 1);  //last parameter= length of destination buffer
    CHECK(ret >= 0, lblKO);
    ret = dspLS(3, tmp);
    CHECK(ret >= 0, lblKO);
    CHECK(strcmp(tmp, "7903932") == 0, lblKO);  //789ABC hex = 7903932 dec
    ret = prtS("Dec = \"789ABC\"");
    CHECK(ret >= 0, lblKO);
    tmrPause(1);
    ret = dspLS(1, "cvtPut...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("cvtPut...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("Dec = \"123456\"");
    CHECK(ret >= 0, lblKO);
    ret = cvtPut(&cvt, recBin, "123456", 0);    //lengh is calculated as strlen
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspLS(2, "cntGetStr...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("cntGetStr...");
    CHECK(ret >= 0, lblKO);
    ret = cntGetStr(cnt, recBin, tmp);
    CHECK(ret >= 0, lblKO);
    CHECK(memcmp(tmp, "\x12\x34\x56", 3) == 0, lblKO);
    ret = dspLS(3, "123456");
    CHECK(ret >= 0, lblKO);
    ret = prtS("Bin = \"\\x12\\x34\\x56\"");
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

int testBinHex(const tContainer * cnt) {
    int ret;
    tConvert cvt;
    char tmp[dspW + 1];

    VERIFY(cnt);

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "BinHex...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("======== BinHex ========");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "cvtInit...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("cvtInit Bin2Hex (str6)");
    CHECK(ret >= 0, lblKO);
    ret = cvtInit(&cvt, cnt, recBin, fmtBin, fmtHex, 7);    //7= length of external representation including ending zero
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspLS(2, "cvtGet...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("cvtGet...");
    CHECK(ret >= 0, lblKO);
    ret = cvtGet(&cvt, recBin, tmp, dspW + 1);  //last parameter= length of destination buffer
    CHECK(ret >= 0, lblKO);
    ret = dspLS(3, tmp);
    CHECK(ret >= 0, lblKO);
    CHECK(strcmp(tmp, "789ABC") == 0, lblKO);
    ret = prtS("Hex = \"789ABC\"");
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspLS(1, "cvtPut...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("cvtPut...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("Hex = \"DEF456\"");
    CHECK(ret >= 0, lblKO);
    ret = cvtPut(&cvt, recBin, "DEF456", 0);    //calculated length= strlen
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspLS(2, "cntGetStr...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("cntGetStr...");
    CHECK(ret >= 0, lblKO);
    ret = cntGetStr(cnt, recBin, tmp);
    CHECK(ret >= 0, lblKO);
    CHECK(memcmp(tmp, "\xDE\xF4\x56", 3) == 0, lblKO);
    ret = dspLS(3, "DEF456");
    CHECK(ret >= 0, lblKO);
    ret = prtS("Bin = \"\\xDE\\xF4\\x56\"");
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

void tcab0032(void) {
    int ret;
    tSample sample;             //memory for record
    tRecord rec;
    word recLen[recEnd];        //contains field length
    word recMap[recEnd];        //contains field offsets
    tContainer cnt;

    trcS("tcab0032 Beg\n");

    nvmStart();                 //initialise non-volatile memory
    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = prtS("**** Vol. mem. RAM ****");
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "dbaSetPtr...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("dbaSetPtr...");
    CHECK(ret >= 0, lblKO);

    memset(&sample, 0, sizeof(tSample));
    dbaSetPtr(&sample);

    memset(recLen, 0, sizeof recLen);
    //fill lengths; offsets will be filled by recInit
    recLen[recNum] = sizeof(word);
    recLen[recDec] = 5;
    recLen[recHex] = 4;
    recLen[recBin] = 3;
    memset(recMap, 0, sizeof recMap);

    tmrPause(1);

    ret = dspLS(1, "Vol recInit...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("Vol recInit...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("Word Str(5) Str(4) Str(3)");

    ret = recInit(&rec, 0xFF, 0, recEnd, recLen, 0, recMap);    //initiate a record in volatile memory
    VERIFY(ret == (int) recSize(&rec));

    tmrPause(1);

    ret = dspLS(2, "cntInit...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("cntInit...");
    CHECK(ret >= 0, lblKO);

    ret = cntInit(&cnt, 'r', &rec);
    CHECK(ret >= 0, lblKO);

    ret = prtS("************************");
    CHECK(ret >= 0, lblKO);
    ret = fillRec(&rec);
    CHECK(ret >= 0, lblKO);
    ret = testNumDec(&cnt);
    CHECK(ret >= 0, lblKO);

    ret = prtS("************************");
    CHECK(ret >= 0, lblKO);
    ret = fillRec(&rec);
    CHECK(ret >= 0, lblKO);
    ret = testNumHex(&cnt);
    CHECK(ret >= 0, lblKO);

    ret = prtS("************************");
    CHECK(ret >= 0, lblKO);
    ret = fillRec(&rec);
    CHECK(ret >= 0, lblKO);
    ret = testNumBin(&cnt);
    CHECK(ret >= 0, lblKO);

    ret = prtS("************************");
    CHECK(ret >= 0, lblKO);
    ret = fillRec(&rec);
    CHECK(ret >= 0, lblKO);
    ret = testDecNum(&cnt);
    CHECK(ret >= 0, lblKO);

    ret = prtS("************************");
    CHECK(ret >= 0, lblKO);
    ret = fillRec(&rec);
    CHECK(ret >= 0, lblKO);
    ret = testHexNum(&cnt);
    CHECK(ret >= 0, lblKO);

    ret = prtS("************************");
    CHECK(ret >= 0, lblKO);
    ret = fillRec(&rec);
    CHECK(ret >= 0, lblKO);
    ret = testBinNum(&cnt);
    CHECK(ret >= 0, lblKO);

    ret = prtS("************************");
    CHECK(ret >= 0, lblKO);
    ret = fillRec(&rec);
    CHECK(ret >= 0, lblKO);
    ret = testBinDec(&cnt);
    CHECK(ret >= 0, lblKO);

    ret = prtS("************************");
    CHECK(ret >= 0, lblKO);
    ret = fillRec(&rec);
    CHECK(ret >= 0, lblKO);
    ret = testBinHex(&cnt);
    CHECK(ret >= 0, lblKO);

    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
  lblEnd:
    tmrPause(3);
    dspStop();                  //close resource
    prtStop();
    trcS("tcab0032 End\n");
}
