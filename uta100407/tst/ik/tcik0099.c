#include <string.h>
#include <unicapt.h>
#include "gcl_sock.h"
#include "gcl_pgsup.h"
#include "gcl_error.h"
//#include "log.h"
#include "gcl.h"

//Integration test case tcik0099.
//Application top5V
//Non UTA ,GMA plugin, handshake test

static uint32 displayHandle;
static uint32 prnHandle;
static char buffer[256];
static gclSock_t sck;

/*
static int16 GprsCheck(void){
	int16 ret;
	uint8 tmpRssi, tmpSta;
	int16 count;

    ret = gmaInqGsmSignLevel(&tmpRssi, &tmpSta);
    sprintf(buffer, "GsmSignLevel ret: %d ", ret);
  	hmiADDisplayText(displayHandle, 0, 0, buffer);
  	prnPrint(prnHandle, buffer);
	if(ret!=RET_OK) return -1;
	sprintf(buffer, "GsmSignLevel tmpSta: %d ", tmpSta);
  	hmiADDisplayText(displayHandle, 0, 0, buffer);
  	prnPrint(prnHandle, buffer);
	if(tmpSta==1) return 1;
	
	count = 60;
	ret = gclPgSupAttach(count*100);
	sprintf(buffer, "gclPgSupAttach ret: %d ", ret);
  	hmiADDisplayText(displayHandle, 0, 0, buffer);
  	prnPrint(prnHandle, buffer);
    if(ret<0) return -1;
    
    while(count--){
    	psyTimerWakeAfter(100);
    	ret = gmaInqGsmSignLevel(&tmpRssi, &tmpSta);
		if(ret!=RET_OK) return -1;
		if(tmpSta) return 1;
    }
    sprintf(buffer, "GsmSignLevel tmpSta: %d ", tmpSta);
  	hmiADDisplayText(displayHandle, 0, 0, buffer);
  	prnPrint(prnHandle, buffer);
  	
  	//gsmReset();
	return -1;
}
*/
static int16 DialInfo(void) {
    int16 ret;

    ret = gclTaskIsAlive();
    if(!ret)
        return 1;
    ret = gclTaskGetState();
    switch (ret) {
      case GCL_PREDIAL:
          ret = 1;
          sprintf(buffer, "GCL_PREDIAL : %d ", ret);
          break;
      case GCL_DIAL:
          ret = 2;
          sprintf(buffer, "GCL_DIAL : %d ", ret);
          break;
      case GCL_CONNECT:
          ret = 3;
          sprintf(buffer, "GCL_CONNECT : %d ", ret);
          break;
      case GCL_LOGIN:
          ret = 4;
          sprintf(buffer, "GCL_LOGIN : %d ", ret);
          break;
      case GCL_CONNECTED:
          ret = 0;
          break;
      default:
          ret = -1;
          break;
    }

    hmiADDisplayText(displayHandle, 0, 0, buffer);
    return ret;
}

static int16 connErrorCallBack(gclConfig_t * gcl) {
    gclSetError(0);
    return gclPostMessage(GCL_RETRY);
}

static int16 nullCallBack(gclConfig_t * gcl) {
    return RET_OK;
}

static int16 StartSck(void) {
    int16 ret;
    gclFunctionList_t fnc;

    memset(&fnc, 0, sizeof(fnc));
    ret = gclPgSupReset();
    if(ret != RET_OK)
        return -1;

    sck.connectionId = 1;
    sck.retries = 3;
    sck.connectTimeout = 6000;
    sck.communicationTimeout = 3000;
    sck.loginTimeout = 3000;
    sck.retryDelay = 5;
    //sck.soLinger= 1;
    sck.ipAddress = "91.121.18.221";
    sck.tcpPort = "6789";

    fnc.predial = gclSockPreDial;
    fnc.dial = gclSockDial;
    fnc.connect = gclSockConnect;
    fnc.login = nullCallBack;
    fnc.hangup = gclSockHangUp;
    fnc.retry = gclConnectRetry;
    fnc.error = connErrorCallBack;
    fnc.send = gclSockSend;
    fnc.recv = gclSockReceive;

    ret = gclSockSet(&sck, &fnc, 0, NULL);
    if(ret < 0)
        goto lblKO;

    ret = gclPgSupAttach(6000);
    if(ret < 0)
        goto lblKO;

    ret = gclStart();
    if(ret < 0)
        goto lblKO;
    ret = gclStartConnection();
    if(ret < 0)
        goto lblKO;

    do {
        psyTimerWakeAfter(10);
        ret = DialInfo();
        if(ret < 0)
            goto lblKO;
    } while(ret > 0);

    return 1;
  lblKO:
    return -1;
}

static int16 SendHandShake(void) {
    int16 ret;
    uint8 msg[256];
    uint32 len, tmp;
    uint8 idx;

//Build message
    memset(msg, 0, sizeof(msg));

    //x00 x00 x00 3 x01 x01 x04 x00 x85 x0B x03 5 V x00 x05 x03 h a n d s h a k e x00
    //x1D x03 2 0 0 7 0 6 0 4 1 1 1 0 1 0 x00 x06 x03 x00 x0F x03 0 x00 x14 x03 0 x00 x01
    len = 4;
    memcpy(msg + len, "\x01\x01\x04\x00", 4);   //bml header
    len += 4;
    memcpy(msg + len, (uint8 *) "\x85", 1); //tag req with attributes
    len += 1;
    memcpy(msg + len, (uint8 *) "\x0B\x03\x35\x56\x00", 5); //app
    len += 5;
    memcpy(msg + len, (uint8 *) "\x05\x03\x68\x61\x6E\x64\x73\x68\x61\x6B\x65\x00", 12);    //id handshake
    len += 12;
    memcpy(msg + len, (uint8 *) "\x1D\x03\x32\x30\x30\x37\x30\x36\x30\x34\x31\x31\x31\x30\x31\x30\x00", 17);    //dt
    len += 17;
    memcpy(msg + len, (uint8 *) "\x06\x03\x30\x30\x30\x30\x30\x30\x30\x31\x00", 11);    //tid
    len += 11;
    memcpy(msg + len, (uint8 *) "\x0F\x03\x30\x00", 4); //mid
    len += 4;
    memcpy(msg + len, (uint8 *) "\x14\x03\x30\x00", 4); //aut
    len += 4;
    memcpy(msg + len, (uint8 *) "\x01", 1); //bmlEND
    len += 1;

    idx = 4;
    tmp = len - 4;
    while(idx--) {
        msg[idx] = (uint8) (tmp % 256);
        tmp /= 256;
    }
    sprintf(buffer, "Bytes sent : %d ", len);
    hmiADDisplayText(displayHandle, 0, 0, buffer);
    prnPrint(prnHandle, buffer);

    //Send message
    ret = gclSend(msg, len);
    return ret;
}

static uint16 RecvRsp(void) {
    int16 ret;
    uint32 len, num;
    uint8 tmp[256];

    //receive message length
    len = 4;
    memset(tmp, 0, len);
    gclChageCommTimeout(2000);
    ret = gclReceive(tmp, len, &len);
    if(ret != RET_OK)
        goto lblKO;
    num = 0;
    while(len--) {
        num *= 256;
        num += tmp[3 - len];
    }

    //receive message
    len = num;
    memset(tmp, 0, len);
    ret = gclReceive(tmp, len, &len);
    if(ret != RET_OK)
        goto lblKO;

    sprintf(buffer, "Bytes recv : %d", len + 4);
    hmiADDisplayText(displayHandle, 0, 0, buffer);
    prnPrint(prnHandle, buffer);

    return (uint16) len;
  lblKO:
    switch (gclLastError()) {
      case GCL_ERR_TIMEOUT:
          ret = 0;
          break;
      default:
          ret = -1;
          break;
    }
    gclSetError(0);
    return ret;
}

void tcik0099(void) {
    int16 ret;

    memset(buffer, 0, sizeof(buffer));

    hmiOpen("DEFAULT", &displayHandle);
    prnOpen("DEFAULT", &prnHandle, PRN_IMMEDIATE);
    prnPrint(prnHandle, "\x1B\x17\x1B\x1a" "tcik0099");

    //hmiADDisplayText(displayHandle, 0, 0, "GprsCheck..");
    //ret = GprsCheck();
    //if(ret<0) goto lblKO;

    hmiADDisplayText(displayHandle, 0, 0, "gsmReset..");
    ret = gclReset();
    sprintf(buffer, "gclReset ret: %d", ret);
    prnPrint(prnHandle, buffer);

    hmiADDisplayText(displayHandle, 0, 0, "StartSck..");
    ret = StartSck();
    if(ret < 0)
        goto lblKO;

    hmiADDisplayText(displayHandle, 1, 0, "SendHandShake..");
    ret = SendHandShake();
    if(ret < 0)
        goto lblKO;

    hmiADDisplayText(displayHandle, 2, 0, "RecvRsp..");
    ret = RecvRsp();
    if(ret < 0)
        goto lblKO;

    prnPrint(prnHandle, "\x1B\x17\x1B\x1a" "PASS");
    prnPrint(prnHandle, "\x1B\x17\x1B\x1a" "----------");
    goto lblEnd;
  lblKO:
    prnPrint(prnHandle, "\x1B\x17\x1B\x1a" "FAIL");
    prnPrint(prnHandle, "\x1B\x17\x1B\x1a" "----------");
  lblEnd:
    gclStop();
    prnClose(prnHandle);
    hmiClose(displayHandle);
}
