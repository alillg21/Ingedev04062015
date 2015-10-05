/** \file
 * Unitary test case tcab0209.
 * Binary XML builder
 * Functions testes:
 * \sa
 *  - bmlPutHdr()
 *  - bmlPutTagBeg()
 *  - bmlPutAtr()
 *  - bmlPutTagEnd()
 *  - bmlPutAtrEnd()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0209.c $
 *
 * $Id: tcab0209.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "pri.h"
#include "tst.h"

enum bmlTag {
    //ABC tags
    bmlTagGet = 5,
    bmlTagPut,
    bmlTagRun,
    bmlTagData,

    //BUS tags
    bmlTagReq = 5,
    bmlTagRsp,
    bmlTagLst,
    bmlTagRow,

    bmlTagEnd
};
enum bmlAtr {
    //ABC Atrs
    bmlAtrId = 5,
    bmlAtrFmt,
    bmlAtrLen,
    bmlAtrVal,
    bmlAtrErr,
    bmlAtrCmd,

    //BUS atrs
    bmlBusAtrApp = 5,
    bmlBusAtrId,
    bmlBusAtrDt,
    bmlBusAtrTid,
    bmlBusAtrMid,
    bmlBusAtrSeq,
    bmlBusAtrPan,
    bmlBusAtrAmt,
    bmlBusAtrCr,
    bmlBusAtrDb,
    bmlBusAtrCode,
    bmlBusAtrDsp,
    bmlBusAtrPrt,
    bmlBusAtrSec,
    bmlBusAtrUsage,
    bmlBusAtrSer,
    bmlBusAtrBat,

    bmlAtrEnd
};

/*  
<?xml version="1.0"?>
<get>
  <data id='3'/>
  <data id='17'/>
</get>
*/
static int testABC(tStream * stm, tBuffer * buf, const char *dst) {
    int ret;
    word dim;

    VERIFY(stm);

    ret = stmInit(stm, buf, dst, 'w');
    CHECK(ret >= 0, lblKO);
    dim = 0;

    ret = bmlPutHdr(stm);
    CHECK(ret == 4, lblKO);
    dim += ret;

    ret = bmlPutTagBeg(stm, bmlTagGet | bmlBitC);   //tag with content without attributes
    CHECK(ret == 1, lblKO);
    dim += ret;

    //First line of data
    ret = bmlPutTagBeg(stm, bmlTagData | bmlBitA);  //tag without content with attributes
    CHECK(ret == 1, lblKO);
    dim += ret;

    ret = bmlPutAtr(stm, bmlAtrId, "3");
    CHECK(ret == 1 + 1 + 1 + 1, lblKO);
    dim += ret;

    ret = bmlPutAtrEnd(stm, bmlTagData | bmlBitA);
    CHECK(ret == 1, lblKO);
    dim += ret;

    ret = bmlPutTagEnd(stm, bmlTagData | bmlBitA);
    CHECK(ret == 0, lblKO);     //no content tags
    dim += ret;

    //Second line of data
    ret = bmlPutTagBeg(stm, bmlTagData | bmlBitA);  //tag without content with attributes
    CHECK(ret == 1, lblKO);
    dim += ret;

    ret = bmlPutAtr(stm, bmlAtrId, "17");
    CHECK(ret == 1 + 1 + 2 + 1, lblKO);
    dim += ret;

    ret = bmlPutAtrEnd(stm, bmlTagData | bmlBitA);
    CHECK(ret == 1, lblKO);
    dim += ret;

    ret = bmlPutTagEnd(stm, bmlTagData | bmlBitA);
    CHECK(ret == 0, lblKO);     //no content tags
    dim += ret;

    //Finish englobing 'get' tag
    ret = bmlPutAtrEnd(stm, bmlTagGet | bmlBitC);
    CHECK(ret == 0, lblKO);     //no attributes
    dim += ret;

    ret = bmlPutTagEnd(stm, bmlTagGet | bmlBitC);
    CHECK(ret == 1, lblKO);
    dim += ret;

    ret = stmClose(stm);
    CHECK(ret >= 0, lblKO);

    trcS("testABC: buf= ");
    trcBAN(bufPtr(buf), bufLen(buf));
    trcS("\n");

    return dim;
  lblKO:
    return -1;
}

/*
<?xml version="1.0"?>
<lst app="7U" id="telecollect"
    dt="20070102230000"
    tid="00000001"
    mid="1234"
    bat="003">
    <row id="open"
        dt="20070102120600"
        seq="1"
        pan="1234567812345678"
    />
    <row id="pass"
        dt="20070102120700"
        seq="2"
        pan="1234123412341234"
        amt="1"
    />
    <row id="pass"
        dt="20070102120800"
        seq="3"
        pan="4321432143214321"
        amt="2"
    />
    <row id="close"
        dt="20070102120800"
        seq="1"
        pan="1234567812345678"
    />
</lst>
*/

static int putBusRow1(tStream * stm) {
    int ret;
    word dim;
    byte tag = bmlTagRow | bmlBitA; //tag without content with attributes

    VERIFY(stm);

    dim = 0;

    ret = bmlPutTagBeg(stm, tag);
    CHECK(ret == 1, lblKO);
    dim += ret;

    ret = bmlPutAtr(stm, bmlBusAtrId, "open");
    CHECK(ret > 0, lblKO);
    dim += ret;

    ret = bmlPutAtr(stm, bmlBusAtrDt, "20080907175100");
    CHECK(ret > 0, lblKO);
    dim += ret;

    ret = bmlPutAtr(stm, bmlBusAtrSeq, "1");
    CHECK(ret > 0, lblKO);
    dim += ret;

    ret = bmlPutAtr(stm, bmlBusAtrPan, "1234567812345678");
    CHECK(ret > 0, lblKO);
    dim += ret;

    ret = bmlPutAtrEnd(stm, tag);
    CHECK(ret == 1, lblKO);
    dim += ret;

    return dim;
  lblKO:
    return -1;
}

static int putBusRow2(tStream * stm) {
    int ret;
    word dim;
    byte tag = bmlTagRow | bmlBitA; //tag without content with attributes

    VERIFY(stm);

    dim = 0;

    ret = bmlPutTagBeg(stm, tag);
    CHECK(ret == 1, lblKO);
    dim += ret;

    ret = bmlPutAtr(stm, bmlBusAtrId, "pass");
    CHECK(ret > 0, lblKO);
    dim += ret;

    ret = bmlPutAtr(stm, bmlBusAtrDt, "20080907175300");
    CHECK(ret > 0, lblKO);
    dim += ret;

    ret = bmlPutAtr(stm, bmlBusAtrSeq, "2");
    CHECK(ret > 0, lblKO);
    dim += ret;

    ret = bmlPutAtr(stm, bmlBusAtrPan, "1234123412341234");
    CHECK(ret > 0, lblKO);
    dim += ret;

    ret = bmlPutAtr(stm, bmlBusAtrAmt, "1");
    CHECK(ret > 0, lblKO);
    dim += ret;

    ret = bmlPutAtrEnd(stm, tag);
    CHECK(ret == 1, lblKO);
    dim += ret;

    return dim;
  lblKO:
    return -1;
}

static int putBusRow3(tStream * stm) {
    int ret;
    word dim;
    byte tag = bmlTagRow | bmlBitA; //tag without content with attributes

    VERIFY(stm);

    dim = 0;

    ret = bmlPutTagBeg(stm, tag);
    CHECK(ret == 1, lblKO);
    dim += ret;

    ret = bmlPutAtr(stm, bmlBusAtrId, "pass");
    CHECK(ret > 0, lblKO);
    dim += ret;

    ret = bmlPutAtr(stm, bmlBusAtrDt, "20080907175400");
    CHECK(ret > 0, lblKO);
    dim += ret;

    ret = bmlPutAtr(stm, bmlBusAtrSeq, "3");
    CHECK(ret > 0, lblKO);
    dim += ret;

    ret = bmlPutAtr(stm, bmlBusAtrPan, "4321432143214321");
    CHECK(ret > 0, lblKO);
    dim += ret;

    ret = bmlPutAtr(stm, bmlBusAtrAmt, "2");
    CHECK(ret > 0, lblKO);
    dim += ret;

    ret = bmlPutAtrEnd(stm, tag);
    CHECK(ret == 1, lblKO);
    dim += ret;

    return dim;
  lblKO:
    return -1;
}

static int putBusRow4(tStream * stm) {
    int ret;
    word dim;
    byte tag = bmlTagRow | bmlBitA; //tag without content with attributes

    VERIFY(stm);

    dim = 0;

    ret = bmlPutTagBeg(stm, tag);
    CHECK(ret == 1, lblKO);
    dim += ret;

    ret = bmlPutAtr(stm, bmlBusAtrId, "close");
    CHECK(ret > 0, lblKO);
    dim += ret;

    ret = bmlPutAtr(stm, bmlBusAtrDt, "20080907175500");
    CHECK(ret > 0, lblKO);
    dim += ret;

    ret = bmlPutAtr(stm, bmlBusAtrSeq, "4");
    CHECK(ret > 0, lblKO);
    dim += ret;

    ret = bmlPutAtr(stm, bmlBusAtrPan, "1234567812345678");
    CHECK(ret > 0, lblKO);
    dim += ret;

    ret = bmlPutAtrEnd(stm, tag);
    CHECK(ret == 1, lblKO);
    dim += ret;

    return dim;
  lblKO:
    return -1;
}

static int putBusLst(tStream * stm) {
    int ret;
    word dim;

    VERIFY(stm);

    dim = 0;

    ret = bmlPutTagBeg(stm, bmlTagLst | bmlBitC | bmlBitA); //tag with content with attributes
    CHECK(ret == 1, lblKO);
    dim += ret;

    ret = bmlPutAtr(stm, bmlBusAtrApp, "7U");
    CHECK(ret > 0, lblKO);
    dim += ret;

    ret = bmlPutAtr(stm, bmlBusAtrId, "telecollect");
    CHECK(ret > 0, lblKO);
    dim += ret;

    ret = bmlPutAtr(stm, bmlBusAtrDt, "20080907171700");
    CHECK(ret > 0, lblKO);
    dim += ret;

    ret = bmlPutAtr(stm, bmlBusAtrTid, "00000001");
    CHECK(ret > 0, lblKO);
    dim += ret;

    ret = bmlPutAtr(stm, bmlBusAtrMid, "1234");
    CHECK(ret > 0, lblKO);
    dim += ret;

    ret = bmlPutAtr(stm, bmlBusAtrBat, "003");
    CHECK(ret > 0, lblKO);
    dim += ret;

    ret = bmlPutAtrEnd(stm, bmlTagLst | bmlBitC | bmlBitA); //tag with content with attributes
    CHECK(ret == 1, lblKO);
    dim += ret;

    ret = putBusRow1(stm);
    CHECK(ret > 0, lblKO);
    dim += ret;

    ret = putBusRow2(stm);
    CHECK(ret > 0, lblKO);
    dim += ret;

    ret = putBusRow3(stm);
    CHECK(ret > 0, lblKO);
    dim += ret;

    ret = putBusRow4(stm);
    CHECK(ret > 0, lblKO);
    dim += ret;

    ret = bmlPutTagEnd(stm, bmlTagLst | bmlBitC | bmlBitA); //tag with content with attributes
    CHECK(ret == 1, lblKO);
    dim += ret;

    return dim;
  lblKO:
    return -1;
}

static int testBUS(tStream * stm, tBuffer * buf, const char *dst) {
    int ret;
    word dim;

    VERIFY(stm);

    ret = stmInit(stm, buf, dst, 'w');
    CHECK(ret >= 0, lblKO);
    dim = 0;

    ret = bmlPutHdr(stm);
    CHECK(ret == 4, lblKO);
    dim += ret;

    ret = putBusLst(stm);
    CHECK(ret > 0, lblKO);
    dim += ret;

    ret = stmClose(stm);
    CHECK(ret > 0, lblKO);

    return dim;
  lblKO:
    return -1;
}

void tcab0209(void) {
    int ret;
    byte dat[4096];
    tBuffer buf;
    tStream stm;
    char tmp[64 + 1];

    trcS("tcab0209 Beg\n");

    nvmStart();
    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    bufInit(&buf, dat, 4096);

    dspLS(0, "ABC RAM...");
    ret = testABC(&stm, &buf, 0);
    CHECK(ret >= 0, lblKO);

    bufReset(&buf);

    dspLS(1, "BUS File...");
    nvmFileName(tmp, 1);
    ret = testBUS(&stm, &buf, tmp);
    CHECK(ret >= 0, lblKO);

    dspLS(3, "Done.");
    goto lblEnd;
  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
  lblEnd:
    tmrPause(3);
    dspStop();
    prtStop();
    trcS("tcab0209 End\n");
}
