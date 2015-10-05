/** \file
 * Unitary test case tcab0030.
 * Structure sRecord methods testing
 * \sa
 *  - recReset()
 *  - recSize()
 *  - recFlush()
 *  - recFldFmt()
 *  - recFldLen()
 *  - recPut()
 *  - recGet()
 *  - recFlush()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0030.c $
 *
 * $Id: tcab0030.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "pri.h"
#include "tst.h"

enum eRec {                     //record fields
    recBeg,                     //start sentinel
    recByte,
    recWord,
    recCard,
    recStr,
    recEnd                      //end sentinel
};

typedef struct sSomeData {
    byte aByte;
    word aWord;
    card aCard;
    char aStr[16];
} tSomeData;

#define L_VOL sizeof(tSomeData)

static int testRec(tRecord * rec) {
    int ret;
    tSomeData buf;
    char tmp[16 + 1];

    VERIFY(recDim(rec) == recEnd);

    ret = dspLS(2, "recReset...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("recReset...");
    CHECK(ret >= 0, lblKO);

    ret = recReset(rec);        //fill by zeroes the memory reserved for the record
    VERIFY(ret == (int) recSize(rec));

    ret = recSize(rec);         //get size of the memory reserved for the record
    VERIFY(ret <= sizeof(buf)); //it is equal under Visual where the structure is packed, and less when structure is aligned

    ret = recFlush(rec);
    CHECK(ret == 0 || ret == (int) recSize(rec), lblKO);

    tmrPause(1);

    ret = dspLS(3, "recPutByte...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("recPutByte...");
    CHECK(ret >= 0, lblKO);

    ret = recFldFmt(rec, recByte);
    CHECK(ret == 'B', lblKO);

    ret = recFldLen(rec, recByte);
    CHECK(ret == 1, lblKO);

    ret = recPutByte(rec, recByte, 0x12);
    CHECK(ret == 1, lblKO);
    ret = prtS("0x12");
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspLS(0, "recPutWord...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("recPutWord...");
    CHECK(ret >= 0, lblKO);

    ret = recFldFmt(rec, recWord);
    CHECK(ret == 'W', lblKO);

    ret = recFldLen(rec, recWord);
    CHECK(ret == 2, lblKO);

    ret = recPutWord(rec, recWord, 0x1234);
    CHECK(ret == 2, lblKO);
    ret = prtS("0x1234");
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspLS(1, "recPutCard...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("recPutCard...");
    CHECK(ret >= 0, lblKO);

    ret = recFldFmt(rec, recCard);
    CHECK(ret == 'C', lblKO);

    ret = recFldLen(rec, recCard);
    CHECK(ret == 4, lblKO);

    ret = recPutCard(rec, recCard, 0x12345678L);
    CHECK(ret == 4, lblKO);
    ret = prtS("0x12345678");
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspLS(2, "recPut String...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("recPut String...");
    CHECK(ret >= 0, lblKO);

    ret = recFldFmt(rec, recStr);
    CHECK(ret == 'S', lblKO);

    ret = recFldLen(rec, recStr);
    CHECK(ret == 16, lblKO);

    ret = recPut(rec, recStr, "lalala", 0); //save string value with variable length
    CHECK(((word) ret) == strlen("lalala"), lblKO); //the ret can be less if the strlen is equal to field length
    ret = prtS("\"lalala\"");
    CHECK(ret >= 0, lblKO);

    tmrPause(1);

    ret = dspLS(3, "recGetByte...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("recGetByte...");
    CHECK(ret >= 0, lblKO);

    ret = recGetByte(rec, recByte, buf.aByte);
    tmrPause(1);
    CHECK(ret == 1, lblKO);
    CHECK(buf.aByte == 0x12, lblKO);
    ret = prtS("0x12");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "recGetWord...");
    CHECK(ret >= 0, lblKO);

    ret = recGetWord(rec, recWord, buf.aWord);
    tmrPause(1);

    CHECK(ret == 2, lblKO);
    CHECK(buf.aWord == 0x1234, lblKO);
    ret = prtS("0x1234");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "recGetCard...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("recGetCard...");
    CHECK(ret >= 0, lblKO);

    ret = recGetCard(rec, recCard, buf.aCard);
    tmrPause(1);

    CHECK(ret == 4, lblKO);
    CHECK(buf.aCard == 0x12345678L, lblKO);
    ret = prtS("0x12345678");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, "recGet String...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("recGet String...");
    CHECK(ret >= 0, lblKO);

    ret = recGetStr(rec, recStr, tmp);  //retrieve string with ending zero
    tmrPause(1);

    CHECK(ret == 16 + 1, lblKO);    //it is the length of the field in the record + ending zero
    CHECK(strcmp(tmp, "lalala") == 0, lblKO);

    ret = dspLS(3, "recGet...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("recGet...");
    CHECK(ret >= 0, lblKO);

    ret = recGet(rec, recStr, buf.aStr, 16);    //retrieve data using default length; no ending zero is added
    tmrPause(1);

    CHECK(ret == 16, lblKO);    //no ending zero
    CHECK(memcmp(buf.aStr, "lalala", 6) == 0, lblKO);

    ret = recFlush(rec);
    CHECK(ret == 0 || ret == (int) recSize(rec), lblKO);

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

void tcab0030(void) {
    int ret;
    byte volDba[L_VOL];
    byte cch[L_VOL];
    tRecord rec;
    word recLen[recEnd];        //contains field lengths
    char recFmt[recEnd];        //contains field formats
    word recMap[recEnd];        //contains field offsets

    trcS("tcab0030 Beg\n");
    nvmStart();                 //initialise non-volatile memory

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = prtS("**** Vol. mem. RAM ****");
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "dbaSetPtr...");
    ret = prtS("dbaSetPtr...");
    CHECK(ret >= 0, lblKO);
    memset(volDba, 0, L_VOL);
    dbaSetPtr(volDba);          //the buffer volDba will be used for volatile section of data base

    tmrPause(1);

    memset(recLen, 0, recEnd * sizeof(word));
    //fill lengths; offsets will be filled by recInit
    recLen[recByte] = sizeof(byte);
    recLen[recWord] = sizeof(word);
    recLen[recCard] = sizeof(card);
    recLen[recStr] = 16;
    memset(recMap, 0, recEnd * sizeof(word));

    memset(recFmt, 0, recEnd * sizeof(char));
    recFmt[recByte] = 'B';
    recFmt[recWord] = 'W';
    recFmt[recCard] = 'C';
    recFmt[recStr] = 'S';

    ret = dspLS(1, "Vol recInit...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("Vol recInit...");
    CHECK(ret >= 0, lblKO);

    ret = recInit(&rec, 0xFF, 0, recEnd, recLen, recFmt, recMap);   //initiate a record in volatile memory
    VERIFY(ret == (int) recSize(&rec));
    tmrPause(1);

    ret = testRec(&rec);        //watch volDba in the debugger window
    CHECK(ret >= 0, lblKO);

    ret = dspClear();
    CHECK(ret >= 0, lblKO);

    ret = prtS("** Non Vol. mem. DFS **");
    CHECK(ret >= 0, lblKO);
    ret = dspLS(1, "Nvm recInit...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("Nvm recInit...");
    CHECK(ret >= 0, lblKO);

    ret = recInit(&rec, 0, 0, recEnd, recLen, recFmt, recMap);  //initiate a record in non-volatile memory
    VERIFY(ret == (int) recSize(&rec));

    tmrPause(1);

    ret = testRec(&rec);        //look at ...\CommonFiles\...\page0000.nvm after executing this test
    CHECK(ret >= 0, lblKO);

    tmrPause(1);

    ret = dspClear();
    CHECK(ret >= 0, lblKO);

    ret = prtS("*** Cache mem. ****");
    CHECK(ret >= 0, lblKO);
    ret = dspLS(1, "Cache...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("Cache... DFS to RAM");
    CHECK(ret >= 0, lblKO);

    ret = recCache(&rec, cch, L_VOL);
    CHECK(ret >= 0, lblKO);

    tmrPause(1);

    ret = testRec(&rec);        //test a record with cache
    CHECK(ret >= 0, lblKO);

    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
    tmrPause(3);
  lblEnd:
    dspStop();                  //close resource
    prtStop();
    trcS("tcab0030 End\n");
}
