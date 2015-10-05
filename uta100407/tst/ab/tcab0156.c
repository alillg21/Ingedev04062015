#include <string.h>
#include "pri.h"
#include "tst.h"

static code const char *srcFile = __FILE__;

#if 0
//Unitary test case tcab0156.
//Functions testing:
//  ecrStart
//  ecrStop
//  ecrPut
//  ecrGet

/*
ecrLoadArticle ecr=9 bar=0000000100016 pri=300.00 dsc=HALO KARTA-300 tax=4
ecrPut: FE 00 01 00 01 01 00  [???????]
ecrGet: FE 03 01 00 00 00 00  [???????]

ecrPut: FE 01 06 00 0B 1D 00 00 01 00 29 00  [??????????)?]
ecrGet: FE 03 01 00 00 00 00  [???????]

ecrPut: FE 02 22 00 00 00 09 00 00 00 00 00 10 00 16 48 41 4C 4F 20 4B 41 52 54 41 2D 33 30 30 1C 00 04 1E 00 30 75 00 00 89 04  [??"????????????HALO KARTA-300?????0u????]
ecrGet: FE 03 01 00 00 00 00  [???????]

ecrPut: FE 04 01 00 88 88 00  [???????]
ecrGet: FE 04 01 00 AA AA 00  [???????]
*/

enum {
    eTypREGNUM = 0x00,
    eTypINIT = 0x01,
    eTypDATA = 0x02,
    eTypACK = 0x03,
    eTypEOC = 0x04,
    eTypSep = 0xFE,
    eTypEnd
};

#define CHK if(ret<0) return ret
static int makePacket(tBuffer * buf, byte typ, const byte * ptr, word len) {
    int ret;
    word sum;

    VERIFY(buf);
    VERIFY(ptr);

    bufReset(buf);
    ret = bufApp(buf, bPtr(eTypSep), 1);
    CHK;
    ret = bufApp(buf, bPtr(typ), 1);
    CHK;
    ret = bufApp(buf, bPtr(LBYTE(len)), 1);
    CHK;
    ret = bufApp(buf, bPtr(HBYTE(len)), 1);
    CHK;
    ret = bufApp(buf, ptr, len);

    sum = 0;
    while(len--)
        sum += *ptr++;

    ret = bufApp(buf, bPtr(LBYTE(sum)), 1);
    CHK;
    ret = bufApp(buf, bPtr(HBYTE(sum)), 1);
    CHK;

    return bufLen(buf);
}

static void ecrPause(void) {    //no result
    tmrStart(0, 100);
    while(tmrGet(0));
    tmrStop(0);
}

static int getACK(void) {
    int ret;
    byte dat[7];
    tBuffer msg;

    bufInit(&msg, dat, 7);

    tmrPause(1);
    ret = ecrGet(&msg, 5);
    CHECK(ret == 7, lblKO);
    CHECK(dat[0] == eTypSep, lblKO);
    CHECK(dat[1] == eTypACK, lblKO);
    CHECK(dat[2] == 1, lblKO);
    CHECK(dat[3] == 0, lblKO);
    //CHECK(dat[4]==0,lblKO);
    return 1;
  lblKO:
    return -1;
}

static int ecrCmdRegNum(void) {
    int ret;
    byte dat[256];
    tBuffer msg;

    bufInit(&msg, dat, 256);

    //FE 00 01 00 01 01 00
    ret = makePacket(&msg, eTypREGNUM, "\x01", 1);
    CHECK(ret == 7, lblKO);
    ret = ecrPut(bufPtr(&msg), bufLen(&msg), 3);
    CHECK(ret == 7, lblKO);

    //FE 03 01 00 00 00 00
    ret = getACK();
    CHECK(ret == 1, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    trcErr(ret);
  lblEnd:
    return ret;
}

static int ecrCmdInit(byte cmd) {
    int ret;
    byte dat[256];
    tBuffer msg;
    byte tmp[6];

    bufInit(&msg, dat, 256);

    //FE 01 06 00 0B 1D 00 00 01 00 29 00
    memcpy(tmp, "\x0B\x00\x00\x00\x01\x00", 6);
    tmp[1] = cmd;
    ret = makePacket(&msg, eTypINIT, tmp, 6);
    CHECK(ret == 12, lblKO);
    ret = ecrPut(bufPtr(&msg), bufLen(&msg), 3);
    CHECK(ret == 12, lblKO);

    //FE 03 01 00 00 00 00
    ret = getACK();
    CHECK(ret == 1, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    trcErr(ret);
  lblEnd:
    return ret;
}

static int ecrCmdEoc(void) {
    int ret;
    byte dat[256];
    tBuffer msg;

    bufInit(&msg, dat, 256);

    //FE 04 01 00 88 88 00
    ret = makePacket(&msg, eTypEOC, "\x88", 1);
    CHECK(ret == 7, lblKO);
    ret = ecrPut(bufPtr(&msg), bufLen(&msg), 3);
    CHECK(ret == 7, lblKO);

    //ecrGet: FE 04 01 00 AA AA 00  [???????]
    ecrPause();
    ret = ecrGet(&msg, 5);
    CHECK(ret == 7, lblKO);
    CHECK(dat[0] == eTypSep, lblKO);
    //CHECK(dat[1]==eTypEOC,lblKO);
    CHECK(dat[2] == 1, lblKO);
    CHECK(dat[3] == 0, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    trcErr(ret);
  lblEnd:
    return ret;
}

static int testLoadArt(word ecr, const char *bar, const char *amt,
                       const char *dsc, byte tax) {
    int ret;
    byte dat[256];
    tBuffer msg;
    byte pck[256];
    tBuffer bdy;
    char tmp[14 + 1];
    byte bin[7];
    card pri;

    bufInit(&msg, dat, 256);
    bufInit(&bdy, pck, 256);

    ret = ecrCmdRegNum();
    CHECK(ret == 1, lblKO);

    ret = ecrCmdInit(0x1D);
    CHECK(ret == 1, lblKO);

    //FE 02 22 00 00 00 09 00 00 00 00 00 10 00 16 48 41 4C 4F 20 4B 41 52 54 41 2D 33 30 30 1C 00 04 1E 00 30 75 00 00 89 04
    ret = bufApp(&bdy, "\x00\x00", 2);  //undocumented?
    CHECK(ret > 0, lblKO);

    ret = bufApp(&bdy, bPtr(LBYTE(ecr)), 1);
    CHECK(ret > 0, lblKO);
    ret = bufApp(&bdy, bPtr(HBYTE(ecr)), 1);
    CHECK(ret > 0, lblKO);

    VERIFY(strlen(bar) <= 14);
    strcpy(tmp, bar);
    fmtPad(tmp, -14, '0');
    VERIFY(strlen(tmp) == 14);
    hex2bin(bin, tmp, 0);
    ret = bufApp(&bdy, bin, 7);
    CHECK(ret > 0, lblKO);

    memset(tmp, 0, 14 + 1);
    strcpy(tmp, dsc);
    ret = bufApp(&bdy, (byte *) tmp, 14);
    CHECK(ret > 0, lblKO);

    ret = bufApp(&bdy, "\x1C\x00", 2);  //fixed flags
    CHECK(ret > 0, lblKO);
    ret = bufApp(&bdy, bPtr(tax), 1);
    CHECK(ret > 0, lblKO);
    ret = bufApp(&bdy, "\x1E\x00", 2);  //fixed flags
    CHECK(ret > 0, lblKO);

    dec2num(&pri, amt, 0);
    ret = bufApp(&bdy, bPtr((byte) (pri % 0x100)), 1);
    CHECK(ret > 0, lblKO);
    pri /= 0x100;
    ret = bufApp(&bdy, bPtr((byte) (pri % 0x100)), 1);
    CHECK(ret > 0, lblKO);
    pri /= 0x100;
    ret = bufApp(&bdy, bPtr((byte) (pri % 0x100)), 1);
    CHECK(ret > 0, lblKO);
    pri /= 0x100;
    ret = bufApp(&bdy, bPtr((byte) (pri % 0x100)), 1);
    CHECK(ret > 0, lblKO);
    pri /= 0x100;

    ret = makePacket(&msg, eTypDATA, bufPtr(&bdy), bufLen(&bdy));
    CHECK(ret == 40, lblKO);
    ret = ecrPut(bufPtr(&msg), bufLen(&msg), 3);
    CHECK(ret == 40, lblKO);

    //FE 03 01 00 00 00 00
    ret = getACK();
    CHECK(ret == 1, lblKO);

    ret = ecrCmdEoc();
    CHECK(ret == 1, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    trcErr(ret);
  lblEnd:
    return ret;
}

/* ret = ecrOnLineSale(7, 3); //ecrPLUnumber / Quantity
ecrPut: FE 00 01 00 01 01 00  [???????]
ecrGet: FE 03 01 00 00 00 00  [???????]
ecrPut: FE 01 06 00 0B 12 00 00 01 00 1E 00  [????????????]
ecrGet: FE 03 01 00 00 00 00  [???????]
ecrPut: FE 02 1C 00 20 20 20 20 20 20 20 20 20 20 20 20 20 20 01 01 01 00 00 00 00 B8 0B 00 00 07 00 01 8E 02  [????              ????????????????]
ecrGet: FE 03 01 00 00 00 00  [???????]
ecrPut: FE 04 01 00 B0 B0 00  [???????]
ecrGet: FE 04 01 00 AA AA 00  [???????]
*/

static int testOnlineSale(word ecr, card qty) {
    int ret;
    byte dat[256];
    tBuffer msg;
    byte pck[256];
    tBuffer bdy;
    char tmp[14 + 1];

    bufInit(&msg, dat, 256);
    bufInit(&bdy, pck, 256);

    ret = ecrCmdRegNum();
    CHECK(ret == 1, lblKO);

    ret = ecrCmdInit(0x12);
    CHECK(ret == 1, lblKO);

    //FE 02 1C 00 20 20 20 20 20 20 20 20 20 20 20 20 20 20 01 01 01 00 00 00 00 B8 0B 00 00 07 00 01 8E 02
    memset(tmp, ' ', 14);
    ret = bufApp(&bdy, (byte *) tmp, 14);
    CHECK(ret > 0, lblKO);

    ret = bufApp(&bdy, "\x01", 1);  //unit name
    CHECK(ret > 0, lblKO);
    ret = bufApp(&bdy, "\x01", 1);  //department
    CHECK(ret > 0, lblKO);
    ret = bufApp(&bdy, "\x01", 1);  //tax
    CHECK(ret > 0, lblKO);
    ret = bufApp(&bdy, "\x00\x00\x00\x00", 4);  //price
    CHECK(ret > 0, lblKO);

    qty *= 1000;
    ret = bufApp(&bdy, bPtr((byte) (qty % 0x100)), 1);
    CHECK(ret > 0, lblKO);
    qty /= 0x100;
    ret = bufApp(&bdy, bPtr((byte) (qty % 0x100)), 1);
    CHECK(ret > 0, lblKO);
    qty /= 0x100;
    ret = bufApp(&bdy, bPtr((byte) (qty % 0x100)), 1);
    CHECK(ret > 0, lblKO);
    qty /= 0x100;
    ret = bufApp(&bdy, bPtr((byte) (qty % 0x100)), 1);
    CHECK(ret > 0, lblKO);
    qty /= 0x100;

    ret = bufApp(&bdy, bPtr(LBYTE(ecr)), 1);
    CHECK(ret > 0, lblKO);
    ret = bufApp(&bdy, bPtr(HBYTE(ecr)), 1);
    CHECK(ret > 0, lblKO);

    ret = bufApp(&bdy, "\x01", 1);  //online sale
    CHECK(ret > 0, lblKO);

    ret = makePacket(&msg, eTypDATA, bufPtr(&bdy), bufLen(&bdy));
    CHECK(ret == 34, lblKO);
    ret = ecrPut(bufPtr(&msg), bufLen(&msg), 3);
    CHECK(ret == 34, lblKO);

    //FE 03 01 00 00 00 00
    ret = getACK();
    CHECK(ret == 1, lblKO);

    ret = ecrCmdEoc();
    CHECK(ret == 1, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    trcErr(ret);
  lblEnd:
    return ret;
}

/* ret = ecrFinishSale();
ecrPut: FE 00 01 00 01 01 00  [???????]
ecrGet: FE 03 01 00 01 01 00  [???????]
ecrPut: FE 01 06 00 0B 12 00 00 01 00 1E 00  [????????????]
ecrGet: FE 03 01 00 01 01 00  [???????]
ecrPut: FE 02 1C 00 20 20 20 20 20 20 20 20 20 20 20 20 20 20 01 01 01 00 00 00 00 E8 03 00 00 01 00 03 B2 02  [????              ????????????????]
ecrGet: FE 03 01 00 01 01 00  [???????]
ecrPut: FE 04 01 00 B0 B0 00  [???????]
ecrGet: FE 03 01 00 01 01 00  [???????]
*/

void tcab0156(void) {
    int ret;

    ret = ecrStart();
    CHECK(ret >= 0, lblKO);

    tmrPause(1);

    //ret= testLoadArt(8, "0000000100015", "20000", "HALO KARTA-200", 4);
    //CHECK(ret>=0,lblKO);

    ret = testOnlineSale(7, 3);
    CHECK(ret >= 0, lblKO);

    ret = testOnlineSale(7, 3);
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
    tmrPause(3);
  lblEnd:
    ecrStop();
    dspStop();                  //close resource
}
#endif
