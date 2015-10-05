/** \file
 * Unitary test case tcab0122.
 * Index search
 * \sa
 *  - idxInit()
 *  - idxDim()
 *  - idxFind()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0122.c $
 *
 * $Id: tcab0122.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "pri.h"
#include "tst.h"

#define DIM 512

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
static word fldMap[fldEnd];
static tTable tab;

static tContainer cnt;          //container built around rec

static int init(void) {
    int ret;
    word len;

    ret = dspLS(0, "init");
    CHECK(ret >= 0, lblKO);

    ret = recInit(&rec, 0, 0, fldEnd, fldLen, 0, fldMap);
    VERIFY(ret == (int) recSize(&rec));
    len = ret;

    ret = tabInit(&tab, 0, 0, len, DIM, 0);
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

static void dbgTrcDat(void) {
    struct sRow {
        word w;
        card c;
        char s[16 + 1];
    } rows[DIM];
    word idx;

    dspLS(0, "Reading Dat...");
    trcS("dbgTrcDat Beg\n");
    for (idx = 0; idx < DIM; idx++) {
        recMove(&rec, &tab, idx);
        recGetWord(&rec, fldWord, rows[idx].w);
        recGetCard(&rec, fldCard, rows[idx].c);
        recGet(&rec, fldStr, rows[idx].s, 16);
    }
    dspLS(1, "Tracing Dat...");
    for (idx = 0; idx < DIM; idx++) {
        trcFN("[%03d]: ", idx);
        trcFN(" w=%04X", rows[idx].w);
        trcFN("(%03d)", rows[idx].w);
        trcFN(" c=%08X", rows[idx].c);
        trcFS(" s=%s\n", rows[idx].s);
    }
    trcS("dbgTrcDat End\n");
}

static void dbgTrcIdx(tIndex * idx) {
    word i;

    trcS("dbgTrcIdx Beg\n");
    dspLS(2, "Tracing Idx...");
    for (i = 0; i < DIM; i++) {
        trcFN("[%03d]: ", i);
        trcFN(" idx=%03d\n", idx->idx[i]);
    }
    trcS("dbgTrc Idx End\n");
}

static int generate(void) {
    int ret;
    word idx;
    char buf[256];

    ret = dspLS(0, "generate...");
    CHECK(ret >= 0, lblKO);

    //put even fldWord at the top half of multirecrod
    for (idx = 0; idx < tabDim(&tab) / 2; idx++) {
        num2dec(buf, idx, 4);
        ret = dspLS(1, buf);
        CHECK(ret >= 0, lblKO);
        recMove(&rec, &tab, idx);

        ret = recPutWord(&rec, fldWord, (word) (idx * 2));  //sorted by this field
        CHECK(ret >= 0, lblKO);
        ret = sysRand() % 10000;
        ret *= 1000;
        ret += idx;
        ret = recPutCard(&rec, fldCard, ret);
        CHECK(ret >= 0, lblKO);
        ret = recPut(&rec, fldStr, buf, 4 + 1);
        CHECK(ret >= 0, lblKO);
    }

    //put odd fldWord at the bottom half of multirecord
    for (idx = 0; idx < tabDim(&tab) / 2; idx++) {
        num2dec(buf, idx, 4);
        ret = dspLS(1, buf);
        CHECK(ret >= 0, lblKO);
        recMove(&rec, &tab, (word) (tabDim(&tab) / 2 + idx));

        ret = recPutWord(&rec, fldWord, (word) (idx * 2 + 1));  //sorted by this field
        CHECK(ret >= 0, lblKO);
        ret = sysRand() % 10000;
        ret *= 1000;
        ret += tabDim(&tab) / 2;
        ret += idx;
        ret = recPutCard(&rec, fldCard, ret);
        CHECK(ret >= 0, lblKO);
        ret = recPut(&rec, fldStr, buf, 4 + 1);
        CHECK(ret >= 0, lblKO);
    }
    ret = dspLS(1, "Done.");
    CHECK(ret >= 0, lblKO);
    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    ret = -1;
  lblEnd:
    tmrPause(1);
    return ret;
}

static int buildIndexWord(tIndex * idx, card ofs, char how) {
    int ret;
    word pos;
    char buf[256];
    static word num[DIM];
    static tConvert cvtWord;    //returns fldWord data element in hex format
    static tContainer cntWord;  //container built around cvtWord

    VERIFY(idx);

    ret = dspLS(0, "buildIndex...");
    CHECK(ret >= 0, lblKO);

    ret = cvtInit(&cvtWord, &cnt, fldWord, fmtNum, fmtHex, 4);  //converts fldWord into hex format
    CHECK(ret >= 0, lblKO);

    ret = cntInit(&cntWord, 'c', &cvtWord); //to be used as matching pattern
    CHECK(ret >= 0, lblKO);

    ret = idxInit(idx, 1, ofs, &tab, &rec, &cntWord, num, 4, 0);
    CHECK(ret >= 0, lblKO);

    if(how == 'l') {
        ret = idxLoad(idx);
        CHECK(ret >= 0, lblKO);
    } else {
        ret = idxReset(idx);
        CHECK(ret >= 0, lblKO);

        for (pos = 0; pos < tabDim(&tab); pos++) {
            num2dec(buf, pos, 4);
            ret = dspLS(1, buf);
            CHECK(ret >= 0, lblKO);
            recMove(&rec, &tab, pos);
            ret = idxIns(idx);
            CHECK(ret >= 0, lblKO);
        }
        ret = idxSave(idx);
        CHECK(ret >= 0, lblKO);
    }
    ret = dspLS(1, "Done.");
    CHECK(ret >= 0, lblKO);
    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    ret = -1;
  lblEnd:
    tmrPause(1);
    return ret;
}

static int buildIndexCard(tIndex * idx, card ofs, char how) {
    int ret;
    word pos;
    char buf[256];
    static word num[DIM];
    static tConvert cvtCard;    //returns fldCard data element in dec format
    static tContainer cntCard;  //container built around cvtCard

    VERIFY(idx);

    ret = dspLS(0, "buildIndex...");
    CHECK(ret >= 0, lblKO);

    ret = cvtInit(&cvtCard, &cnt, fldCard, fmtNum, fmtDec, 10); //converts fldCard into dec format
    CHECK(ret >= 0, lblKO);

    ret = cntInit(&cntCard, 'c', &cvtCard); //to be used as matching pattern
    CHECK(ret >= 0, lblKO);

    ret = idxInit(idx, 1, ofs, &tab, &rec, &cntCard, num, 10, 0);
    CHECK(ret >= 0, lblKO);

    if(how == 'l') {
        ret = idxLoad(idx);
        CHECK(ret >= 0, lblKO);
    } else {
        ret = idxReset(idx);
        CHECK(ret >= 0, lblKO);

        for (pos = 0; pos < tabDim(&tab); pos++) {
            num2dec(buf, pos, 4);
            ret = dspLS(1, buf);
            CHECK(ret >= 0, lblKO);
            recMove(&rec, &tab, pos);
            ret = idxIns(idx);
            CHECK(ret >= 0, lblKO);
        }
        ret = idxSave(idx);
        CHECK(ret >= 0, lblKO);
    }
    ret = dspLS(1, "Done.");
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

static int searchWord(tIndex * idx) {
    int ret;
    word pos;
    const char *pat = 0;

    ret = dspLS(0, "searchWord");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "Find...");
    CHECK(ret >= 0, lblKO);

    //Try find 00AA
    pat = "00AA";
    ret = idxFind(idx, pat, 0, tabDim(&tab));
    CHECK(ret >= 0, lblKO);
    CHECK(ret < tabDim(&tab), lblKO);

    pos = ret;
    if(pos < tabDim(&tab)) {
        ret = dspLS(1, "Found!");
    } else {
        ret = dspLS(1, "Not Found.");
    }
    CHECK(ret >= 0, lblKO);

    ret = prtS("SearchWord Pattern:");
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

static int browseCard(tIndex * idx) {
    int ret;
    word pos;
    card c0, c1;
    char buf[dspW + 1];

    ret = dspLS(0, "browseCard");
    CHECK(ret >= 0, lblKO);

    c0 = 0;
    c1 = 0;
    for (pos = 0; pos < idxDim(idx); pos++) {
        ret = idxSet(idx, pos);
        CHECK(ret >= 0, lblKO);

        //ret= prtRec();
        //CHECK(ret>=0,lblKO);

        c0 = c1;
        ret = recGetCard(&rec, fldCard, c1);
        CHECK(ret >= 0, lblKO);

        num2dec(buf, pos, 3);
        ret = dspLS(1, buf);
        CHECK(ret >= 0, lblKO);

        num2dec(buf, c1, 0);
        ret = dspLS(2, buf);
        CHECK(ret >= 0, lblKO);

        if(!pos)
            continue;
        CHECK(c0 < c1, lblKO);
    }

    ret = 1;
    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    ret = -1;
  lblEnd:
    tmrPause(1);
    dspClear();
    return ret;
}

void tcab0122(void) {
    int ret;
    byte vol[256];
    tIndex idx;
    card ofs;

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
    //for(ret= 0; ret<tabDim(&tab); ret++){
    //  recMove(&rec,&tab,ret);
    //  prtRec();
    //}

    dbgTrcDat();

    ofs = 0;
    ret = buildIndexWord(&idx, ofs, 'g');
    CHECK(ret >= 0, lblKO);
    dbgTrcIdx(&idx);

    ret = searchWord(&idx);
    CHECK(ret >= 0, lblKO);

    ofs += idxSize(&idx);
    ret = buildIndexCard(&idx, ofs, 'g');
    CHECK(ret >= 0, lblKO);
    ret = browseCard(&idx);
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    tmrPause(3);
  lblEnd:
    prtStop();
    dspStop();
}
