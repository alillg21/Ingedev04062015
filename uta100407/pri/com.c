/** \file
 * Generic communication functions primitives
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/pri/com.c $
 *
 * $Id: com.c 2616 2010-04-07 11:16:41Z abarantsev $
 */

#include <string.h>
#include "pri.h"

#undef trcOpt
#define trcOpt (TRCOPT & BIT(trcCOM))

static char comSettings[32 + 1];    //input data for task
static int comRet;              //task return code

#ifdef __UNICAPT__
static void taskComSet(card arg) {
    VERIFY(comSettings);
    trcS("taskComSet Beg\n");
    if(arg) {                   //started as parallel task
        semDec(1);              //inform the parent task that the task is started
        semInc(2);              //wait while the parent task gives the control on the com port    
    }
    comRet = comSet(comSettings);
    trcS("taskComSet End\n");
    if(arg) {                   //started as parallel task
        tskComCtl(0);           //return the control on the comport to the parent
        semDec(0);              //release semaphore
        tskStop();              //delete task
    }
}
#endif

static int comConnectMdm(const char *chn, const char *srv) {
    int ret = -comKO;

    if(chn) {
        trcFS("comConnectMdm chn=%s\n", chn);
    }
    if(srv) {
        trcFS("comConnectMdm srv=%s\n", srv);
    }

    if(chn) {
        VERIFY(strlen(chn) < 32);
        strcpy(comSettings, chn);
        ret = comStart(chnMdm);
        CHECK(ret >= 0, lblKO);
        if(!srv) {              //start come settings initialisation only
#ifdef __UNICAPT__
            ret = tskStart(taskComSet, 1);
            CHECK(ret >= 0, lblKO);
            semInc(1);          //wait to be sure that the task is really started    
            ret = tskComCtl(1); //now give the control on comport to the task
            CHECK(ret >= 0, lblKO);
            semDec(2);          //inform the task that it can perform the job now
#endif
            goto lblEnd;
        }
        comRet = comSet(comSettings);
    }

    if(srv) {
        if(!chn) {              //com settings was started in background
#ifdef __UNICAPT__
            ret = semInc(0);    //waiting for semDec(0) call from taskComSet
            CHECK(ret >= 0, lblKO);
#endif
        }
        ret = comRet;
        CHECK(ret >= 0, lblKO);

        ret = comDial(srv);     //call the server

        CHECK(ret >= 0, lblEnd);
    }

    goto lblEnd;
  lblKO:
    ret = -comKO;
  lblEnd:
    trcFN("comConnectMdm ret=%d\n", ret);
    return ret;
}

static int comConnectTcp(byte tcp, const char *chn, const char *srv) {
    int ret;

    VERIFY(chn);
    VERIFY(srv);
    trcFS("comConnectTcp chn=%s\n", chn);
    trcFS("comConnectTcp srv=%s\n", srv);

    VERIFY(strlen(chn) < 32);
    strcpy(comSettings, chn);
    ret = comStart(tcp);
    CHECK(ret >= 0, lblKO);
    ret = comSet(chn);
    CHECK(ret >= 0, lblKO);
    ret = comDial(srv);
    CHECK(ret >= 0, lblEnd);

    goto lblEnd;
  lblKO:
    ret = -comKO;
  lblEnd:
    trcFN("comConnectTcp ret=%d\n", ret);
    return ret;
}

static int comConnectCom(byte comport, const char *chn, const char *srv) {
    int ret;

    VERIFY(chn);
    VERIFY(srv);
    trcFS("comConnectCom chn=%s\n", chn);
    trcFS("comConnectCom srv=%s\n", srv);

    VERIFY(strlen(chn) < 32);
    strcpy(comSettings, chn);
    ret = comStart(comport);
    CHECK(ret >= 0, lblKO);
    ret = comSet(chn);
    CHECK(ret >= 0, lblKO);

    ret = comDial(srv);
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    ret = -comKO;
  lblEnd:
    trcFN("comConnectCom ret=%d\n", ret);
    return ret;
}

static int comConnectGprs(const char *chn, const char *srv) {
    int ret;

    VERIFY(chn);
    VERIFY(srv);
    trcFS("comConnectGprs chn=%s\n", chn);
    trcFS("comConnectGprs srv=%s\n", srv);

    VERIFY(strlen(chn) < 32);
    strcpy(comSettings, chn);
    ret = comStart(chnGprs);
    CHECK(ret >= 0, lblKO);
    ret = comSet(chn);
    CHECK(ret >= 0, lblKO);
    ret = comDial(srv);
    CHECK(ret >= 0, lblEnd);

    goto lblEnd;
  lblKO:
    ret = -comKO;
  lblEnd:
    trcFN("comConnectGprs ret=%d\n", ret);
    return ret;
}

static int comConnectHdlc(const char *chn, const char *srv) {
    int ret;

    VERIFY(chn);
    VERIFY(srv);
    trcFS("comConnectHdlc chn=%s\n", chn);
    trcFS("comConnectHdlc srv=%s\n", srv);

    VERIFY(strlen(chn) < 32);
    strcpy(comSettings, chn);
    ret = comStart(chnHdlc);
    CHECK(ret >= 0, lblKO);
    ret = comSet(chn);
    CHECK(ret >= 0, lblKO);
    ret = comDial(srv);
    CHECK(ret >= 0, lblEnd);

    goto lblEnd;
  lblKO:
    ret = -comKO;
  lblEnd:
    trcFN("comConnectHdlc ret=%d\n", ret);
    return ret;
}

/** Initializes a communication line com and/or connects to server according to the control
 * strings chn and srv.
 * The control strings depend on the communication line used.
 * Now only internal modem communication line (com==0) is implemented.
 * The control string chn is used to initialise the communication line.
 * For internal modem it is:
 *  - chn[0]: number of data bits in ASCII (‘7’ or ‘8’)
 *  - chn[1]: parity (‘N’,’E’ or ‘O’)
 *  - chn[2]: number of stop bits in ASCII(‘0’,’1’ or ‘2’)
 *  - chn+3: baud rate in ASCI (1200,9400 or 9600)
 * The srv control string is used to perform the connection with the server.
 * For dialup connection it is:
 *  - phone number in the same format as for comDial function
 * The operation can be performed in two phases:
 *  - if srv is zero pointer, only communication line initialization is started as a background task
 *  - if chn is zero pointer, the function waits while communication line initialization finishes
 *    and performs connection
 *  - if both control strings are not zero,
 *    the operations are performed consecutively without any background tasking
 * \param  com (I) Communication line code (0= internal modem)
 * \param  chn (I) Communication line initialization control string
 * \param  srv (I) Server connection control string
 * \return 
 *    - non-negative value if OK.
 *    - otherwise the return code is a negative value according to: 
 *        - comTimeout : timeout of 1 minute
 *        - comAnn : operation aborted by user pressing red CLEAR key
 *        - comNoCarrier: NO CARRIER
 *        - comNoDialTone: NO DIAL TONE
 *        - comBusy: BUSY
 *        - comKO: any other problem
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\com.c
*/
int comConnect(char com, const char *chn, const char *srv) {
    switch (com) {
      case 'M':
          return comConnectMdm(chn, srv);   //internal modem connection
      case 'N':
          return comConnectTcp(chnTcp5100, chn, srv);   //network TCP/IP connection I5100 with ethernet card
      case 'n':
          return comConnectTcp(chnTcp7780, chn, srv);   //network TCP/IP connection I7780 bluetooth
      case '1':
          return comConnectCom(chnCom1, chn, srv);  //Comport1 connection
      case '2':
          return comConnectCom(chnCom2, chn, srv);  //Comport2 connection
      case 'G':
          return comConnectGprs(chn, srv);  //GPRS connection
      case 'H':
          return comConnectHdlc(chn, srv);  //HDLC connection
      default:
          break;
    }
    return -comKO;
}

static int comDisconnectMdm(const char *chn, const char *srv) {
    int ret;

    if(chn) {
        trcFS("comDisconnectMdm chn=%s\n", chn);
    }
    if(srv) {
        trcFS("comDisconnectMdm srv=%s\n", srv);
    }

    if(srv)
        comHangStart();
    if(chn) {
        comHangWait();
        comStop();
    }

    ret = 1;
    goto lblEnd;

  lblEnd:
    trcFN("comDisconnectMdm ret=%d\n", ret);
    return ret;
}

static int comDisconnectTcp(const char *chn, const char *srv) {
    int ret;

    if(chn) {
        trcFS("comDisconnectTcp chn=%s\n", chn);
    }
    if(srv) {
        trcFS("comDisconnectTcp srv=%s\n", srv);
    }

    comHangStart();
    comHangWait();
    comStop();

    ret = 1;
    goto lblEnd;

  lblEnd:
    trcFN("comDisconnectTcp ret=%d\n", ret);
    return ret;
}

static int comDisconnectCom(const char *chn, const char *srv) {
    int ret;

    if(chn) {
        trcFS("comDisconnectCom chn=%s\n", chn);
    }
    if(srv) {
        trcFS("comDisconnectCom srv=%s\n", srv);
    }

    comHangStart();
    comHangWait();
    comStop();

    ret = 1;
    goto lblEnd;

  lblEnd:
    trcFN("comDisconnectCom ret=%d\n", ret);
    return ret;
}

static int comDisconnectGprs(const char *chn, const char *srv) {
    int ret;

    if(chn) {
        trcFS("comDisconnectGprs chn=%s\n", chn);
    }
    if(srv) {
        trcFS("comDisconnectGprs srv=%s\n", srv);
    }

    comHangStart();
    comHangWait();
    comStop();

    ret = 1;
    goto lblEnd;

  lblEnd:
    trcFN("comDisconnectGprs ret=%d\n", ret);
    return ret;
}

static int comDisconnectHdlc(const char *chn, const char *srv) {
    int ret;

    if(chn) {
        trcFS("comDisconnectHdlc chn=%s\n", chn);
    }
    if(srv) {
        trcFS("comDisconnectHdlc srv=%s\n", srv);
    }

    comHangStart();
    comHangWait();
    comStop();

    ret = 1;
    goto lblEnd;

  lblEnd:
    trcFN("comDisconnectHdlc ret=%d\n", ret);
    return ret;
}

/** Disconnect from the server.
 * The first parameter is the communication line code. Only zero value (internal modem) is supported.
 * If srv is not zero pointer, the function starts hang up operation.
 * If ctl is not zero pointer, the function waits while the hang up is finished and closes the com port.
 * \param com (I) Communication line code (0= internal modem)
 * \param       chn (I) Communication line initialization control string
 * \param       srv (I) server connection control string
 * \return non-negative value if OK.
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\com.c
*/
int comDisconnect(char com, const char *chn, const char *srv) {
    switch (com) {
      case 'M':
          return comDisconnectMdm(chn, srv);    //internal modem disconnection
      case 'N':
          return comDisconnectTcp(chn, srv);    //network TCP/IP disconnection
      case '1':
      case '2':
          return comDisconnectCom(chn, srv);    //ComPort disconnection
      case 'G':
          return comDisconnectGprs(chn, srv);   //GPRS disconnection
      case 'H':
          return comDisconnectHdlc(chn, srv);   //HDLC disconnection
      default:
          break;
    }
    return -comKO;
}

/** Send request message req of length len.
 * \param req (I) Request message to send
 * \param len (I) number of characters to send
 * \return non-negative value if OK.
 * \pre 
 *    - req!=0
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\com.c
*/
int comSendReq(const byte * req, word len) {
    int ret;

    ret = comSendBuf(req, len);
    CHECK(ret >= 0, lblKO);

    ret = len;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    trcFN("comSendReq ret=%d\n", ret);
    return ret;
}

/** Receive response message terminated by a control character in trm into buffer rsp using timeout dly. The reception stops when one of the characters in trm is received. If rsp is not zero the characters received are accumulated in it; otherwise they are skipped.
 * \param rsp (O) Pointer to a buffer to accumulate the characters received; or zero
 * \param trm (I) Zero-ended terminator character string
 * \param dly (I) reception timeout
 * \return
 *    - number of characters received if OK.
 *    - 0 if timeout.
 *    - negative in case of reception problem.
 * \pre
 *    - trm!=0
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\com.c
 */
int comRecvRsp(tBuffer * rsp, const byte * trm, byte dly) {
    int ret;

    //VERIFY(trm);//With this line we can not send 0 as terminator. We need zero when there is no no trminator in the message and we have to receive all the buffer.

    ret = comRecvBuf(rsp, trm, dly);
    CHECK(ret >= 0, lblKO);

    if(rsp)
        ret = bufLen(rsp);
    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    tmrStop(1);
    trcFN("comRecvRsp ret=%d\n", ret);
    return ret;
}

/** Receive a packet formatted as STX-DATA-ETX-LRC into buffer rsp using the timeout dly. The checksum lrc is not checked.
 * \param rsp (O) Pointer to a buffer to accumulate the characters received
 * \param dly (I) Reception timeout
 * \return
 *    - number of characters received if OK.
 *    - 0 if timeout.
 *    - negative in case of reception problem
 * \pre 
 *    - rsp!=0
 *    - trm!=0
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\com.c
 */
int comRecvStxDataEtxLrc(tBuffer * rsp, byte dly) {
    int ret;
    byte trm[] = { cENQ, cEOT, cDLE, cSTX, cETX, cNAK, 0x80 | cETX, 0x80 | cNAK, 0 };   //take in account parity bit
    byte rcv;

    VERIFY(rsp);

    bufReset(rsp);
    VERIFY(bufLen(rsp) == 0);

    while(1) {
        ret = comRecv(&rcv, dly);   //receive STX
        CHECK(ret == 1, lblKO);
        if((rcv & 0x7F) == cSTX)
            break;
    }
    rcv = cSTX;

    ret = bufApp(rsp, &rcv, 1);
    CHECK(ret == 1, lblKO);
    if(!((rcv != cSTX) ^ (rcv != (cSTX | 0x80)))) { //unexpected character - stop
        ret = 1;
        goto lblEnd;
    }

    ret = comRecvRsp(rsp, trm, dly);    //receive data
    CHECK(ret > 0, lblKO);
    VERIFY(bufLen(rsp) == ret);

    ret = comRecv(&rcv, dly);   //receive LRC
    CHECK(ret == 1, lblKO);

    ret = bufApp(rsp, &rcv, 1);
    CHECK(ret > 0, lblKO);
    VERIFY(bufLen(rsp) == ret);

    trcFS("comRecvStxDataEtxLrc rsp=%s\n", (char *) bufPtr(rsp));
    trcS("bin: ");
    trcBN(bufPtr(rsp), bufLen(rsp));
    trcS("\n");

    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    tmrStop(1);
    return ret;
}

int comRecvDataEOD(tBuffer * rsp, byte dly) {
    int ret;
    byte trm[] = { 0x0D, cPR, 0 };
    VERIFY(rsp);

    bufReset(rsp);
    VERIFY(bufLen(rsp) == 0);

    ret = comRecvRsp(rsp, trm, dly);    //receive data
    CHECK(ret > 0, lblKO);
    VERIFY(bufLen(rsp) == ret);

    trcFS("comRecvDataEOD rsp=%s\n", (char *) bufPtr(rsp));
    trcS("bin: ");
    trcBN(bufPtr(rsp), bufLen(rsp));
    trcS("\n");

    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    tmrStop(1);
    return ret;
}

/**
 * \todo To be documented
 */
int comRecvDataEtx(tBuffer * rsp, byte dly) {
    int ret;
    byte trm[] = { 0x0D, 0x05, 0x0A, cENQ, cEOT, cDLE, cETX, 0 };
    //byte trm[] = { 0x0D,0x05,0x0A 0 };
    VERIFY(rsp);

    bufReset(rsp);
    VERIFY(bufLen(rsp) == 0);

    ret = comRecvRsp(rsp, trm, dly);    //receive data
    CHECK(ret > 0, lblKO);
    VERIFY(bufLen(rsp) == ret);

    trcFS("comRecvDataEtx rsp=%s\n", (char *) bufPtr(rsp));
    trcS("bin: ");
    trcBN(bufPtr(rsp), bufLen(rsp));
    trcS("\n");

    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    tmrStop(1);
    return ret;
}

static byte patternFound(const tBuffer * buf, const tBuffer * pat) {
    const byte *ptr1;
    const byte *ptr2;
    word len1, len2, idx1, idx2;

    ptr1 = bufPtr(buf);
    len1 = bufLen(buf);
    ptr2 = bufPtr(pat);
    len2 = bufLen(pat);

    idx1 = len1;
    idx2 = len2;
    while(idx2--) {
        if(!idx1)
            return 0;
        idx1--;
        if(ptr1[idx1] != ptr2[idx2])
            return 0;
    }
    VERIFY(memcmp(ptr1 + len1 - len2, ptr2, len2) == 0);
    return 1;
}

/**
 * \todo To be documented
 */
int comRecvPat(tBuffer * rsp, const tBuffer * pat, byte dly) {
    int ret;
    word len;
    byte rcv;

    VERIFY(rsp);
    VERIFY(pat);

    ret = tmrStart(1, (word) dly * 100);
    CHECK(ret >= 0, lblKO);

    rcv = 0;
    len = 0;
    while(tmrGet(1)) {
        ret = comRecv(&rcv, 1); //receive character
        if(ret <= 0)
            continue;
        len++;
        ret = bufApp(rsp, &rcv, 1);
        CHECK(ret >= 0, lblKO);
        if(patternFound(rsp, pat))
            break;
    }
    if(!tmrGet(1))
        ret = 0;
    else
        ret = len;

    if(rsp) {
        trcFS("comRecvPat rsp=%s\n", (char *) bufPtr(rsp));
        trcS("bin: ");
        trcBN(bufPtr(rsp), bufLen(rsp));
        trcS("\n");
    } else {
        trcFS("comRecvPat pat=%s\n", (char *) bufPtr(pat));
        trcS("bin: ");
        trcBN(bufPtr(pat), bufLen(pat));
        trcS("\n");
    }

    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    tmrStop(1);
    trcFN("comRecvPat ret=%d\n", ret);
    return ret;
}

int chkStart(const char *para) {
    int ret = -comKO;

    ret = comStart(chnCom1);
    CHECK(ret >= 0, lblKOStart);
    ret = comSet(para);
    CHECK(ret >= 0, lblKOSet);
    ret = comDial("");
    CHECK(ret >= 0, lblKODial);
    goto lblEnd;
  lblKO:
    trcS("Pre-req Failed\n");
    goto lblEnd;
  lblKOStart:
    trcS("comStart Failed\n");
    goto lblEnd;
  lblKOSet:
    trcS("comSet Failed\n");
    goto lblEnd;
  lblKODial:
    trcS("comDial Failed\n");
    goto lblEnd;
  lblEnd:
    return ret;
}

int chkGet(byte * rawData) {
    int ret = -comKO;
    char msg[256];
    tBuffer buf;
    byte dat[256];

    bufInit(&buf, dat, sizeof(dat));

    memset(msg, 0, sizeof(msg));
    ret = comRecvBuf(&buf, 0, 60);
    CHECK(ret >= 0, lblKOReceive);
    memcpy(rawData, bufPtr(&buf), bufLen(&buf));

    goto lblEnd;
  lblKOReceive:
    trcS("comReceive Failed\n");
    goto lblEnd;
  lblEnd:
    return ret;
}

int chkStop(void) {
    comHangStart();
    comHangWait();
    comStop();
    return 1;
}

int clsGet(const char *para, byte * buf) {
    int ret = 0;

    return ret;
}
