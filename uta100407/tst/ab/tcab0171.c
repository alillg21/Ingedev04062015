#include <string.h>
#include "pri.h"
#include "tst.h"

//Unitary test case tcab0171.
//idxSort test

#define DIM 512

enum eFld {                     //multirecord fields
    fldCard,                    //field containing card value
    fldStr,                     //field containing string value
    fldEnd                      //end sentinel
};

static tRecord rec;
static word fldLen[fldEnd] = {
    sizeof(card),
    8
};
static word fldMap[fldEnd];
static tTable tab;

static tContainer cnt;          //container built around rec

static int init(void) {
    int ret;
    word len;

    ret = dspLS(0, "init");
    CHECK(ret >= 0, lblKO);

    ret = recInit(&rec, 0, 0, fldEnd, fldLen, fldMap);
    VERIFY(ret == (int) recSize(&rec));
    len = ret;

    ret = tabInit(&tab, 0, 0, len, DIM);
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
    card num;
    char buf[256];

    ret = dspLS(0, "generate...");
    CHECK(ret >= 0, lblKO);

    for (idx = 0; idx < tabDim(&tab); idx++) {
        num2dec(buf, idx, 4);
        ret = dspLS(1, buf);
        CHECK(ret >= 0, lblKO);
        recMove(&rec, &tab, idx);

        //the random numbers should be unique
        num = sysRand() / DIM;
        //num%= DIM; //to be removed
        num *= DIM;
        num += idx;             //now we are sure that there is no duplicate
        ret = recPutCard(&rec, fldCard, num);
        CHECK(ret >= 0, lblKO);
        num2hex(buf, num, 8);
        ret = recPutStr(&rec, fldStr, buf);
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

    ret = dspClear();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "buildIndex...");
    CHECK(ret >= 0, lblKO);

    ret = cvtInit(&cvtCard, &cnt, fldCard, fmtNum, fmtHex, 10); //converts fldCard into hex format
    CHECK(ret >= 0, lblKO);

    ret = cntInit(&cntCard, 'c', &cvtCard); //to be used as matching pattern
    CHECK(ret >= 0, lblKO);

    ret = idxInit(idx, 1, ofs, &tab, &rec, &cntCard, num, 10, 0);
    CHECK(ret >= 0, lblKO);

    if(how == 'q') {
        for (pos = 0; pos < tabDim(&tab); pos++) {
            //idx->idx[pos]= pos;
            //idx->dim++;
            num2dec(buf, pos, 4);
            ret = dspLS(1, buf);
            CHECK(ret >= 0, lblKO);
            recMove(&rec, &tab, pos);
            ret = idxApp(idx);
            CHECK(ret >= 0, lblKO);
        }
        ret = idxSort(idx);
        CHECK(ret >= 0, lblKO);
        num2dec(buf, (word) ret, 4);
        ret = dspLS(1, buf);
        CHECK(ret >= 0, lblKO);
        ret = prtS("sort swaps:");
        CHECK(ret >= 0, lblKO);
        prtS(buf);
        CHECK(ret >= 0, lblKO);
    } else {                    //insertions
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

static int browseCard(tIndex * idx) {
    int ret;
    word pos;
    card c0, c1;
    char buf[dspW + 1];
    char tmp[prtW + 1];

    ret = dspLS(0, "browseCard");
    CHECK(ret >= 0, lblKO);

    c0 = 0;
    c1 = 0;
    for (pos = 0; pos < idxDim(idx); pos++) {
        ret = idxSet(idx, pos);
        CHECK(ret >= 0, lblKO);

        c0 = c1;
        ret = recGetCard(&rec, fldCard, c1);
        CHECK(ret >= 0, lblKO);

        num2dec(buf, pos, 3);
        ret = dspLS(1, buf);
        CHECK(ret >= 0, lblKO);

        strcpy(tmp, buf);
        strcat(tmp, ": ");

        num2hex(buf, c1, 8);
        ret = dspLS(2, buf);
        CHECK(ret >= 0, lblKO);

        strcat(tmp, buf);
#ifdef WIN32
        prtS(tmp);
#endif

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

static void prtDT(const char *msg) {
    char tmp[prtW + 1];
    char buf[prtW + 1];

    strcpy(buf, msg);
    getDateTime(tmp);
    strcat(buf, tmp);
    prtS(buf);
}

void tcab0171(void) {
    int ret;
    byte vol[256];
    tIndex idx;

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

    prtDT("Q beg");
    ret = buildIndexCard(&idx, 0, 'q');
    CHECK(ret >= 0, lblKO);
    prtDT("Q end");
    ret = browseCard(&idx);
    CHECK(ret >= 0, lblKO);

    prtDT("I beg");
    ret = buildIndexCard(&idx, 0, 'i');
    CHECK(ret >= 0, lblKO);
    prtDT("I end");
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
