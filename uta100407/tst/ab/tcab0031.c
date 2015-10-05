/** \file
 * Unitary test case tcab0031.
 * Functions tested:
 * \sa
 *  - recMove()
 *  - recPos()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0031.c $
 *
 * $Id: tcab0031.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "pri.h"
#include "tst.h"

//Create and fill two linked tables.
//Using cursor function recMove retrieve related data

//typedef struct sBinRecord{ bin table record
//    char bin1[6]; 
//    char bin2[6];
//    byte profile;
//}tBinRecord;

enum eBin {                     //bin table record fields
    binBeg,                     //start sentinel
    binBin1,                    //lower BIN bound
    binBin2,                    //upper BIN bound
    binProfile,                 //related card profile
    binEnd                      //end sentinel
};

//typedef struct sProfileRecord{ //bin profile table record
//    char iss[10]; //issuer
//    char acq[10]; //acquirer
//}tProfileRecord;

enum ePrf {                     //bin profile table record fields
    prfBeg,                     //start sentinel
    prfIss,                     //issuer
    prfAcq,                     //acquirer
    prfEnd                      //end sentinel
};

static int buildBinTable(tTable * tab, tRecord * rec, word ofs) {
    int ret;
    word len;
    static word binLen[binEnd]; //contains field lengths
    static word binMap[binEnd]; //contains field offsets

    ret = dspLS(0, "Bin recInit...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("Bin recInit...");
    CHECK(ret >= 0, lblKO);

    //pre-fill binMap by lengths
    memset(binLen, 0, binEnd * sizeof(word));
    binLen[binBin1] = 6;
    binLen[binBin2] = 6;
    binLen[binProfile] = sizeof(byte);
    memset(binMap, 0, binEnd * sizeof(word));

    //initiate bin record in section 0 at the offset ofs
    ret = recInit(rec, 0, ofs, binEnd, binLen, 0, binMap);
    VERIFY(ret == (int) recSize(rec));
    len = ret;
    tmrPause(1);

    ret = dspLS(1, "Bin tabInit...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("Bin tabInit...");
    CHECK(ret >= 0, lblKO);

    //initiate bin table of 3 records in section 0 at the offset ofs of record length len
    ret = tabInit(tab, 0, ofs, len, 3, 0);
    VERIFY(ret == (int) tabSize(tab));
    len = ret;
    tmrPause(1);

    ret = dspLS(2, "Fill bin table...");
    CHECK(ret >= 0, lblKO);

    ret = tabReset(tab);
    CHECK(ret == (int) tabSize(tab), lblKO);

    //Fill the first record by ("400000","449999",0)
    ret = recMove(rec, tab, 0); //move to the beginning of the table
    CHECK(ret == 0, lblKO);

    ret = recPos(rec, tab);
    CHECK(ret == 0, lblKO);

    ret = recPut(rec, binBin1, "400000", 0);
    CHECK(((word) ret) == 6, lblKO);    //ending zero is not saved

    ret = recPutStr(rec, binBin2, "449999");
    CHECK(((word) ret) == 6, lblKO);    //ending zero is not saved

    ret = recPutByte(rec, binProfile, 0);
    CHECK(((word) ret) == 1, lblKO);    //field type is byte

    //Fill the second record by ("500000","599999",1)
    ret = recMove(rec, tab, 1); //move to the next record
    CHECK(ret == 1, lblKO);

    ret = recPos(rec, tab);
    CHECK(ret == 1, lblKO);

    ret = recPutStr(rec, binBin1, "500000");
    CHECK(((word) ret) == 6, lblKO);    //ending zero is not saved

    ret = recPutStr(rec, binBin2, "599999");
    CHECK(((word) ret) == 6, lblKO);    //ending zero is not saved

    ret = recPutByte(rec, binProfile, 1);
    CHECK(((word) ret) == 1, lblKO);    //field type is byte
    ret = prtS("500000 599999 1");
    CHECK(ret >= 0, lblKO);
    //Fill the last record by ("450000","499999",0)
    ret = recMove(rec, tab, 2); //move to the last record
    CHECK(ret == 2, lblKO);

    ret = recPos(rec, tab);
    CHECK(ret == 2, lblKO);

    ret = recPutStr(rec, binBin1, "450000");
    CHECK(((word) ret) == 6, lblKO);    //ending zero is not saved

    ret = recPutStr(rec, binBin2, "499999");
    CHECK(((word) ret) == 6, lblKO);    //ending zero is not saved

    ret = recPutByte(rec, binProfile, 0);
    CHECK(((word) ret) == 1, lblKO);    //field type is byte
    ret = prtS("450000 499999 0");
    CHECK(ret >= 0, lblKO);
    ret = ofs + tabSize(tab);
    tmrPause(1);

    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
    tmrPause(3);
    ret = -1;
  lblEnd:
    return ret;
}

static int buildProfileTable(tTable * tab, tRecord * rec, word ofs) {
    int ret;
    word len;
    static word prfLen[prfEnd]; //contains field lengths
    static word prfMap[prfEnd]; //contains field offsets

    ret = dspLS(0, "Profile recInit...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("Profile recInit...");
    CHECK(ret >= 0, lblKO);

    //pre-fill binMap by lengths
    memset(prfLen, 0, prfEnd * sizeof(word));
    prfLen[prfIss] = 10;
    prfLen[prfAcq] = 10;
    memset(prfMap, 0, prfEnd * sizeof(word));

    //initiate bin record in section 0 at the offset ofs
    ret = recInit(rec, 0, ofs, prfEnd, prfLen, 0, prfMap);
    VERIFY(ret == (int) recSize(rec));
    len = ret;
    tmrPause(1);

    ret = dspLS(1, "Profile tabInit...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("Profile tabInit...");
    CHECK(ret >= 0, lblKO);

    //initiate bin table of 2 records in section 0 at the offset ofs of record length len
    ret = tabInit(tab, 0, ofs, len, 2, 0);
    VERIFY(ret == (int) tabSize(tab));
    len = ret;
    tmrPause(1);

    ret = dspLS(2, "Fill prf table...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("Fill prf table...");
    CHECK(ret >= 0, lblKO);

    ret = tabReset(tab);
    CHECK(ret == (int) tabSize(tab), lblKO);

    //Fill the first record by ("VISA","ACI")
    ret = recMove(rec, tab, 0); //move to the beginning of the table
    CHECK(ret == 0, lblKO);

    ret = recPos(rec, tab);
    CHECK(ret == 0, lblKO);

    ret = recPutStr(rec, prfIss, "VISA");
    CHECK(((word) ret) == 4, lblKO);    //ending zero is saved

    ret = recPutStr(rec, prfAcq, "ACI");
    CHECK(((word) ret) == 3, lblKO);    //ending zero is saved
    ret = prtS("VISA ACI");
    CHECK(ret >= 0, lblKO);

    //Fill the second record by ("EUROPAY","CTL")
    ret = recMove(rec, tab, 1); //move to the next record
    CHECK(ret == 1, lblKO);

    ret = recPos(rec, tab);
    CHECK(ret == 1, lblKO);

    ret = recPutStr(rec, prfIss, "EUROPAY");
    CHECK(((word) ret) == 7, lblKO);    //ending zero is saved

    ret = recPutStr(rec, prfAcq, "CTL");
    CHECK(((word) ret) == 3, lblKO);    //ending zero is saved
    ret = prtS("EUROPAY CTL");
    CHECK(ret >= 0, lblKO);

    ret = ofs + tabSize(tab);
    tmrPause(1);

    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
    tmrPause(3);
    ret = -1;
  lblEnd:
    return ret;
}

static int classify(const char *pan, const tTable * tabBin, tRecord * recBin,
                    const tTable * tabPrf, tRecord * recPrf) {
    int ret;
    word idxBin;
    byte idxPrf;
    char bin[6 + 1];
    char buf[10 + 1];
    char tmp[prtW + 1];

    ret = dspClear();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "classify...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("classify...");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, pan);
    CHECK(ret >= 0, lblKO);
    strcpy(tmp, "Pan: ");
    strcpy(&tmp[5], pan);
    ret = prtS(tmp);
    CHECK(ret >= 0, lblKO);

    for (idxBin = 0; idxBin < tabDim(tabBin); idxBin++) {
        ret = recMove(recBin, tabBin, idxBin);
        CHECK(ret == idxBin, lblKO);

        ret = recPos(recBin, tabBin);
        CHECK(ret == idxBin, lblKO);

        ret = recGetStr(recBin, binBin1, bin);
        CHECK(ret == 6 + 1, lblKO); //ending zero is filled

        if(memcmp(pan, bin, 6) < 0)
            continue;

        ret = recGetStr(recBin, binBin2, bin);
        CHECK(ret == 6 + 1, lblKO); //ending zero is filled

        if(memcmp(bin, pan, 6) < 0)
            continue;

        break;
    }

    VERIFY(idxBin <= tabDim(tabBin));
    if(idxBin >= tabDim(tabBin)) {
        ret = dspLS(2, "NOT FOUND");
        CHECK(ret >= 0, lblKO);
        ret = prtS("NOT FOUND");
        CHECK(ret >= 0, lblKO);
        return 0;
    }

    ret = recGetByte(recBin, binProfile, idxPrf);
    CHECK(ret == 1, lblKO);     //field type is byte

    ret = recMove(recPrf, tabPrf, idxPrf);
    CHECK(ret == idxPrf, lblKO);

    ret = recPos(recPrf, tabPrf);
    CHECK(ret == idxPrf, lblKO);

    ret = recGetStr(recPrf, prfIss, buf);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, buf);
    CHECK(ret >= 0, lblKO);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    ret = recGetStr(recPrf, prfAcq, buf);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(3, buf);
    CHECK(ret >= 0, lblKO);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    ret = idxBin;
    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
    tmrPause(3);
    ret = -1;
  lblEnd:
    return ret;
}

void tcab0031(void) {
    int ret;
    tTable tabBin, tabPrf;
    tRecord recBin, recPrf;
    word ofs;

    trcS("tcab0031 Beg\n");

    nvmStart();                 //initialise non-volatile memory

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ofs = 0;
    ret = buildBinTable(&tabBin, &recBin, ofs);
    CHECK(ret >= 0, lblKO);

    ofs = ret;
    ret = buildProfileTable(&tabPrf, &recPrf, ofs);
    CHECK(ret >= 0, lblKO);

    ret = classify("4444444444444444", &tabBin, &recBin, &tabPrf, &recPrf);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = classify("5555555555555555", &tabBin, &recBin, &tabPrf, &recPrf);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = classify("4999999999999999", &tabBin, &recBin, &tabPrf, &recPrf);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = classify("9999999999999999", &tabBin, &recBin, &tabPrf, &recPrf);
    CHECK(ret >= 0, lblKO);
    tmrPause(3);

    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
    tmrPause(3);
  lblEnd:
    dspStop();                  //close resource
    prtStop();
    trcS("tcab0031 End\n");
}
