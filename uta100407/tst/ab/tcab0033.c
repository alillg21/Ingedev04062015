/** \file
 * Unitary test case tcab0033.
 * Testing combo.
 * \sa
 *  - cmbInit
 *  - cmbGet
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0033.c $
 *
 * $Id: tcab0033.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "pri.h"
#include "tst.h"

static int testSbs(void) {
    int ret;

    enum eRec {                 //record fields
        recStr,
        recOfs,
        recLength,
        recEnd                  //end sentinel
    };
    typedef struct sTestSbs {
        char str[16];
        byte ofs;
        byte len;
    } tTestSbs;
    tRecord rec;
    word recLen[recEnd];        //contains field lengths
    word recMap[recEnd];        //contains field offsets
    tTestSbs volDba;

    tContainer cnt;
    tCombo cmb;
    tLocator loc[3];
    char buf[prtW + 1];

    ret = prtS("==== Vol. mem. SRAM ====");
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "dbaSetPtr...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("dbaSetPtr...");
    CHECK(ret >= 0, lblKO);

    memset(&volDba, 0, sizeof(volDba));
    dbaSetPtr(&volDba);

    memset(recLen, 0, sizeof recLen);
    //fill lengths; offset will be filled by recInit
    recLen[recStr] = 16;
    recLen[recOfs] = sizeof(byte);
    recLen[recLength] = sizeof(byte);
    memset(recMap, 0, sizeof recMap);

    ret = dspLS(1, "recInit...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("Vol recInit...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("Str(16) Byte Byte");

    ret = recInit(&rec, 0xFF, 0, recEnd, recLen, 0, recMap);    //initiate a record in volatile memory
    CHECK(ret > 0, lblKO);

    tmrPause(1);

    ret = dspLS(2, "cntInit...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("cntInit...");
    CHECK(ret >= 0, lblKO);

    ret = cntInit(&cnt, 'r', &rec);
    CHECK(ret > 0, lblKO);

    tmrPause(1);

    loc[0].cnt = &cnt;
    loc[0].key = recStr;        //source string
    loc[1].cnt = &cnt;
    loc[1].key = recOfs;        //offset
    loc[2].cnt = &cnt;
    loc[2].key = recLength;     //length

    ret = dspLS(3, "cmbInit...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("==== cmbInit oprSbs =====");
    CHECK(ret >= 0, lblKO);

    ret = prtS("FillRecord...");
    CHECK(ret >= 0, lblKO);
    ret = recPutStr(&rec, recStr, "ABCDEFGHIJKLMNO");
    CHECK(ret > 0, lblKO);      //source string
    ret = prtS("Str=\"ABCDEFGHIJKLMNO\"");
    CHECK(ret >= 0, lblKO);
    ret = recPutByte(&rec, recOfs, 3);
    CHECK(ret > 0, lblKO);      //offset
    ret = prtS("Ofs=3");
    CHECK(ret >= 0, lblKO);
    ret = recPutByte(&rec, recLength, 4);
    CHECK(ret > 0, lblKO);      //length 
    ret = prtS("Len=4");
    CHECK(ret >= 0, lblKO);

    ret = cmbInit(&cmb, oprSbs, loc, prtW + 1);

    CHECK(ret > 0, lblKO);
    tmrPause(1);

    ret = dspLS(2, "cmbGet...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("cmbGet...");
    CHECK(ret >= 0, lblKO);

    ret = cmbGet(&cmb, 0, buf, prtW + 1);   //get substring
    CHECK(ret > 0, lblKO);

    ret = prtS(buf);
    CHECK(ret > 0, lblKO);
    CHECK(strcmp(buf, "DEFG") == 0, lblKO);
    tmrPause(1);

    //another example: transfer of length by value
    loc[0].cnt = &cnt;
    loc[0].key = recStr;        //source string
    loc[1].cnt = &cnt;
    loc[1].key = recOfs;        //offset
    loc[2].cnt = 0;             //to indicate that the parameter is transferred by value
    loc[2].key = 5;             //length

    ret = dspLS(3, "cmbInit...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("==== cmbInit oprSbs =====");
    CHECK(ret >= 0, lblKO);

    ret = prtS("FillRecord...");
    CHECK(ret >= 0, lblKO);
    ret = recPutStr(&rec, recStr, "ABCDEFGHIJKLMNO");
    CHECK(ret > 0, lblKO);      //source string
    ret = prtS("Str=\"ABCDEFGHIJKLMNO\"");
    CHECK(ret >= 0, lblKO);
    ret = recPutByte(&rec, recOfs, 3);
    CHECK(ret > 0, lblKO);      //offset
    ret = prtS("Ofs=3");
    CHECK(ret >= 0, lblKO);
    ret = prtS("Len=5");
    CHECK(ret >= 0, lblKO);

    ret = cmbInit(&cmb, oprSbs, loc, prtW + 1);
    CHECK(ret > 0, lblKO);
    tmrPause(1);

    ret = dspLS(2, "cmbGet...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("cmbGet...");
    CHECK(ret >= 0, lblKO);

    ret = cmbGet(&cmb, 0, buf, prtW + 1);   //get substring
    CHECK(ret > 0, lblKO);

    ret = prtS(buf);
    CHECK(ret > 0, lblKO);
    CHECK(strcmp(buf, "DEFGH") == 0, lblKO);
    tmrPause(1);

    ret = 1;
    goto lblEnd;

  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    return ret;
}

static int testMut(void) {
    int ret;
    tTable tab;
    tContainer cnt;
    tCombo cmb;
    tLocator loc[3];
    char buf[prtW + 1];
    char volDba[prtW * 4];

    ret = prtS("==== Vol. mem. SRAM ====");
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "dbaSetPtr...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("dbaSetPtr...");
    CHECK(ret >= 0, lblKO);

    memset(volDba, 0, sizeof volDba);
    dbaSetPtr(&volDba);

    ret = dspLS(1, "tabInit...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("Vol tabInit...");
    CHECK(ret >= 0, lblKO);
    ret = prtS(" rows:4 len:16 ");

    ret = tabInit(&tab, 0xFF, 0, prtW, 4, 0);   //initiate a table in volatile memory of four rows of width dspW
    VERIFY(ret == (int) tabSize(&tab));

    tmrPause(1);

    ret = dspLS(2, "cntInit...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("cntInit...");
    CHECK(ret >= 0, lblKO);

    ret = cntInit(&cnt, 't', &tab);
    CHECK(ret > 0, lblKO);

    tmrPause(1);

    loc[0].cnt = &cnt;
    loc[0].key = NULLKEY;       //combo will be applied to any row
    loc[1].cnt = &cnt;
    loc[1].key = 1;
    loc[2].cnt = &cnt;
    loc[2].key = 2;

    ret = dspLS(3, "cmbInit...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("==== cmbInit oprMut =====");
    CHECK(ret >= 0, lblKO);

    ret = prtS("FillTable...");
    CHECK(ret >= 0, lblKO);
    ret = tabPutStr(&tab, 0, "20040902164700");
    CHECK(ret > 0, lblKO);      //foreground string
    ret = prtS("Row0=\"20040902164700\"");
    CHECK(ret >= 0, lblKO);
    ret = tabPutStr(&tab, 1, "-: ");
    CHECK(ret > 0, lblKO);      //background string
    ret = prtS("Row1=\"-: \"");
    CHECK(ret >= 0, lblKO);
    ret = tabPutStr(&tab, 2, "ghAefAabcdCCijBklBmn");
    CHECK(ret > 0, lblKO);      //control string
    ret = prtS("Row2=\"ghAefAabcdCCijBklBmn\"");
    CHECK(ret >= 0, lblKO);

    ret = cmbInit(&cmb, oprMut, loc, prtW + 1);
    CHECK(ret > 0, lblKO);

    tmrPause(1);

    ret = dspLS(2, "cmbGet...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("cmbGet...");
    CHECK(ret >= 0, lblKO);

    ret = cmbGet(&cmb, 0, buf, prtW + 1);   //build combo using row 0
    CHECK(ret > 0, lblKO);

    ret = prtS(buf);
    CHECK(ret > 0, lblKO);
    CHECK(strcmp(buf, "02-09-2004  16:47:00") == 0, lblKO);

    ret = prtS("FillTable...");
    CHECK(ret >= 0, lblKO);
    ret = tabPutStr(&tab, 3, "20040905203900"); //foreground string
    CHECK(ret > 0, lblKO);

    ret = dspLS(2, "cmbGet...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("cmbGet...");
    CHECK(ret >= 0, lblKO);
    ret = cmbGet(&cmb, 3, buf, prtW + 1);   //build combo using row 3
    CHECK(ret > 0, lblKO);

    ret = prtS(buf);
    CHECK(ret > 0, lblKO);
    CHECK(strcmp(buf, "05-09-2004  20:39:00") == 0, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    return ret;
}

static int testPad(void) {
    int ret;
    tTable tab;
    tContainer cnt;
    tCombo cmb;
    tLocator loc[3];
    char buf[prtW + 1];
    char volDba[prtW * 2];

    ret = prtS("==== Vol. mem. SRAM ====");
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "dbaSetPtr...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("dbaSetPtr...");
    CHECK(ret >= 0, lblKO);

    memset(volDba, 0, sizeof volDba);
    dbaSetPtr(&volDba);

    ret = dspLS(1, "tabInit...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("Vol tabInit...");
    CHECK(ret >= 0, lblKO);
    ret = prtS(" rows:2 len:16 ");

    ret = tabInit(&tab, 0xFF, 0, prtW, 2, 0);   //initiate a table in volatile memory of two rows of width dspW
    VERIFY(ret == (int) tabSize(&tab));

    tmrPause(1);

    ret = dspLS(2, "cntInit...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("cntInit...");
    CHECK(ret >= 0, lblKO);
    ret = cntInit(&cnt, 't', &tab);
    CHECK(ret > 0, lblKO);

    tmrPause(1);

    loc[0].cnt = &cnt;
    loc[0].key = 0;
    loc[1].cnt = 0;             //padding type is transferred by value
    loc[1].key = 'L';
    loc[2].cnt = &cnt;
    loc[2].key = 1;
    ret = dspLS(3, "cmbInit...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("==== cmbInit oprPad =====");
    CHECK(ret >= 0, lblKO);
    ret = prtS("FillTable...");
    CHECK(ret >= 0, lblKO);
    ret = tabPutStr(&tab, 0, "0.001");
    CHECK(ret > 0, lblKO);      //original string
    ret = prtS("Row0=\"0.001\"");
    CHECK(ret >= 0, lblKO);
    ret = tabPutStr(&tab, 1, "            0.12");
    CHECK(ret > 0, lblKO);      //control sample string
    ret = prtS("Row1=\"           x.xxx\"");
    CHECK(ret >= 0, lblKO);

    ret = cmbInit(&cmb, oprPad, loc, prtW + 1);
    CHECK(ret > 0, lblKO);
    tmrPause(1);

    ret = dspLS(2, "cmbGet...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("cmbGet...");
    CHECK(ret >= 0, lblKO);

    ret = cmbGet(&cmb, 0, buf, prtW + 1);   //build combo using row 0
    CHECK(ret > 0, lblKO);

    ret = prtS(buf);
    CHECK(ret > 0, lblKO);
    CHECK(strcmp(buf, "           0.001") == 0, lblKO);
    ret = 1;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    return ret;
}

static int testAmt(void) {
    int ret;

#define lenAmt 16
    enum eRec {                 //record fields
        recAmt,
        recExp,
        recSep,
        recEnd                  //end sentinel
    };
    typedef struct sTestAmt {
        char amt[lenAmt];
        byte exp;
        char sep[3];
    } tTestAmt;
    tRecord rec;
    word recLen[recEnd];        //contains field offsets
    word recMap[recEnd];        //contains field offsets
    tTestAmt volDba;

    tContainer cnt;
    tCombo cmb;
    tLocator loc[3];
    char buf[prtW + 1];

    ret = prtS("==== Vol. mem. SRAM ====");
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "dbaSetPtr...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("dbaSetPtr...");
    CHECK(ret >= 0, lblKO);

    memset(&volDba, 0, sizeof(volDba));
    dbaSetPtr(&volDba);

    memset(recLen, 0, sizeof recLen);
    //fill lengths; offset will be filled by recInit
    recLen[recAmt] = lenAmt;
    recLen[recExp] = sizeof(byte);
    recLen[recSep] = 2;
    memset(recMap, 0, sizeof recMap);

    ret = dspLS(1, "recInit...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("Vol recInit...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("Str(16) Byte Str(2)");
    ret = recInit(&rec, 0xFF, 0, recEnd, recLen, 0, recMap);    //initiate a record in volatile memory
    CHECK(ret > 0, lblKO);

    tmrPause(1);

    ret = dspLS(2, "cntInit...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("cntInit...");
    CHECK(ret >= 0, lblKO);

    ret = cntInit(&cnt, 'r', &rec);
    CHECK(ret > 0, lblKO);

    tmrPause(1);

    //only two arguments, so the last locator is not used
    loc[0].cnt = &cnt;
    loc[0].key = recAmt;        //source amount string
    loc[1].cnt = &cnt;
    loc[1].key = recExp;        //exponent
    loc[2].cnt = 0;             //default argument is "."
    loc[2].key = 0;

    ret = dspLS(3, "cmbInit...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("==== cmbInit oprSbs =====");
    CHECK(ret >= 0, lblKO);

    ret = prtS("FillRecord...");
    CHECK(ret >= 0, lblKO);
    ret = recPutStr(&rec, recAmt, "1");
    CHECK(ret > 0, lblKO);      //source amount string
    ret = prtS("Amt=\"1\"");
    CHECK(ret >= 0, lblKO);
    ret = recPutByte(&rec, recExp, 3);
    CHECK(ret > 0, lblKO);      //exponent
    ret = prtS("Exp=3");
    CHECK(ret >= 0, lblKO);
    ret = cmbInit(&cmb, oprAmt, loc, prtW + 1);
    CHECK(ret > 0, lblKO);

    tmrPause(1);

    ret = dspLS(2, "cmbGet...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("cmbGet...");
    CHECK(ret >= 0, lblKO);

    ret = cmbGet(&cmb, 0, buf, prtW + 1);   //get amount formatted
    CHECK(ret > 0, lblKO);

    ret = prtS(buf);
    CHECK(ret > 0, lblKO);
    CHECK(strcmp(buf, "0.001") == 0, lblKO);

    //now with third argument
    loc[0].cnt = &cnt;
    loc[0].key = recAmt;        //source amount string
    loc[1].cnt = &cnt;
    loc[1].key = recExp;        //exponent
    loc[2].cnt = &cnt;
    loc[2].key = recSep;

    ret = dspLS(3, "cmbInit...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("==== cmbInit oprSbs =====");
    CHECK(ret >= 0, lblKO);

    ret = cmbInit(&cmb, oprAmt, loc, prtW + 1);
    CHECK(ret > 0, lblKO);

    ret = prtS("FillRecord...");
    CHECK(ret >= 0, lblKO);
    ret = recPutStr(&rec, recAmt, "123456789");
    CHECK(ret > 0, lblKO);      //source amount string
    ret = prtS("Amt=\"123456789\"");
    CHECK(ret >= 0, lblKO);
    ret = recPutByte(&rec, recExp, 2);
    CHECK(ret > 0, lblKO);      //exponent
    ret = prtS("Exp=2");
    CHECK(ret >= 0, lblKO);
    ret = recPutStr(&rec, recSep, ", ");
    CHECK(ret > 0, lblKO);      //separator string
    ret = prtS("Sep=\", \"");
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspLS(2, "cmbGet...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("cmbGet...");
    CHECK(ret >= 0, lblKO);

    ret = cmbGet(&cmb, 0, buf, prtW + 1);   //get amount formatted
    CHECK(ret > 0, lblKO);

    ret = prtS(buf);
    CHECK(ret > 0, lblKO);
    CHECK(strcmp(buf, "1 234 567,89") == 0, lblKO);

    ret = 1;
    goto lblEnd;

  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    return ret;
}

static int testSpr(void) {
    int ret;
    enum eRec {                 //record fields
        recFmt1,
        recFmt2,
        recStr,
        recNum1,
        recNum2,
        recEnd                  //end sentinel
    };
    typedef struct sTestSpr {
        char fmt1[prtW];
        char fmt2[prtW];
        char str[prtW];
        card num1;
        word num2;
    } tTestSpr;
    tRecord rec;
    word recLen[recEnd];        //contains field offsets
    word recMap[recEnd];        //contains field offsets
    tTestSpr volDba;

    tContainer cnt;
    tCombo cmb1, cmb2;
    tLocator loc1[3];
    tLocator loc2[3];
    char buf[prtW + 1];

    //Initialize pointer to volatile memory buffer
    memset(&volDba, 0, sizeof(volDba));
    dbaSetPtr(&volDba);

    ret = dspLS(1, "recInit...");
    CHECK(ret >= 0, lblKO);

    memset(recLen, 0, sizeof recLen);
    //fill lengths; offset will be filled by recInit
    recLen[recFmt1] = prtW;
    recLen[recFmt2] = prtW;
    recLen[recStr] = prtW;
    recLen[recNum1] = sizeof(card);
    recLen[recNum2] = sizeof(card);
    memset(recMap, 0, sizeof recMap);

    ret = recInit(&rec, 0xFF, 0, recEnd, recLen, 0, recMap);    //initiate a record in volatile memory
    CHECK(ret > 0, lblKO);

    tmrPause(1);

    //Create a container around the table
    ret = dspLS(2, "cntInit...");
    CHECK(ret >= 0, lblKO);
    ret = cntInit(&cnt, 'r', &rec);
    CHECK(ret > 0, lblKO);

    tmrPause(1);

    ret = dspLS(3, "cmbInit...");
    CHECK(ret >= 0, lblKO);

    //only two arguments, so the last locator is not used
    loc1[0].cnt = &cnt;
    loc1[0].key = recFmt1;      //format string
    loc1[1].cnt = &cnt;
    loc1[1].key = recStr;       //for %s specifier
    loc1[2].cnt = 0;
    loc1[2].key = 0;

    //more complicated, all arguments are used
    loc2[0].cnt = &cnt;
    loc2[0].key = recFmt2;      //format string
    loc2[1].cnt = &cnt;
    loc2[1].key = recNum1;      //for %d specifier
    loc2[2].cnt = &cnt;
    loc2[2].key = recNum2;      //for %04x specifier

    //Create combos for sprintf operation
    ret = cmbInit(&cmb1, oprSpr, loc1, prtW + 1);
    CHECK(ret > 0, lblKO);
    ret = cmbInit(&cmb2, oprSpr, loc2, prtW + 1);
    CHECK(ret > 0, lblKO);

    tmrPause(1);

    //Fill the record
    ret = recPutStr(&rec, recFmt1, "string= [%s]"); //format string
    CHECK(ret > 0, lblKO);
    ret = recPutStr(&rec, recStr, "ABC");   //string to be formatted
    CHECK(ret > 0, lblKO);

    ret = recPutStr(&rec, recFmt2, "dec= [%d] hex=[%04x]"); //format string
    CHECK(ret > 0, lblKO);
    ret = recPutCard(&rec, recNum1, 123);   //to be formatted as %d
    CHECK(ret > 0, lblKO);
    ret = recPutWord(&rec, recNum2, 0xABCD);    //to be formatted as %04x
    CHECK(ret > 0, lblKO);

    //Test the first combo
    ret = cmbGet(&cmb1, NULLKEY, buf, prtW);    //NULLKEY means: take the key from combo location array
    CHECK(ret > 0, lblKO);

    //Show the result
    prtS(buf);
    CHECK(strcmp(buf, "string= [ABC]") == 0, lblKO);

    //Test the second combo
    ret = cmbGet(&cmb2, NULLKEY, buf, prtW);
    CHECK(ret > 0, lblKO);

    //Show the result
    prtS(buf);
    CHECK(strcmp(buf, "dec= [123] hex=[abcd]") == 0, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    tmrPause(2);
    return ret;
}

static int testAddSub(void) {
    int ret;
    tTable tab;
    tContainer cnt;
    tCombo cmbAdd, cmbSub;
    tLocator loc[3];
    char buf[prtW + 1];
    char volDba[prtW * 2];

    memset(volDba, 0, sizeof volDba);
    dbaSetPtr(&volDba);

    ret = dspLS(1, "tabInit...");
    CHECK(ret >= 0, lblKO);

    ret = tabInit(&tab, 0xFF, 0, prtW, 2, 0);   //initiate a table in volatile memory of two rows of width dspW
    VERIFY(ret == (int) tabSize(&tab));

    tmrPause(1);

    ret = dspLS(2, "cntInit...");
    CHECK(ret >= 0, lblKO);
    ret = cntInit(&cnt, 't', &tab);
    CHECK(ret > 0, lblKO);

    tmrPause(1);

    ret = dspLS(3, "cmbInit...");
    CHECK(ret >= 0, lblKO);

    loc[0].cnt = &cnt;
    loc[0].key = NULLKEY;       //combo will be applied to any row
    loc[1].cnt = &cnt;
    loc[1].key = 1;
    loc[2].cnt = 0;
    loc[2].key = 0;

    ret = cmbInit(&cmbAdd, oprAdd, loc, prtW + 1);
    CHECK(ret > 0, lblKO);
    ret = cmbInit(&cmbSub, oprSub, loc, prtW + 1);
    CHECK(ret > 0, lblKO);

    tmrPause(1);

    ret = tabPutStr(&tab, 0, "321");    //first argument
    CHECK(ret > 0, lblKO);

    ret = tabPutStr(&tab, 1, "123");    //second argument
    CHECK(ret > 0, lblKO);

    ret = cmbGet(&cmbAdd, 0, buf, prtW + 1);
    CHECK(ret > 0, lblKO);

    ret = prtS(buf);
    CHECK(ret > 0, lblKO);
    CHECK(strcmp(buf, "444") == 0, lblKO);

    ret = cmbGet(&cmbSub, 0, buf, prtW + 1);
    CHECK(ret > 0, lblKO);

    ret = prtS(buf);
    CHECK(ret > 0, lblKO);
    CHECK(strcmp(buf, "198") == 0, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    return ret;
}

static int testIf(void) {
    int ret;
    tContainer cntCnd, cntTrue, cntFalse;
    tTable tab;
    tLocator loc[3];
    tCombo cmbIf;
    byte tmp;
    char buf[dspW + 1];
    char volDba[sizeof(byte) * 2];

    //memset(volDba, 0, prtW * 4);
    memset(volDba, 0, sizeof volDba);
    dbaSetPtr(&volDba);

    ret = tabInit(&tab, 0xFF, 0, sizeof(byte), 2, 0);
    VERIFY(ret == (int) tabSize(&tab));
    tmp = 0;
    ret = tabPut(&tab, 0, &tmp, sizeof(tmp));
    CHECK(ret > 0, lblKO);
    tmp = 1;
    ret = tabPut(&tab, 1, &tmp, sizeof(tmp));
    CHECK(ret > 0, lblKO);
    ret = cntInit(&cntCnd, 't', &tab);
    ret = cntInit(&cntTrue, 's', "True");
    CHECK(ret > 0, lblKO);
    ret = cntInit(&cntFalse, 's', "False");
    CHECK(ret > 0, lblKO);

    memset(loc, 0, sizeof(loc));
    loc[0].cnt = &cntCnd;
    loc[0].key = NULLKEY;       //combo will be applied to any row
    loc[1].cnt = &cntTrue;
    loc[1].key = 0;
    loc[2].cnt = &cntFalse;
    loc[2].key = 0;
    ret = cmbInit(&cmbIf, oprIf, loc, (byte) dspW + 1);
    CHECK(ret > 0, lblKO);

    ret = cmbGet(&cmbIf, 0, buf, dspW);
    CHECK(strcmp(buf, "False") == 0, lblKO);

    ret = cmbGet(&cmbIf, 1, buf, dspW);
    CHECK(strcmp(buf, "True") == 0, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    return ret;
}

static int testEq(void) {
    int ret;
    tContainer cntLft, cntRgt;
    tLocator loc[3];
    tCombo cmbEq;
    byte rsp;
    char volDba[prtW * 4];

    memset(volDba, 0, sizeof volDba);
    dbaSetPtr(&volDba);

    memset(loc, 0, sizeof(loc));
    loc[0].cnt = &cntLft;
    loc[0].key = 0;
    loc[1].cnt = &cntRgt;
    loc[1].key = 0;
    loc[2].cnt = 0;
    loc[2].key = 0;
    ret = cmbInit(&cmbEq, oprEq, loc, (byte) dspW + 1);
    CHECK(ret > 0, lblKO);

    ret = cntInit(&cntLft, 's', "Lalala");
    CHECK(ret > 0, lblKO);
    ret = cntInit(&cntRgt, 's', "Lalala");
    CHECK(ret > 0, lblKO);
    ret = cmbGet(&cmbEq, 0, &rsp, 1);
    CHECK(rsp == 1, lblKO);

    ret = cntInit(&cntLft, 's', "Lalala");
    CHECK(ret > 0, lblKO);
    ret = cntInit(&cntRgt, 's', "Lululu");
    CHECK(ret > 0, lblKO);
    ret = cmbGet(&cmbEq, 0, &rsp, 1);
    CHECK(rsp == 0, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    return ret;
}

void tcab0033(void) {
    int ret;

    trcS("tcab0033 Beg\n");

    nvmStart();                 //initialise non-volatile memory
    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(3, "Combo Amt...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("****** Combo Amt *******");
    CHECK(ret >= 0, lblKO);
    ret = testAmt();
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "Combo Eq...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("****** Combo Eq *******");
    CHECK(ret >= 0, lblKO);

    ret = testEq();
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "Combo Sbs...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("****** Combo Sbs *******");
    CHECK(ret >= 0, lblKO);
    ret = testSbs();
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(1, "Combo Mut...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("****** Combo Mut *******");
    CHECK(ret >= 0, lblKO);
    ret = testMut();
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(2, "Combo Pad...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("****** Combo Pad *******");
    CHECK(ret >= 0, lblKO);
    ret = testPad();
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "Combo Spr...");
    CHECK(ret >= 0, lblKO);
    ret = testSpr();
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "Combo Add Sub...");
    CHECK(ret >= 0, lblKO);
    ret = testAddSub();
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "Combo If...");
    CHECK(ret >= 0, lblKO);
    ret = testIf();
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    goto lblEnd;

  lblKO:
    dspLS(0, "KO");
  lblEnd:
    tmrPause(3);
    dspStop();
    prtStop();                  //close resource
    trcS("tcab0033 End\n");
}
