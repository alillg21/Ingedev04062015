#include <string.h>
#include "pri.h"
#include "tst.h"

//Unitary test case tcab0144.
//Connectivity test

#define CHK CHECK(ret>=0,lblKO)
static int makeHdr(tBuffer * req) {
    int ret;
    char dt[12 + 1];

    ret = bufApp(req, (byte *) "9.", 0);
    CHK;                        //Device Type
    ret = bufApp(req, (byte *) "00", 0);
    CHK;                        //Transmission Number
    ret = bufApp(req, (byte *) "ING01           ", 0);
    CHK;                        //Terminal ID
    ret = bufApp(req, (byte *) "      ", 0);
    CHK;                        //Employee ID
    ret = getDateTime(dt);
    ret = bufApp(req, (byte *) dt, 0);
    CHK;                        //Current Date/Time
    ret = bufApp(req, (byte *) "A", 0);
    CHK;                        //Message Type
    ret = bufApp(req, (byte *) "O", 0);
    CHK;                        //Message SubType
    ret = bufApp(req, (byte *) "95", 0);
    CHK;                        //Transaction Code
    ret = bufApp(req, (byte *) "000", 0);
    CHK;                        //Processing Flags
    ret = bufApp(req, (byte *) "000", 0);
    CHK;                        //Response Code
    return bufLen(req);
  lblKO:
    return -1;
}

static int sendReq1(void) {
    int ret;
    byte lrc;
    byte dReq[512];
    tBuffer bReq;

    bufInit(&bReq, dReq, 512);
    //ret= bufApp(&bReq,bPtr(cSTX),1); CHK;
    ret = makeHdr(&bReq);
    CHK;

/*
    ret= bufApp(&bReq,bPtr(cFS),1); CHK;
    ret= bufApp(&bReq,(byte *)"V",0); CHK; //Amount 1
    ret= bufApp(&bReq,(byte *)"00",0); CHK; //Category Code
    ret= bufApp(&bReq,(byte *)"1",0); CHK; //Acces Code
    ret= bufApp(&bReq,(byte *)"00",0); CHK; //Processing Flag
    ret= bufApp(&bReq,(byte *)"0000000000",0); CHK; //Filler
*/
    ret = bufApp(&bReq, bPtr(cETX), 1);
    CHK;
    lrc = stdLrc(bufPtr(&bReq) + 1, bufLen(&bReq) - 1); //calculate LRC
    //ret= bufApp(&bReq,&lrc,1); CHK;//append LRC
    ret = comSendReq(bufPtr(&bReq), bufLen(&bReq)); //send all the pack STX-DATA-ETX-LRC
    CHECK(ret == bufLen(&bReq), lblKO);
    return bufLen(&bReq);
  lblKO:
    return -1;
}

static int recvRsp(void) {
    int ret;
    byte dRsp[512];
    tBuffer bRsp;

    bufInit(&bRsp, dRsp, 512);
    ret = comRecvStxDataEtxLrc(&bRsp, 60);  //receive package
    CHECK(ret >= 0, lblKO);
    return bufLen(&bRsp);
  lblKO:
    return -1;
}

void tcab0144(void) {
    int ret;
    int idx;
    byte b;
    char *ptr;
    char msg[256];

    //tBuffer buf;
    //byte dat[256];

    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    memset(msg, 0, dspW + 1);
    ret = comStart(chnMdm);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "comSet...");
    CHECK(ret >= 0, lblKO);
    ptr = "8N12400";            //SATIM
    //ptr= "8N12400"; //Oxigen
    //ptr = "7E11200";            //ATOS
//      ptr= "7E12400"; //SMT
//      ptr= "7E11200"; //SG
    ret = dspLS(3, ptr);
    CHECK(ret >= 0, lblKO);

    TRCDT("comSet...");
    ret = comSet(ptr);
    CHECK(ret >= 0, lblKO);
    TRCDT("comSet done");

    ret = dspLS(0, "comSet OK");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "comDial...");
    CHECK(ret >= 0, lblKO);
    //ptr= "0-00-302109503410"; //Greece
    //ptr= "0-4117133"; //Oxigen
    //ptr = "0-0836065555";       //France
    //ptr= "0-00-914422540268"; //Delhi
    //ptr= "0-00-2218399550"; //Dakar
    //ptr= "0-00-21671951036"; //Tunis
    //ptr= "0-00-97165739374"; //Dubai
    //ptr= "0-00-3614212700"; //Hungary
    //ptr= "0-00-243811559000"; //Kongo
    ptr = "0-00-21321448641";   //Algerie
    ret = dspLS(3, ptr);
    CHECK(ret >= 0, lblKO);

    TRCDT("comDial...");
    ret = comDial(ptr);
    if(ret < 0) {
        switch (-ret) {
          case comAnn:
              ret = dspLS(2, "comDial Aborted");
              break;
          case comBusy:
              ret = dspLS(2, "comDial BUSY");
              break;
          case comNoDialTone:
              ret = dspLS(2, "NO TONE");
              break;
          case comNoCarrier:
              ret = dspLS(2, "NO CARRIER");
              break;
          default:
              ret = dspLS(2, "UNKNOWN PB");
              break;
        }
        goto lblEnd;
    }
    CHECK(ret >= 0, lblKO);
    TRCDT("comDial done");
    ret = dspLS(1, "comDial OK");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(3, "Receiving per char");
    CHECK(ret >= 0, lblKO);
    for (idx = 0; idx < 17; idx++) {
        ret = comRecv(&b, 20);  //receive characters
        if(ret < 0)
            break;
        //bin2hex(msg + idx * 2, &b, 2);
        //dspLS(2, msg);
    }
    ret = dspLS(3, "Receiving done");
    CHECK(ret >= 0, lblKO);

    comSendBuf((byte *) "110500223\x0D\x0A", 11);

    CHECK(ret >= 0, lblKO);
    for (idx = 0; idx < 21; idx++) {
        ret = comRecv(&b, 20);  //receive characters
        if(ret < 0)
            break;
        bin2hex(msg + idx * 2, &b, 2);
        dspLS(2, msg);
    }
    ret = dspLS(3, "Receiving done");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "send req 1");
    CHECK(ret >= 0, lblKO);
    ret = sendReq1();
    CHECK(ret > 0, lblKO);
    ret = dspLS(0, "req1 OK");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "wait for rsp 1");
    CHECK(ret >= 0, lblKO);
    ret = recvRsp();
    CHECK(ret > 0, lblKO);
    ret = dspLS(1, "rsp 1 OK");
    CHECK(ret >= 0, lblKO);

    ret = comHangStart();
    CHECK(ret >= 0, lblKO);

    ret = comHangWait();
    CHECK(ret >= 0, lblKO);

    ret = comStop();            //close the port
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    trcErr(ret);
    dspLS(0, "KO!");
  lblEnd:
    tmrPause(3);
    comStop();
    dspStop();
}
