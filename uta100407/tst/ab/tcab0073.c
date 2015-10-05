#include <string.h>
#include "pri.h"
#include "tst.h"

static code const char *srcFile = __FILE__;

//Integration test case tcab0073.
//Testing a ISO8583 transaction by PPP with RAWBANK

static int appLLLVAR(byte * map, tBuffer * req, byte bit, const char *val) {
    int ret;
    byte tmp;
    char buf[3 + 1];

    VERIFY(map);
    VERIFY(req);
    VERIFY(val);
    bitOn(map, bit);
    tmp = strlen(val);
    num2dec(buf, tmp, 3);       //LLVAR
    ret = bufApp(req, (byte *) buf, 3);
    if(ret < 0)
        return ret;
    ret = bufApp(req, (byte *) val, tmp);
    if(ret < 0)
        return ret;
    return 3 + tmp;
}

static int appLLVAR(byte * map, tBuffer * req, byte bit, const char *val) {
    int ret;
    byte tmp;
    char buf[2 + 1];

    VERIFY(map);
    VERIFY(req);
    VERIFY(val);
    bitOn(map, bit);
    tmp = strlen(val);
    num2dec(buf, tmp, 2);       //LLVAR
    ret = bufApp(req, (byte *) buf, 2);
    if(ret < 0)
        return ret;
    ret = bufApp(req, (byte *) val, tmp);
    if(ret < 0)
        return ret;
    return 2 + tmp;
}

static int appVal(byte * map, tBuffer * req, byte bit, const char *val) {
    int ret;

    VERIFY(map);
    VERIFY(req);
    VERIFY(val);
    bitOn(map, bit);
    ret = bufApp(req, (byte *) val, 0);
    return ret;
}

static int appPan(byte * map, tBuffer * req, const char *val) { //2,PAN,LLVAR
    VERIFY(map);
    VERIFY(req);
    VERIFY(val);
    return appLLVAR(map, req, 2, val);
}

static int appPrcCod(byte * map, tBuffer * req, const char *val) {  //3,Processing Code,n6
    VERIFY(strlen(val) == 6);
    return appVal(map, req, 3, val);
}

static int appAmt(byte * map, tBuffer * req, const char *val) { //4,Amount,n12
    VERIFY(strlen(val) == 12);
    return appVal(map, req, 4, val);
}

static int appDatTim(byte * map, tBuffer * req, const char *val) {  //7,Transaction date and time,MMDDhhmmss
    VERIFY(strlen(val) == 10);
    return appVal(map, req, 7, val);
}

static int appStan(byte * map, tBuffer * req, const char *val) {    //11,STAN,n6
    VERIFY(strlen(val) == 6);
    return appVal(map, req, 11, val);

}

static int appTime(byte * map, tBuffer * req, const char *val) {    //12,Time,local transaction,n6 hhmmss
    VERIFY(strlen(val) == 6);
    return appVal(map, req, 12, val);

}

static int appDate(byte * map, tBuffer * req, const char *val) {    //13,Date,local transaction,n4 MMDD
    VERIFY(strlen(val) == 4);
    return appVal(map, req, 13, val);

}

static int appExpDat(byte * map, tBuffer * req, const char *val) {  //14,Date,expiration,n4
    VERIFY(strlen(val) == 4);
    return appVal(map, req, 14, val);

}

static int appPosEntMod(byte * map, tBuffer * req, const char *val) {   //22,Point-of-Service Entry Mode,n3
    VERIFY(strlen(val) == 3);
    return appVal(map, req, 22, val);

}

static int appTrk2(byte * map, tBuffer * req, const char *val) {    //35,Track 2 Data,LLVAR
    VERIFY(map);
    VERIFY(req);
    VERIFY(val);
    return appLLVAR(map, req, 35, val);
}

static int appTid(byte * map, tBuffer * req, const char *val) { //41,Card Acceptor Terminal Identification,an8
    VERIFY(strlen(val) == 8);
    return appVal(map, req, 41, val);

}

static int appMid(byte * map, tBuffer * req, const char *val) { //42,Card Acceptor Identification Code,an15
    VERIFY(strlen(val) == 15);
    return appVal(map, req, 42, val);

}

static int appCur(byte * map, tBuffer * req, const char *val) { //49,Currency Code, Transaction,n3
    VERIFY(strlen(val) == 3);
    return appVal(map, req, 49, val);

}

static int appSecRelControl(byte * map, tBuffer * req, const char *val) {   //53,Security Related Control,n16
    VERIFY(strlen(val) == 16);
    return appVal(map, req, 53, val);

}

static int appNetMngmt(byte * map, tBuffer * req, const char *val) {    //70,Network Management Information Code,n3
    VERIFY(strlen(val) == 3);
    return appVal(map, req, 70, val);

}

static int appNetMngmtS_125(byte * map, tBuffer * req, const char *val) {   //125,Network Management Information Code S_125,n3
    VERIFY(strlen(val) == 3);
    return appLLLVAR(map, req, 125, val);

}

#define CHK CHECK(ret>=0,lblKO)
static buildMsg100(tBuffer * req, word mti) {
    int ret;
    byte bin[8];
    char tmp[16 + 1];
    card len;

    memset(bin, 0, 8);
    ret = appPan(bin, req, "5311307000011314");
    CHK;
    VERIFY(bitTest(bin, 2));
    ret = appPrcCod(bin, req, "000000");
    CHK;
    VERIFY(bitTest(bin, 3));
    ret = appAmt(bin, req, "000000000150");
    CHK;
    VERIFY(bitTest(bin, 4));
    ret = appDatTim(bin, req, "0708153343");
    CHK;
    VERIFY(bitTest(bin, 7));
    ret = appStan(bin, req, "000095");
    CHK;
    VERIFY(bitTest(bin, 11));   //to be incremented each time
    ret = appTime(bin, req, "153343");
    CHK;
    VERIFY(bitTest(bin, 12));
    ret = appDate(bin, req, "0708");
    CHK;
    VERIFY(bitTest(bin, 13));
    ret = appExpDat(bin, req, "0610");
    CHK;
    VERIFY(bitTest(bin, 14));
    ret = appPosEntMod(bin, req, "902");
    CHK;
    VERIFY(bitTest(bin, 22));
    ret = appTrk2(bin, req, "5311307000011314=0610101261700032");
    CHK;
    VERIFY(bitTest(bin, 35));
    ret = appTid(bin, req, "00000001");
    CHK;
    VERIFY(bitTest(bin, 41));
    ret = appMid(bin, req, "000000000000004");
    CHK;
    VERIFY(bitTest(bin, 42));
    ret = appCur(bin, req, "840");
    CHK;
    VERIFY(bitTest(bin, 49));

//insert bitmap at the beginning of the message
    bin2hex(tmp, bin, 8);
    bufIns(req, 0, (byte *) tmp, 16);

    //insert mti at the beginning of the request
    num2dec(tmp, mti, 4);
    bufIns(req, 0, (byte *) tmp, 4);

    //convert len into hex format and put it at the beginning of the request buffer
    len = bufLen(req);
    num2bin(bin, len, 2);
    bufIns(req, 0, bin, 2);

    trcFS("req=%s\n", (char *) bufPtr(req) + 2);
    trcBN(bufPtr(req), bufLen(req));
    trcS("\n");
    return bufLen(req);
  lblKO:
    return -1;
}

static buildMsg800(tBuffer * req, word mti) {
    int ret;
    byte bin[8];
    char tmp[16 + 1];
    card len;

    memset(bin, 0, 8);
    ret = appPrcCod(bin, req, "TL0000");
    CHK;
    VERIFY(bitTest(bin, 3));
    ret = appDatTim(bin, req, "0519122332");
    CHK;
    VERIFY(bitTest(bin, 7));
    ret = appStan(bin, req, "000079");
    CHK;
    VERIFY(bitTest(bin, 11));   //to be incremented each time
    ret = appTid(bin, req, "00000003");
    CHK;
    VERIFY(bitTest(bin, 41));
    ret = appSecRelControl(bin, req, "1111111111111111");
    CHK;
    VERIFY(bitTest(bin, 53));
    ret = appNetMngmt(bin, req, "171");
    CHK;
    VERIFY(bitTest(bin, 70));
    ret = appNetMngmtS_125(bin, req, "CBDB06520D8B21AC76FB");
    CHK;
    VERIFY(bitTest(bin, 125));

    //insert bitmap at the beginning of the message
    //bin2hex(tmp,bin,8);
    bufIns(req, 0, (byte *) "0400000000000008", 16);
    bufIns(req, 0, (byte *) "A220000000800800", 16);

    //insert mti at the beginning of the request
    num2dec(tmp, mti, 4);
    bufIns(req, 0, (byte *) tmp, 4);

    //convert len into hex format and put it at the beginning of the request buffer
    len = bufLen(req);
    num2bin(bin, len, 2);
    bufIns(req, 0, bin, 2);

    trcFS("req=%s\n", (char *) bufPtr(req) + 2);
    trcBN(bufPtr(req), bufLen(req));
    trcS("\n");
    return bufLen(req);
  lblKO:
    return -1;
}

/*
static int makeIso(tBuffer *req,word msg){
    int ret;
    VERIFY(msg==100);
    ret= bufApp(req,(byte *)"\x00\x96",0); CHK; //Length
    ret= bufApp(req,(byte *)"0100",0); CHK; //MTI, Message Type Identifier
    ret= bufApp(req,(byte *)"723C040020C08000",0); CHK; //Bitmap
    //72 3C 04 00 20 C0 80 00:
    //72:0111 0010 2,3,4,7
    //3C:0011 1100 11,12,13,14
    //04:0000 0100 22
    //00:0000 0000
    //20:0010 0000 35
    //C0:1100 0000 41,42
    //80:1000 0000 49
    //00:0000 0000
    ret= bufApp(req,h(byte *)"165311307000011314",0); CHK; //2,PAN,LLVAR
    ret= bufApp(req,(byte *)"000000",0); CHK; //3,Processing Code,n6
    ret= bufApp(req,(byte *)"000000000150",0); CHK; //4,Amount,n12
    ret= bufApp(req,(byte *)"0519153343",0); CHK; //7,Transaction date and time,MMDDhhmmss
    ret= bufApp(req,(byte *)"000094",0); CHK; //11,STAN,n
    ret= bufApp(req,(byte *)"153343",0); CHK; //12,Time,local transaction,n6 hhmmss
    ret= bufApp(req,(byte *)"0519",0); CHK; //13,Date,local transaction,n4 MMDD
    ret= bufApp(req,(byte *)"0610",0); CHK; //14,Date,expiration,n4
    ret= bufApp(req,(byte *)"902",0); CHK; //22,Point-of-Service Entry Mode,n3
    ret= bufApp(req,(byte *)"335311307000011314=0610101261700032",0); CHK; //35,Track 2 Data,LLVAR
    ret= bufApp(req,(byte *)"00000001",0); CHK; //41,Card Acceptor Terminal Identification,an8
    ret= bufApp(req,(byte *)"000000000000004",0); CHK; //42,Card Acceptor Identification Code,an15
    ret= bufApp(req,(byte *)"840",0); CHK; //49,Currency Code, Transaction,n3
    
    return bufLen(req);
lblKO:
    return -1;
}
*/

static parseMsg(tBuffer * rsp) {
    byte map[8];
    word len;
    card mti;
    const byte *ptr;
    byte idx;

    ptr = bufPtr(rsp);
    len = ptr[0] * 256 + ptr[1];
    CHECK(2 + len == bufLen(rsp), lblKO);

    ptr += 2;
    dec2num(&mti, (char *) ptr, 4);
    trcFN("Message type received mti=%d\n", mti);

    ptr += 4;
    hex2bin(map, (char *) ptr, 8);
    trcS("Bit map received:");
    trcBN(map, 8);
    trcS("[");
    for (idx = 1; idx <= 64; idx++) {
        if(!bitTest(map, idx))
            continue;
        trcFN(" %d", idx);
    }
    trcS("]\n");

    return bufLen(rsp);
  lblKO:
    return -1;
}

static int sendMsg(const tBuffer * req) {
    int ret;

    VERIFY(req);
//    ret= comSendReq(bufPtr(req),(word)(bufLen(req)+1));
//    CHECK(ret==bufLen(req)+1,lblKO);    
//    return bufLen(req)+1;
    ret = comSendReq(bufPtr(req), bufLen(req));
    CHECK(ret == bufLen(req), lblKO);
    return bufLen(req);
  lblKO:
    return -1;
}

static int recvRsp(tBuffer * rsp) {
    int ret;
    word len;
    byte tmp;

    VERIFY(rsp);

    ret = comRecv(&tmp, 10);
    CHECK(ret >= 0, lblKO);
    bufApp(rsp, &tmp, 1);
    len = tmp;

    ret = comRecv(&tmp, 10);
    CHECK(ret >= 0, lblKO);
    bufApp(rsp, &tmp, 1);
    len = len * 256 + tmp;
    trcFN("len=%d\n", len);

    while(len--) {
        ret = comRecv(&tmp, 10);
        CHECK(ret >= 0, lblKO);
        bufApp(rsp, &tmp, 1);
    }
    trcFS("rsp= %s\n", (char *) bufPtr(rsp) + 2);
    trcBN(bufPtr(rsp), bufLen(rsp));
    trcS("\n");

    return bufLen(rsp);
  lblKO:
    return -1;
}

static int connectRawbank(void) {
    int ret;

#ifdef WIN32
    const byte chn = chnMdm;
    const char *init = "8N11200";
    const char *adr = "0-00-243-811559000";
#else
    const byte chn = chnPpp;
    const char *init = "0,00243811559000|danfr|onacti|60";
    const char *adr = "192.168.96.22-7555";
#endif

    ret = dspLS(0, "comStart...");
    CHECK(ret >= 0, lblKO);
    TRCDT("comStart beg");
    ret = comStart(chn);
    CHECK(ret >= 0, lblKO);
    TRCDT("comStart end");
    ret = dspLS(0, "comStart OK");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "comSet...");
    CHECK(ret >= 0, lblKO);
    TRCDT("comSet beg");
    ret = comSet(init);
    CHECK(ret >= 0, lblKO);
    TRCDT("comSet end");
    ret = dspLS(1, "comSet OK");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, "comDial...");
    CHECK(ret >= 0, lblKO);
    ret = dspLS(3, adr);
    CHECK(ret >= 0, lblKO);

    TRCDT("comDial beg");
    ret = comDial(adr);
    TRCDT("comDial end");
    CHECK(ret >= 0, lblKO);
    ret = dspLS(3, "comDial OK");
    CHECK(ret >= 0, lblKO);
    return 1;
  lblKO:
    return -1;
}

static int disconnect(void) {
    int ret;

    ret = comHangStart();
    CHECK(ret >= 0, lblKO);
    ret = comHangWait();
    CHECK(ret >= 0, lblKO);
    ret = comStop();
    CHECK(ret >= 0, lblKO);
    return 1;
  lblKO:
    return -1;
}

static int test100(void) {
    int ret;
    byte dMsg[512];
    tBuffer bMsg;

    bufInit(&bMsg, dMsg, 512);
    ret = buildMsg100(&bMsg, 100);
    CHECK(ret > 0, lblKO);

    ret = connectRawbank();
    CHECK(ret > 0, lblKO);

    ret = dspLS(1, "send req");
    CHECK(ret >= 0, lblKO);
    ret = sendMsg(&bMsg);
    CHECK(ret > 0, lblKO);
    ret = dspLS(1, "req OK");
    CHECK(ret >= 0, lblKO);

    bufReset(&bMsg);
    ret = dspLS(2, "wait for rsp");
    CHECK(ret >= 0, lblKO);
    ret = recvRsp(&bMsg);
    CHECK(ret > 0, lblKO);
    ret = dspLS(2, "rsp OK");
    CHECK(ret >= 0, lblKO);

    ret = disconnect();
    CHECK(ret > 0, lblKO);

    ret = parseMsg(&bMsg);
    CHECK(ret > 0, lblKO);

    return 1;
  lblKO:
    return -1;
}

static int test800(void) {
    int ret;
    byte dMsg[512];
    tBuffer bMsg;

    bufInit(&bMsg, dMsg, 512);
    ret = buildMsg800(&bMsg, 800);
    CHECK(ret > 0, lblKO);

    ret = connectRawbank();
    CHECK(ret > 0, lblKO);

    ret = dspLS(1, "send req");
    CHECK(ret >= 0, lblKO);
    ret = sendMsg(&bMsg);
    CHECK(ret > 0, lblKO);
    ret = dspLS(1, "req OK");
    CHECK(ret >= 0, lblKO);

    bufReset(&bMsg);
    ret = dspLS(2, "wait for rsp");
    CHECK(ret >= 0, lblKO);
    ret = recvRsp(&bMsg);
    CHECK(ret > 0, lblKO);
    ret = dspLS(2, "rsp OK");
    CHECK(ret >= 0, lblKO);

    ret = disconnect();
    CHECK(ret > 0, lblKO);

    ret = parseMsg(&bMsg);
    CHECK(ret > 0, lblKO);

    return 1;
  lblKO:
    return -1;
}

void tcab0073(void) {
    int ret;

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "Start tracing...");
    CHECK(ret >= 0, lblKO);
#ifdef WIN32
    trcMode(0xFF);
#else
    trcMode(1);
#endif
    ret = dspLS(0, "Testing...");
    CHECK(ret >= 0, lblKO);

#ifdef __Msg_100_
    ret = 100;
#endif
#ifdef __Msg_800_
    ret = 800;
#endif

    switch (ret) {
      case 100:
          ret = test100();
          break;
      case 800:
          ret = test800();
          break;
      default:
          ret = -1;
          break;
    }
    CHECK(ret > 0, lblKO);

    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO!");
    tmrPause(3);
  lblEnd:
    tmrPause(2);
    comStop();
    prtStop();
    dspStop();
}
