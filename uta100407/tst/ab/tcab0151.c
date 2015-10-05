/** \file
 * Unitary test case tcab0151.
 * Functions testing:
 * \sa
 *  - tabExport()
 *  - recExport()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0151.c $
 *
 * $Id: tcab0151.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "pri.h"
#include "tst.h"

static int testTab(tTable * tab) {
    int ret;
    byte idx;

    VERIFY(tabSize(tab) == dspW * 3);
    VERIFY(tabDim(tab) == 3);

    ret = tabPutStr(tab, 0, "Row 0");   //save the string with ending zero into row 0
    CHECK(ret == (int) strlen("Row 0") + 1, lblKO);

    ret = tabPutStr(tab, 1, "Row 1");   //save the string with ending zero into row 1
    CHECK(ret == (int) strlen("Row 1") + 1, lblKO);

    ret = tabPutStr(tab, 2, "Row 2");   //save the string with ending zero into row 2
    CHECK(ret == (int) strlen("Row 2") + 1, lblKO);

    tmrPause(1);

    for (idx = 0; idx < 3; idx++) {
        ret = tabExport(tab, idx, 3, 'S', 0);
        CHECK(ret > 0, lblKO);
    }

    ret = 1;
    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
    tmrPause(3);
    ret = -1;
  lblEnd:
    return ret;
}

enum eRec {                     //record fields
    recBeg,                     //start sentinel
    recByte,
    recWord,
    recCard,
    recStr,
    recEnd                      //end sentinel
};

static int testRec(tRecord * rec) {
    int ret;

    VERIFY(recDim(rec) == recEnd);

    ret = recPutByte(rec, recByte, 0x12);
    CHECK(ret == 1, lblKO);

    ret = recPutWord(rec, recWord, 0x1234);
    CHECK(ret == 2, lblKO);

    ret = recPutCard(rec, recCard, 0x12345678L);
    CHECK(ret == 4, lblKO);

    ret = recPut(rec, recStr, "lalala", 0); //save string value with variable length
    CHECK(((word) ret) == strlen("lalala"), lblKO); //the ret can be less if the strlen is equal to field length

    ret = recExport(rec, recByte, 3, 'H', "byte");
    ret = recExport(rec, recWord, 3, 'D', "word");
    ret = recExport(rec, recCard, 3, 'H', "card");
    ret = recExport(rec, recStr, 3, 'S', "string");

    ret = 1;
    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
    tmrPause(3);
    ret = -1;
  lblEnd:
    return ret;
}

void tcab0151(void) {
    int ret;
    byte volDba[128];
    tTable tab;
    tRecord rec;
    word recLen[recEnd];        //contains field lengths
    word recMap[recEnd];        //contains field offsets

    trcS("tcab0151 Beg\n");

    nvmStart();                 //initialise non-volatile memory
    nvmRemove(3);               //don't check the return code; maybe the file does not exist

    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "dbaSetPtr...");
    memset(volDba, 0, 128);
    dbaSetPtr(volDba);          //the buffer volDba will be used for volatile section of data base

    tmrPause(1);

    ret = dspLS(1, "tabExport...");
    CHECK(ret >= 0, lblKO);
    ret = tabInit(&tab, 0xFF, 0, dspW, 3, 0);   //initiate a table in volatile memory of three rows of width dspW
    VERIFY(ret == (int) tabSize(&tab));

    ret = testTab(&tab);        //test volatile memory table
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "tabExport OK");
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspLS(2, "recExport...");
    CHECK(ret >= 0, lblKO);
    memset(recLen, 0, recEnd * sizeof(word));
    //fill lengths; offsets will be filled by recInit
    recLen[recByte] = sizeof(byte);
    recLen[recWord] = sizeof(word);
    recLen[recCard] = sizeof(card);
    recLen[recStr] = 16;
    memset(recMap, 0, recEnd * sizeof(word));
    ret = recInit(&rec, 0xFF, tabSize(&tab), recEnd, recLen, 0, recMap);    //initiate a record in volatile memory
    VERIFY(ret == (int) recSize(&rec));

    ret = testRec(&rec);        //watch volDba in the debugger window
    CHECK(ret >= 0, lblKO);
    ret = dspLS(2, "recExport OK");
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
    tmrPause(3);
  lblEnd:
    dspStop();                  //close resource
    trcS("tcab0151 End\n");
}
