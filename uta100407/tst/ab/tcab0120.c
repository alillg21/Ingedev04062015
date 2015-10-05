/** \file
 * Unitary test case tcab0120.
 * Linear search
 * \sa
 *  - linFind()
 *  - searchInit()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0120.c $
 *
 * $Id: tcab0120.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "pri.h"
#include "tst.h"

enum eFld {                     //multirecord fields
    fldByte,                    //field containing byte value
    fldWord,                    //field containing word value
    fldCard,                    //field containing card value
    fldStr,                     //field containing string value
    fldEnd                      //end sentinel
};

static tRecord rec;
static word fldLen[fldEnd] = {
    sizeof(byte),
    sizeof(word),
    sizeof(card),
    16
};
static char fldFmt[fldEnd] = {
    'B',
    'W',
    'C',
    'S'
};
static word fldMap[fldEnd];
static tTable tab;

static tContainer cnt;          //container built around rec

static int init(void) {
    int ret;
    word len;

    ret = dspLS(0, "init");
    CHECK(ret >= 0, lblKO);
    ret = prtS("init");
    CHECK(ret >= 0, lblKO);

    ret = recInit(&rec, 0, 0, fldEnd, fldLen, fldFmt, fldMap);
    VERIFY(ret == (int) recSize(&rec));
    len = ret;

    ret = tabInit(&tab, 0, 0, len, 512, 0);
    VERIFY(ret == (int) tabSize(&tab));

    ret = cntInit(&cnt, 'r', &rec);
    CHECK(ret >= 0, lblKO);

    ret = 1;

    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    ret = -1;
  lblEnd:
    tmrPause(1);
    return ret;
}

static int generate(void) {
    int ret;
    word idx;
    char buf[256];

    ret = dspLS(1, "generate...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("generate...");
    CHECK(ret >= 0, lblKO);

    for (idx = 0; idx < tabDim(&tab); idx++) {
        num2dec(buf, idx, 4);
        ret = dspLS(2, buf);
        CHECK(ret >= 0, lblKO);
        recMove(&rec, &tab, idx);

        ret = recPutByte(&rec, fldByte, (byte) (idx % 256));
        CHECK(ret >= 0, lblKO);
        ret = recPutWord(&rec, fldWord, idx);
        CHECK(ret >= 0, lblKO);
        ret = recPutCard(&rec, fldCard, sysRand());
        CHECK(ret >= 0, lblKO);
        ret = recPut(&rec, fldStr, buf, 4 + 1);
        CHECK(ret >= 0, lblKO);
    }
    ret = dspLS(2, "Done.");
    CHECK(ret >= 0, lblKO);
    ret = prtS("  Table of 512 records");
    CHECK(ret >= 0, lblKO);
    ret = prtS(" Byte Word Card Str(16) ");
    CHECK(ret >= 0, lblKO);
    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    ret = -1;
  lblEnd:
    tmrPause(1);
    return ret;
}

static int prtRec(void) {
    int ret;
    char tmp[prtW + 1];
    char buf[prtW + 1];
    byte b;
    word w;
    card c;

    ret = prtS("Record:");
    CHECK(ret >= 0, lblKO);

    ret = recGetByte(&rec, fldByte, b);
    CHECK(ret >= 0, lblKO);
    strcpy(buf, "Byte: ");
    num2hex(tmp, b, 2);
    strcat(buf, tmp);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    ret = recGetWord(&rec, fldWord, w);
    CHECK(ret >= 0, lblKO);
    strcpy(buf, "Word: ");
    num2dec(tmp, w, 0);
    strcat(buf, tmp);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    ret = recGetCard(&rec, fldCard, c);
    CHECK(ret >= 0, lblKO);
    strcpy(buf, "Card: ");
    num2dec(tmp, c, 0);
    strcat(buf, tmp);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    ret = recGet(&rec, fldStr, tmp, prtW + 1);
    CHECK(ret >= 0, lblKO);
    strcpy(buf, "Str: ");
    strcat(buf, tmp);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    ret = prtS(" === === ===");
    CHECK(ret >= 0, lblKO);

    ret = 1;

    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    ret = -1;
  lblEnd:
    return ret;
}

static int searchByte(void) {
    int ret;
    word idx;
    const char *pat = 0;
    tSearch lin;
    tConvert cvtByte;           //returns fldByte data element in hex format
    tContainer cntByte;         //container built around cvtByte

    ret = dspLS(0, "searchByte");
    CHECK(ret >= 0, lblKO);
    ret = prtS("****** searchByte ******");
    CHECK(ret >= 0, lblKO);

    ret = cvtInit(&cvtByte, &cnt, fldByte, fmtNum, fmtHex, 2);  //converts fldByte into hex format
    CHECK(ret >= 0, lblKO);

    ret = cntInit(&cntByte, 'c', &cvtByte); //to be used as matching pattern
    CHECK(ret >= 0, lblKO);

    ret = searchInit(&lin, &tab, &rec, &cntByte);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, "Search...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("Search...");
    CHECK(ret >= 0, lblKO);

    pat = "99";

    //Try find 99
    ret = linFind(&lin, pat, 0, tabDim(&tab));
    CHECK(ret >= 0, lblKO);
    CHECK(ret < tabDim(&tab), lblKO);

    idx = ret;
    if(idx < tabDim(&tab)) {
        ret = dspLS(2, "Found!");
        CHECK(ret >= 0, lblKO);
    } else {
        ret = dspLS(2, "Not Found.");
        CHECK(ret >= 0, lblKO);
        ret = prtS("Not Found.");
        CHECK(ret >= 0, lblKO);
        ret = prtS("   === === ===");
    }

    ret = prtS("SearchByte Pattern:");
    CHECK(ret >= 0, lblKO);

    ret = prtS(pat);
    CHECK(ret >= 0, lblKO);

    ret = prtRec();
    CHECK(ret >= 0, lblKO);

    tmrPause(1);

    //try once more
    ret = dspLS(2, "Search next 2...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("Search next 2...");
    CHECK(ret >= 0, lblKO);

    CHECK(idx < tabDim(&tab), lblKO);
    idx++;
    ret = linFind(&lin, pat, idx, tabDim(&tab));    //continue search
    CHECK(ret >= 0, lblKO);
    CHECK(ret < tabDim(&tab), lblKO);

    idx = ret;
    if(idx < tabDim(&tab)) {
        ret = dspLS(2, "Found!");
        CHECK(ret >= 0, lblKO);
    } else {
        ret = dspLS(2, "Not Found.");
        CHECK(ret >= 0, lblKO);
        ret = prtS("Not Found.");
        CHECK(ret >= 0, lblKO);
        ret = prtS("   === === ===");
    }

    ret = prtS("SearchByte Pattern:");
    CHECK(ret >= 0, lblKO);

    ret = prtS(pat);
    CHECK(ret >= 0, lblKO);

    ret = prtRec();
    CHECK(ret >= 0, lblKO);

    tmrPause(1);

    //try once more (should fail this time)
    ret = dspLS(2, "Search next 3...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("Search next 3...");
    CHECK(ret >= 0, lblKO);

    CHECK(idx < tabDim(&tab), lblKO);
    idx++;
    ret = linFind(&lin, pat, idx, tabDim(&tab));    //continue search
    CHECK(ret >= 0, lblKO);

    idx = ret;
    if(idx < tabDim(&tab)) {
        ret = dspLS(2, "Found!");
        CHECK(ret >= 0, lblKO);
    } else {
        ret = dspLS(2, "Not Found.");
        CHECK(ret >= 0, lblKO);
        ret = prtS("Not Found.");
        CHECK(ret >= 0, lblKO);
        ret = prtS("   === === ===");
    }

    CHECK(idx == tabDim(&tab), lblKO);
    tmrPause(1);

    //try backward search
    ret = dspLS(2, "Search back...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("Search backward...");
    CHECK(ret >= 0, lblKO);

    ret = linFind(&lin, pat, (word) (tabDim(&tab) - 1), 0); //continue search
    CHECK(ret >= 0, lblKO);
    CHECK(ret < tabDim(&tab), lblKO);

    idx = ret;
    if(idx < tabDim(&tab)) {
        ret = dspLS(2, "Found!");
        CHECK(ret >= 0, lblKO);
    } else {
        ret = dspLS(2, "Not Found.");
        CHECK(ret >= 0, lblKO);
        ret = prtS("Not Found.");
        CHECK(ret >= 0, lblKO);
        ret = prtS("   === === ===");
    }

    ret = prtS("SearchByte Pattern:");
    CHECK(ret >= 0, lblKO);

    ret = prtS(pat);
    CHECK(ret >= 0, lblKO);

    ret = prtRec();
    CHECK(ret >= 0, lblKO);

    ret = 1;

    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    ret = -1;
  lblEnd:
    tmrPause(1);
    return ret;
}

static int searchWordStr(void) {
    int ret;
    word idx;
    const char *pat = 0;
    tConvert cvtWord;           //returns fldWord data element in hex format
    tContainer cntWord;         //container built around cvtWord
    tCombo cmbWordStr;          //concatenates cvtWord and fldStr data elemetns
    tLocator arg[3];            //arguments for combo
    tContainer cntWordStr;      //container built around cvtWordStr
    tSearch lin;

    ret = dspLS(0, "searchWordStr");
    CHECK(ret >= 0, lblKO);
    ret = prtS("***** searchWordStr *****");
    CHECK(ret >= 0, lblKO);

    ret = cvtInit(&cvtWord, &cnt, fldWord, fmtNum, fmtHex, 4);  //converts fldWord into hex format
    CHECK(ret >= 0, lblKO);

    ret = cntInit(&cntWord, 'c', &cvtWord); //to be used as matching pattern
    CHECK(ret >= 0, lblKO);

    arg[0].cnt = &cntWord;
    arg[0].key = NULLKEY;
    arg[1].cnt = &cnt;
    arg[1].key = fldStr;
    arg[2].cnt = 0;
    arg[2].key = 0;
    ret = cmbInit(&cmbWordStr, oprCat, arg, prtW + 1);
    CHECK(ret >= 0, lblKO);

    ret = cntInit(&cntWordStr, 'x', &cmbWordStr);   //to be used as matching pattern
    CHECK(ret >= 0, lblKO);

    ret = searchInit(&lin, &tab, &rec, &cntWordStr);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, "Search...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("Search...");
    CHECK(ret >= 0, lblKO);

    pat = "01990409";           //0199: hex value of fldWord, 0409: value of fldStr

    //Try find pattern
    ret = linFind(&lin, pat, 0, tabDim(&tab));
    CHECK(ret >= 0, lblKO);
    CHECK(ret < tabDim(&tab), lblKO);

    idx = ret;
    if(idx < tabDim(&tab)) {
        ret = dspLS(2, "Found!");
        CHECK(ret >= 0, lblKO);
    } else {
        ret = dspLS(2, "Not Found.");
        CHECK(ret >= 0, lblKO);
        ret = prtS("Not Found.");
        CHECK(ret >= 0, lblKO);
    }

    ret = prtS("SearchWordStr Pattern:");
    CHECK(ret >= 0, lblKO);

    ret = prtS(pat);
    CHECK(ret >= 0, lblKO);
    ret = prtS("0x199 \"0409\"");
    CHECK(ret >= 0, lblKO);

    ret = prtRec();
    CHECK(ret >= 0, lblKO);

    tmrPause(1);

    ret = 1;

    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    ret = -1;
  lblEnd:
    tmrPause(1);
    return ret;
}

void tcab0120(void) {
    int ret;
    byte vol[256];

    nvmStart();
    dbaSetPtr(vol);

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = init();
    CHECK(ret >= 0, lblKO);
    ret = generate();
    CHECK(ret >= 0, lblKO);

    ret = searchByte();
    CHECK(ret >= 0, lblKO);
    ret = searchWordStr();
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    tmrPause(3);
  lblEnd:
    prtStop();
    dspStop();
}
