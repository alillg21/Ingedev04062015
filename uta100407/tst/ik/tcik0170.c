/// \file tcik0170.c

#include "ctx.h"
#include "sys.h"
#include <string.h>
#include "tst.h"

/** UTA test case for integration of GPRS GSM plug-in function.
* pgnLastErr
* pgnStopGprs
* pgnStartGprs
 */

static int treatGsmErr(int err) {
    int ret;

    switch (err) {
      case 0:
          ret = dspLS(0, "No error");
          CHECK(ret >= 0, lblKO);
          break;
      case -1:
          ret = dspLS(0, "PIN Required");
          CHECK(ret >= 0, lblKO);
          break;
      case -2:
          ret = dspLS(0, "PUK Required");
          CHECK(ret >= 0, lblKO);
          break;
      case -3:
          ret = dspLS(0, "SIM not inserted");
          CHECK(ret >= 0, lblKO);
          break;
      default:
          ret = dspLS(0, "Unkown error");
          CHECK(ret >= 0, lblKO);
          break;
    }

    return ret;
  lblKO:
    return -1;
}

static int restartGprs(void) {
    int ret;

    ret = pgnStopGprs();
    CHECK(ret >= 0, lblKO);

    ret = pgnStartGprs();
    CHECK(ret >= 0, lblKO);

    return ret;
  lblKO:
    return -1;
}

static int treatGprsErr(int err) {
    int ret = 0;

    switch (err) {
      case -11:                // netNiStart error when the error not fit in the errors bellow
      case -12:                // netNiStart error NET_ERR_NI_IN_WRONG_STATE
      case -13:                // netNiStart error NET_ECONNREFUSED
      case -14:                // netNiStart error NET_ETIMEDOUT
          ret = dspLS(0, "Contact GPRS provider");
          break;
      case -20:                // error in the attach process.
      case -22:                // GPRS not allowed (GSM_CME_ERROR error number 107)
      case -23:                // GPRS UNSPECIFIED error (GSM_CME_ERROR error 148)
      case -24:                // GPRS PDP AUTH FAILURE (GSM_CME_ERROR error 149)
      case -25:                // GPRS MISSING/UNK APN (GSM_CME_ERROR error 533)
      case -27:                // temp unavailable   (GSM_CME_ERROR error 134)
      case -28:                // connection timeout
      case -30:                // GPRS internal error
      case -31:                // APN not set
          ret = restartGprs();
          break;
    }

    return ret;
}

static int sendRequest(void) {
    int ret;
    byte dReq[1024];
    tBuffer bReq;

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

static int ConnectSocket(void) {
    int ret = 0;

/*
	dspLS(2, "gclStart");
  ret = utaGclStart(gclSck, "91.121.18.221|6789");
  CHECK(ret >= 0, lblKO);

  ret = utaGclDial();
  CHECK(ret >= 0, lblKO);

  ret = tmrStart(2, 30 * 100);    // 30 s. timeout
  CHECK(ret >= 0, lblKO);

  ret = kbdStart(1);
  CHECK(ret >= 0, lblKO);

  while(utaGclDialInfo() != gclStaFinished) {
      CHECK(kbdKey() == 0, lblKO);
      CHECK(tmrGet(2), lblKO);
  }

  ret = utaGclDialErr();
  CHECK(ret >= 0, lblKO);

  ret = sendRequest();
  */
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    return ret;
}

void tcik0170(void) {
//#ifdef __PGN_GSM__
    int ret;
    int err;
    char buf[dspW + 1];
    byte connType;

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    memset(buf, 0, sizeof(buf));

    //ret = ConnectSocket();
    //CHECK(ret < 0, lblEnd); //No treatment if no error

    ret = pgnLastErr(&connType);
    sprintf(buf, "pgnLastErr = %ld", ret);
    prtS(buf);
    dspLS(0, buf);
    CHECK(ret < 0, lblEnd);     //No treatment if no error

    err = ret;

    ret = dspLS(0, "Error code:");
    CHECK(ret >= 0, lblKO);

    switch (connType) {
      case pgnConnNull:        //General error
          prtS("General error");
          break;
      case pgnConnGsm:         // GSM Connection Error
          prtS("pgnConnGsm");
          ret = treatGsmErr(err);
          break;
      case pgnConnGprs:        //  GPRS Connection Error
          prtS("pgnConnGprs");
          ret = treatGprsErr(err);
          break;
      default:
          break;
    }
    tmrPause(1);
  lblKO:
    goto lblEnd;
  lblEnd:
    //tmrPause(3);
    prtStop();
    dspStop();
//#else
    //return;
//#endif
}
