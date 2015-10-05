/** \file
 * Unitary test case tcab0121.
 * Binary search
 * \sa
 *  - binFind()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0121.c $
 *
 * $Id: tcab0121.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "pri.h"
#include "tst.h"

enum eFld {                     //multirecord fields
    fldWord,                    //field containing word value
    fldCard,                    //field containing card value
    fldStr,                     //field containing string value
    fldEnd                      //end sentinel
};

static tRecord rec;
static word fldLen[fldEnd] = {
    sizeof(word),
    sizeof(card),
    16
};
static char fldFmt[fldEnd] = {
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

        ret = recPutWord(&rec, fldWord, (word) (idx * 2));  //sorted by this field
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
    ret = prtS("    Word Card Str(16) ");
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
    word w;
    card c;

    ret = prtS("Record:");
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

static int findPat(tSearch * bin, word pat) {
    int ret;
    int idx;
    char buf[4 + 1];
    char tmp[prtW + 1];

    num2hex(buf, pat, 4);
    ret = dspLS(3, buf);

    ret = binFind(bin, buf, 0, tabDim(&tab));
    idx = ret;
    CHECK(ret >= 0, lblKO);
    //CHECK(ret < tabDim(&tab), lblKO);

    if(ret < tabDim(&tab)) {
        ret = dspLS(0, "Found!");
    } else {
        ret = dspLS(0, "Not Found.");
    }
    CHECK(ret >= 0, lblKO);

    ret = prtS("SearchWord Pattern:");
    CHECK(ret >= 0, lblKO);

    strcpy(tmp, "0x");
    strcpy(&tmp[2], buf);
    ret = prtS(tmp);
    CHECK(ret >= 0, lblKO);

    if(idx < tabDim(&tab))
        ret = prtRec();
    else
        ret = prtS("Not found");
    CHECK(ret >= 0, lblKO);

    ret = idx;
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    return ret;
}

static int searchWord(void) {
    int ret;
    tSearch bin;
    tConvert cvtWord;           //returns fldWord data element in hex format
    tContainer cntWord;         //container built around cvtWord

    ret = dspLS(0, "searchWord");
    CHECK(ret >= 0, lblKO);
    ret = prtS("****** searchWord ******");
    CHECK(ret >= 0, lblKO);

    ret = cvtInit(&cvtWord, &cnt, fldWord, fmtNum, fmtHex, 4);  //converts fldWord into hex format
    CHECK(ret >= 0, lblKO);

    ret = cntInit(&cntWord, 'c', &cvtWord); //to be used as matching pattern
    CHECK(ret >= 0, lblKO);

    ret = searchInit(&bin, &tab, &rec, &cntWord);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, "Search...");
    CHECK(ret >= 0, lblKO);

    ret = findPat(&bin, 0xAA);
    CHECK(ret == 0xAA / 2, lblKO);
    tmrPause(1);

    ret = findPat(&bin, 0x00);
    CHECK(ret == 0, lblKO);
    tmrPause(1);

    ret = findPat(&bin, (word) (2 * (tabDim(&tab) - 1)));
    CHECK(ret == tabDim(&tab) - 1, lblKO);
    tmrPause(1);

    ret = findPat(&bin, 11);    //not found
    CHECK(ret == tabDim(&tab), lblKO);
    tmrPause(1);

    ret = 1;
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    return ret;
}

void tcab0121(void) {
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

    ret = searchWord();
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    tmrPause(3);
  lblEnd:
    prtStop();
    dspStop();
}
