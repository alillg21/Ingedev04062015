/// \file tcik0107.c
#include <string.h>
#include <unicapt.h>
#include "sys.h"
#include "pri.h"
#include "tst.h"

/** UTA Unitary test case for GPRS attach/detach.
 * \sa
 *  -
 */

#define CHK if(ret<0) return ret;

typedef struct sTleParam {
    byte tmrG;                  //global timeout
    byte tmrF;                  //frame timeout
    byte tmrC;                  //character timeout
    byte tryS;                  //number of send trial
    byte tryR;                  //number of receive trial
} tTleParam;

static int getPgnConn(void) {
    int ret;
    byte Rssi, Sta;
    int attach;

    Sta = 0;
    Rssi = 0;
    dspLS(1, "Check connection");
    ret = pgnSgnLvl(&Rssi, &Sta);
    CHECK(ret >= 0, lblKO);

    if(Sta == 0) {
        attach = 1;
        dspLS(1, "gprs attach");
        ret = gmaInqGsmGprsConn(&attach);
        CHECK(ret >= 0, lblKO);
    }

    return Sta;
  lblKO:
    return -1;
}

static int onlDialGcl_tst(void) {
    int ret;

    ret = tmrStart(2, 30 * 100);    // 30 s. timeout
    CHECK(ret >= 0, lblKO);

    ret = kbdStart(1);
    CHECK(ret >= 0, lblKO);

    ret = tftStart();
    CHECK(ret >= 0, lblKO);

    ret = utaGclDial();
    CHECK(ret >= 0, lblKO);

    while(utaGclDialInfo() != gclStaFinished) {
        CHECK(kbdKey() == 0, lblStop);
        CHECK(tftGet() == 0, lblStop);
        CHECK(tmrGet(2), lblKO);
    }

    ret = utaGclDialErr();

    ret = 1;
    goto lblEnd;
  lblKO:
    ret = utaGclDialErr();
    trcFN("utaGclDialErr: err=%d\n", ret);
    ret = -1;
    goto lblEnd;
  lblStop:
    ret = 0;
    goto lblEnd;
  lblEnd:
    tftStop();
    kbdStop();
    tmrStop(2);
    trcFN("onlGclDial: ret=%d\n", ret);
    return ret;
}

static int tleInit_tst(tTleParam * par) {
    VERIFY(par);
    par->tmrG = 120;
    //par->tmrG = 300; //add 1min for connexion
    par->tmrF = 20;
    par->tmrC = 3;
    par->tryS = 3;
    par->tryR = 3;
    return 1;
}

static int sendReq_tst(tTleParam * par) {
    int ret;
    byte dReq[1024];
    tBuffer bReq;

    VERIFY(par);
    trcS("sendReq: Beg\n");

    bufInit(&bReq, dReq, sizeof(dReq));
    bufReset(&bReq);            //reset request buffer

    ret = bufApp(&bReq, (byte *) "\x00\x00\x00\x3B", 4);

    ret = bufApp(&bReq, (byte *) "\x01\x01\x04\x00", 4);    //binary xml header
    CHECK(ret > 0, lblKO);

    ret = bufApp(&bReq, (byte *) "\x85", 1);    //tag req with attributes
    ret = bufApp(&bReq, (byte *) "\x0B\x03\x35\x56\x00", 5);    //app
    ret = bufApp(&bReq, (byte *) "\x05\x03\x68\x61\x6E\x64\x73\x68\x61\x6B\x65\x00", 12);   //id handshake
    ret = bufApp(&bReq, (byte *) "\x1D\x03\x32\x30\x30\x37\x30\x36\x30\x34\x31\x31\x31\x30\x31\x30\x00", 17);   //dt
    ret = bufApp(&bReq, (byte *) "\x06\x03\x30\x30\x30\x30\x30\x30\x30\x31\x00", 11);   //tid
    ret = bufApp(&bReq, (byte *) "\x0F\x03\x30\x00", 4);    //mid
    ret = bufApp(&bReq, (byte *) "\x14\x03\x30\x00", 4);    //aut
    ret = bufApp(&bReq, (byte *) "\x01", 1);    //bmlEND

    ret = utaGclSendBuf(bufPtr(&bReq), bufLen(&bReq));
    trcFN("comSendBuf : ret=%d\n", ret);
    CHECK(ret == bufLen(&bReq), lblKO); //send the request

    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    trcFN("sendReq: ret=%d\n", ret);
    return ret;
}

void tcik0107(void) {
    int ret;
    tTleParam par;

    dspStart();
    ret = dspClear();
    CHECK(ret >= 0, lblKO);

    dspLS(0, "tcik0107...");

    ret = getPgnConn();
    CHECK(ret >= 0, lblKO);

    dspLS(2, "gclStart");
    ret = utaGclStart(gclSck, "91.121.18.221|6789");
    CHECK(ret >= 0, lblKO);

    dspLS(3, "gclDial");
    ret = onlDialGcl_tst();
    if(ret < 0) {
        utaGclStop();
        goto lblKO;
    }

    dspLS(0, "tleInit");
    ret = tleInit_tst(&par);
    CHECK(ret >= 0, lblKO);

    dspLS(1, "sendReq");
    ret = sendReq_tst(&par);
    CHECK(ret >= 0, lblKO);

    dspLS(1, "gprs detach");
    ret = gmaInqGsmGprsDiscon(1);
    CHECK(ret >= 0, lblKO);

    dspLS(2, "utaGclStop");
    utaGclStop();

    goto lblEnd;
  lblKO:
    utaGclStop();
    trcErr(ret);
    dspLS(3, "KO! tcik0107");
  lblEnd:
    tmrPause(1);
    trcS("tcik0107: End\n");
    dspStop();
    return;
}
