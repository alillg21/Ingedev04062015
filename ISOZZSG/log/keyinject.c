/** \file
 * Utility to inject MasterKey from Serial Port(COM0).
 *
 * All of them follow the return code convention:
 *    - >0 : operation OK, continue
 *    -  0 : user cancel or timeout: smooth exit
 *    - <0 : irrecoverable error, stop processing anyway
 */

#include "SDK30.h"
#include "OEM_Public.h"
#include "sys.h"
#undef __USE_DEL_INTERFACE__
#include "TlvTree.h"
#include "LinkLayer.h"
#include "oem_dgprs.h"

#include "key.h"
#include "log.h"
#include "stdlib.h"


#include "keyinject.h" //@@AS0.0.11

typedef struct sComChn {
    byte chn;                   // communication channel used
    byte chnRole;
    struct {
        LL_HANDLE hdl;
        char separator;
        unsigned long int conn_delay;   // 4 Second delay
        unsigned long int ll_conn_delay;
        unsigned int send_timeout;
        int ifconn;
#ifdef __CHN_COM__
        byte BperB;             //BITS_PER_BYTE 7 or not
        byte Parity;            //PARITY
#endif
    } prm;
    TLV_TREE_NODE hCfg;
    TLV_TREE_NODE hPhyCfg;
    TLV_TREE_NODE hDLinkCfg;
    TLV_TREE_NODE hTransCfg;
} tComChn;


static tComChn injectCom;
byte serialBuff[260];


static int injectComSetCom(const char *init)
{
    int ret;
    byte datasize;              //data bits: 7 or 8
    byte parity;                //odd,even,none
    byte stopbits;              //1,2
    unsigned long sgmBaudRate = LL_PHYSICAL_V_BAUDRATE_1200;
    unsigned char physConf;

    trcFS("prepadiComSetCom %s\n", init);
    VERIFY(init);
    VERIFY(injectCom.chn == chnCom1 || injectCom.chn == chnCom2 || injectCom.chn == chnCom3);

    injectCom.prm.separator = '|';    //common for all types of chn
    injectCom.prm.conn_delay = 4 * 100;   //4 Second delay
    injectCom.prm.send_timeout = 30 * 100;
    injectCom.prm.ifconn = FALSE;

    datasize = LL_PHYSICAL_V_8_BITS;

    switch (*init++) {          //number of data bits
      case '7':
          datasize = LL_PHYSICAL_V_7_BITS;
          break;
      case '8':
          datasize = LL_PHYSICAL_V_8_BITS;
          break;
      default:
          CHECK(!init, lblKOConfigure);
    }

    parity = LL_PHYSICAL_V_NO_PARITY;
    switch (*init++) {          //parity
      case 'N':
          parity = LL_PHYSICAL_V_NO_PARITY;
          break;
      case 'E':
          parity = LL_PHYSICAL_V_EVEN_PARITY;
          break;
      case 'O':
          parity = LL_PHYSICAL_V_ODD_PARITY;
          break;
      default:
          CHECK(!init, lblKOConfigure);
    }

    stopbits = LL_PHYSICAL_V_1_STOP;
    switch (*init++) {          //number of stop bits
      case '0':
          stopbits = LL_PHYSICAL_V_1_STOP;
          break;
      case '1':
          stopbits = LL_PHYSICAL_V_1_STOP;
          break;
      case '2':
          stopbits = LL_PHYSICAL_V_2_STOP;
          break;
      default:
          CHECK(!init, lblKOConfigure);
    }

    if(*init == 0) {
        sgmBaudRate = LL_PHYSICAL_V_BAUDRATE_1200;
        init++;
    } else if(memcmp(init, "300", 3) == 0) {
        sgmBaudRate = LL_PHYSICAL_V_BAUDRATE_300;
        init += 3;
    } else if(memcmp(init, "1200", 4) == 0) {
        sgmBaudRate = LL_PHYSICAL_V_BAUDRATE_1200;
        init += 4;
    } else if(memcmp(init, "2400", 4) == 0) {
        sgmBaudRate = LL_PHYSICAL_V_BAUDRATE_2400;
        init += 4;
    } else if(memcmp(init, "4800", 4) == 0) {
        sgmBaudRate = LL_PHYSICAL_V_BAUDRATE_4800;
        init += 4;
    } else if(memcmp(init, "9600", 4) == 0) {
        sgmBaudRate = LL_PHYSICAL_V_BAUDRATE_9600;
        init += 4;
    } else if(memcmp(init, "19200", 5) == 0) {
        sgmBaudRate = LL_PHYSICAL_V_BAUDRATE_19200;
        init += 5;
    } else if(memcmp(init, "38400", 5) == 0) {
        sgmBaudRate = LL_PHYSICAL_V_BAUDRATE_38400;
        init += 5;
    } else if(memcmp(init, "57600", 5) == 0) {
        sgmBaudRate = LL_PHYSICAL_V_BAUDRATE_57600;
        init += 5;
    } else if(memcmp(init, "115200", 6) == 0) {
        sgmBaudRate = LL_PHYSICAL_V_BAUDRATE_115200;
        init += 6;
    }

    injectCom.hCfg = TlvTree_New(LL_TAG_LINK_LAYER_CONFIG);

    injectCom.hPhyCfg =
        TlvTree_AddChild(injectCom.hCfg, LL_TAG_PHYSICAL_LAYER_CONFIG, NULL, 0);

//	physConf = LL_PHYSICAL_V_COM0;
    physConf =
        (injectCom.chn == chnCom1 ? LL_PHYSICAL_V_COM0 :
         injectCom.chn == chnCom1 ? LL_PHYSICAL_V_COM1 : LL_PHYSICAL_V_COM2);
    TlvTree_AddChildInteger(injectCom.hPhyCfg, LL_PHYSICAL_T_LINK, physConf,
                            LL_PHYSICAL_L_LINK);

    CHECK(sgmBaudRate, lblKOConfigure);
    TlvTree_AddChildInteger(injectCom.hPhyCfg, LL_PHYSICAL_T_BAUDRATE, sgmBaudRate,
                            LL_PHYSICAL_L_BAUDRATE);

    TlvTree_AddChildInteger(injectCom.hPhyCfg, LL_PHYSICAL_T_BITS_PER_BYTE, datasize,
                            LL_PHYSICAL_L_BITS_PER_BYTE);

    TlvTree_AddChildInteger(injectCom.hPhyCfg, LL_PHYSICAL_T_STOP_BITS, stopbits,
                            LL_PHYSICAL_L_STOP_BITS);

    TlvTree_AddChildInteger(injectCom.hPhyCfg, LL_PHYSICAL_T_PARITY, parity,
                            LL_PHYSICAL_L_PARITY);

//tambahan ++
    TlvTree_AddChildInteger(injectCom.hPhyCfg, LL_PHYSICAL_T_FLOW_CTRL, LL_PHYSICAL_V_NO_FLOW_CTRL,
                            LL_PHYSICAL_L_FLOW_CTRL);
//    injectCom.prm.hdl = NULL;
//tambahan --
    ret = LL_Configure(&injectCom.prm.hdl, injectCom.hCfg);
    CHECK(ret == LL_ERROR_OK
          || ret == LL_ERROR_ALREADY_CONNECTED, lblKOConfigure);

    ret = 1;
    goto lblEnd;
  lblKOConfigure:
    ret = -1;
    goto lblEnd;
  lblEnd:
    trcFN("prepadiComSetCom ret=%d\n", ret);
    return ret;
}

int injectOpenCom(void)
{
	int ret;

	injectCom.chn = chnCom1;
	injectCom.hCfg = NULL;
	injectCom.hDLinkCfg = NULL;
	injectCom.hPhyCfg = NULL;
	injectCom.hTransCfg = NULL;

	if(injectComSetCom("8N138400") == -1)
		return -1;

    ret = LL_Connect(injectCom.prm.hdl);
    if(ret != LL_ERROR_OK && ret != LL_ERROR_ALREADY_CONNECTED)
    	return -1;

	return 0;
}


int injectCloseCom(void)
{
	int ret;

	LL_Configure(&injectCom.prm.hdl, NULL);
	if(injectCom.prm.hdl)
	{
		ret = LL_Disconnect(injectCom.prm.hdl);
		if(ret!=LL_ERROR_OK)
			return -1;
		injectCom.prm.hdl = 0;
	}
	return 0;
}

static int injectSendBufCom(const byte * msg, byte len) {

    int ret,i,buflen;
    byte chsum;

	serialBuff[0] = 0x02;
	serialBuff[1] = len;
	buflen=2;
	memcpy(&serialBuff[buflen],msg,len);
	buflen+=len;
	serialBuff[buflen] = 0x03;
	buflen++;

	chsum =0;
	for (i=1;i<buflen;i++)
		chsum ^= serialBuff[i];

	serialBuff[buflen] = chsum;
	buflen++;


    ret = LL_Send(injectCom.prm.hdl, buflen, serialBuff, LL_INFINITE);
    CHECK(ret == buflen, lblKO);
    ttestall(0, 100);           //to make sure sending is finished before receiving

    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    trcFN("injectSendBufCom ret=%d\n", ret);
    return ret;
}

static int injectRecvCom(byte * outData,byte *outLen, int dly) {

    int ret = 0,msglen;
    byte chsum,i;

    ret = LL_Receive(injectCom.prm.hdl, sizeof(serialBuff), serialBuff, dly * 100);
    CHECK(ret > 0, lblKO);

	CHECK(serialBuff[0]==0x02, lblKO);

	msglen = serialBuff[1];

	CHECK(ret==(msglen+4),lblKO);

	CHECK(serialBuff[ret-2]==0x03,lblKO);

	chsum=0;
	for (i=1;i<(ret-1);i++)
		chsum ^= serialBuff[i];

	CHECK(chsum==serialBuff[ret-1],lblKO);

	memcpy(outData,&serialBuff[2],msglen);

	*outLen = msglen;

    return (ret);
  lblKO:
    trcFN("injectRecvCom ret=%d\n", ret);
    return -1;

}


void gen_random_data (byte len,byte *data_out)
{
    int         i;

    for (i = 0; i < len; i++)
        *(data_out + i) = (byte) (rand () % 256);
}

int loadTransportTest(void) {
    byte ucLoc;
    byte tucCrt[4];
    char tcTmp[prtW + 1];
 //   byte tucKey[16];

    int ret;
    byte CrtLen;


#ifdef _DEBUG_MK
    byte prtBuf[50];
#endif

#ifdef __APR__
    CrtLen = 3;
#else
    CrtLen = 4;
#endif

    memset(tcTmp, 0, prtW + 1);
    memset(tucCrt, 0, sizeof(tucCrt));
    trcS("loadTransportTest Beg\n");

    ret = dspClear();
#ifdef _DEBUG_MK
	sprintf(prtBuf,"dspClear=%d",ret);prtS(prtBuf);
#endif
    CHECK(ret >= 0, lblKO);
    ret = cryStart('m');
#ifdef _DEBUG_MK
	sprintf(prtBuf,"cryStart=%d",ret);prtS(prtBuf);
#endif
    CHECK(ret >= 0, lblKO);
//    ret = usrInfo(infTestKeyLoad);
//    CHECK(ret >= 0, lblKO);

    ucLoc = __RootKeyLoc__;
    ret = cryEraseKeysArray(ucLoc);//TEST_ONLY
    ret = cryLoadTestKey(ucLoc);
    CHECK(ret >= 0, lblKO);
    ret = cryVerify(ucLoc, tucCrt);
    CHECK(ret >= 0, lblKO);
    strcpy(tcTmp, "TK ");
    ret = bin2hex(&tcTmp[3], &ucLoc, 1);
    CHECK(ret == 2, lblKO);
    strcat(tcTmp, ": ");
    VERIFY(strlen(tcTmp) == 7);
    ret = bin2hex(&tcTmp[7], tucCrt, 4);
    CHECK(ret == 8, lblKO);

    MAPPUTSTR(msgVar, &tcTmp[3], lblKO);
    CHECK(ret >= 0, lblKO);
    goto lblEnd;
  lblKO:
    usrInfo(infProcessingError);
    ret=-1;
    goto lblEnd;
  lblEnd:
    cryStop('m');
    trcS("loadTransportTest End\n");
    return(ret);
}

/*
 * key_index : 1 up to 16
 * key_value : 16 bytes key value
 */
int storeMasterKey(byte key_index,byte *key_value) {
    byte ucLoc;
    byte tucCrt[4];
    char tcTmp[prtW + 1];
    byte tucKey[16];

    int ret;
    byte CrtLen;

#ifdef _DEBUG_MK
    byte prtBuf[50];
#endif

    CrtLen = 4;

    memset(tcTmp, 0, prtW + 1);
    memset(tucCrt, 0, sizeof(tucCrt));
    trcS("storeMasterKey Beg\n");

    //ret = dspClear();
#ifdef _DEBUG_MK
	sprintf(prtBuf,"dspClear=%d",ret);prtS(prtBuf);
#endif
    //CHECK(ret >= 0, lblKO);
    ret = cryStart('m');
#ifdef _DEBUG_MK
	sprintf(prtBuf,"cryStart=%d",ret);prtS(prtBuf);
#endif
    CHECK(ret >= 0, lblKO);

    //usrInfo(infLodMstKey);



    memcpy(tucKey, key_value, 16);

    //ucLoc = __appMKeyLoc__;
    ucLoc = key_index+16;

    ret = cryLoadDKuDK(ucLoc, __RootKeyLoc__, tucKey, 'g');
#ifdef _DEBUG_MK
	sprintf(prtBuf,"cryLoadDKuDK=%d",ret);prtS(prtBuf);
#endif
    CHECK(ret >= 0, lblKO);

    ret = cryVerify(ucLoc, tucCrt);
#ifdef _DEBUG_MK
	sprintf(prtBuf,"cryVerify=%d",ret);prtS(prtBuf);
#endif
    CHECK(ret >= 0, lblKO);
    strcpy(tcTmp, "MK ");
    ret = bin2hex(&tcTmp[3], &ucLoc, 1);
    CHECK(ret == 2, lblKO);
    strcat(tcTmp, ": ");
    VERIFY(strlen(tcTmp) == 7);
    ret = bin2hex(&tcTmp[7], tucCrt, 4);
    CHECK(ret == 8, lblKO);

    tmrPause(1);
    goto lblEnd;
  lblKO:
    //usrInfo(infProcessingError);
    ret=-1;
    goto lblEnd;
  lblEnd:
    cryStop('m');
    trcS("storeMasterKey End\n");
    return(ret);
}

#ifdef USE_SEC_AREA
int storeWorkingKey(byte key_index,byte *key_value) {
    byte ucLoc,cipherLoc;
    byte tucCrt[4];
    char tcTmp[prtW + 1];
    byte tucKey[16];

    int ret;
    byte CrtLen;

#ifdef _DEBUG_MK
    byte prtBuf[50];
#endif

//	byte keyD[50]; //download key


    CrtLen = 3;

    memset(tcTmp, 0, prtW + 1);
    memset(tucCrt, 0, sizeof(tucCrt));
    trcS("storeWorkingKey Beg\n");

    ret = cryStart('m');
#ifdef _DEBUG_MK
	sprintf(prtBuf,"cryStart=%d",ret);prtS(prtBuf);
#endif
    CHECK(ret >= 0, lblKO);

    //MAPPUTSTR(msgVar, &tcTmp[3], lblKO);

    //usrInfo(infLodWrkKey);

    memcpy(tucKey,key_value,16);

    ucLoc = key_index;
    cipherLoc = key_index+16;

    ret = cryLoadDKuDK(ucLoc, cipherLoc, tucKey, 'p');
#ifdef _DEBUG_MK
	sprintf(prtBuf,"cryLoadDKuDK=%d",ret);prtS(prtBuf);
#endif
    CHECK(ret >= 0, lblKO);

    ret = cryVerify(ucLoc, tucCrt);
#ifdef _DEBUG_MK
	sprintf(prtBuf,"cryVerify=%d",ret);prtS(prtBuf);
#endif
    CHECK(ret >= 0, lblKO);

    strcpy(tcTmp, "WK ");
    ret = bin2hex(&tcTmp[3], &ucLoc, 1);
    CHECK(ret == 2, lblKO);
    strcat(tcTmp, ": ");
    VERIFY(strlen(tcTmp) == 7);
    ret = bin2hex(&tcTmp[7], tucCrt, 4);
    CHECK(ret == 8, lblKO);
    //TODO rptReceipt();
//    ret = prtS(tcTmp);
//    CHECK(ret >= 0, lblKO);
    //tmrPause(1);
	 //dspClear();
	 //dspLS(2,"LOAD KEY SUCCESS");
	 //tmrPause(2);
    //MAPPUTSTR(msgVar, &tcTmp[3], lblKO);
//    usrInfo(infVarLin3);
    //CHECK(memcmp(tucCrt, "\xF7\x53\xC2\x98", CrtLen) == 0, lblKO);  //it is the certificate of the key 6B218F24DE7DC66C

    //TODO rptReceipt();
    goto lblEnd;
  lblKO:
    //usrInfo(infProcessingError);
    ret=-1;
    goto lblEnd;
  lblEnd:
    cryStop('m');
    trcS("storeWorkingKey End\n");
    return(ret);
}
#else //USE_SEC_AREA
int storeWorkingKey(byte key_index,byte *key_value) {//@@SIMAS-SW_PIN
	int i;
	int ret;
    for(i=0;i<dimAcq;i++)
    {
        mapMove(acqBeg,i);
        ret=mapPut(acqTWK,key_value,lenKEY);
        if(ret<0) return ret;
    }
    return (8);
}

#endif //USE_SEC_AREA

//@@AS-SIMAS - Start
int store_acqTMK(byte *tmk)
{
	int i;
	int ret;
    for(i=0;i<dimAcq;i++)
    {
        mapMove(acqBeg,i);
        ret=mapPut(acqTMK,tmk,lenKEY);
        if(ret<0) return ret;
    }
    return (0);
}

int store_acqTMKRefNo(byte *tmkRefNo)
{
	int i;
	int ret;
    for(i=0;i<dimAcq;i++)
    {
        mapMove(acqBeg,i);
        ret=mapPut(acqTMKRefNo,tmkRefNo,lenTMKRefNo);
        if(ret<0) return ret;
    }
	return (0);
}

int store_acqTAKLogon(byte *tak)
{
	int i;
	int ret;
    for(i=0;i<dimAcq;i++)
    {
        mapMove(acqBeg,i);
        ret=mapPut(acqTAKLogon,tak,lenKEY);
        if(ret<0) return ret;
    }
    return (0);

}


int store_acqTAKnTLE(byte *tak,byte *tle)
{
	int i;
	int ret;
    for(i=0;i<dimAcq;i++)
    {
        mapMove(acqBeg,i);
        ret=mapPut(acqTAK,tak,lenKEY);
        if(ret<0) return ret;
        ret=mapPut(acqTLE,tle,lenKEY);
        if(ret<0) return ret;
    }
    return (0);
}

#ifdef SIMAS_TMS
int strdec2bin(byte *dataIn,int inLen,byte *dataOut,int *outLen)
{
	int i,j,k;
	byte tmp[5];

	if ((inLen % 3) != 0)
		return (-1);
	*outLen=inLen/3;
	memset(tmp,0x00,sizeof(tmp));
	k=0;
	for (i=0; i<*outLen;i++) {
		memcpy(tmp,dataIn+k,3);
		*(dataOut+i) = atoi(tmp);
		k+=3;
	}
}

#ifdef USE_SEC_AREA
int storeKeyFromTMS(void)
{

	int ret,outLen;
   	byte buff[100],sessBuf[lenKEY],key_index,TMK[lenKEY],secTMK[lenKEY],TAK[lenKEY],TMK_REFNO[lenTMKRefNo+1],sessBuf2[lenKEY];
   	char tmp[lenKEY_hex*2+1],tmp1[lenKEY_hex+1],tmp2[lenKEY_hex+1];

   	//Retrieve Data from registry
	memset(tmp1,0x00,sizeof(tmp1));
	MAPGET(regTMK,tmp1,lblKO);
	memset(tmp2,0x00,sizeof(tmp2));
	MAPGET(regTAKLogon,tmp2,lblKO);
	memset(tmp,0x00,sizeof(tmp));
	memcpy(tmp,tmp1,lenKEY_hex);
	memcpy(&tmp[lenKEY_hex],tmp2,lenKEY_hex);

	//Store TMS Reference Number
	memset(TMK_REFNO, 0, 8);
	strdec2bin(tmp,24,TMK_REFNO,&outLen);
	if (store_acqTMKRefNo(TMK_REFNO) <0) {
		dspClear();
		usrInfo(infLodMstKeyFail);
		return (-1);
	}
	memcpy(sessBuf2,TMK_REFNO,8);
	sessBuf2[8]=TMK_REFNO[7]; sessBuf2[9]=TMK_REFNO[6]; sessBuf2[10]=TMK_REFNO[5]; sessBuf2[11]=TMK_REFNO[4];
	sessBuf2[12]=TMK_REFNO[3]; sessBuf2[13]=TMK_REFNO[2]; sessBuf2[14]=TMK_REFNO[1]; sessBuf2[15]=TMK_REFNO[0];



   	#if defined (SST_DEBUG) || defined (SAT_DEBUG)
		memcpy(sessBuf,"\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11",lenKEY);
	#else
	  	memcpy(sessBuf,"\x11\x1A\x1A\x18\x00\x00\x00\x13\x07\x05\x15\x08\x05\x1E\x08\x13",lenKEY);
	#endif

	//Decode TMK
	strdec2bin(&tmp[24],48,TMK,&outLen);
	stdDED(buff, TMK, sessBuf2);
	stdDED(&buff[8], &TMK[8], sessBuf2);
	memcpy(TMK, buff, lenKEY);
	stdEDE(buff, TMK, sessBuf);
	stdEDE(&buff[8], &TMK[8], sessBuf);
	memcpy(secTMK, buff, lenKEY);

	//Decode TAK
	strdec2bin(&tmp[72],48,TAK,&outLen);


#if defined (SST_DEBUG) || defined (SAT_DEBUG)
	if (loadTransportTest() < 0)
		return (-1);
#endif


	//Store TMK to secure area
	key_index = 1;
	dspClear();
	usrInfo(infLodMstKey);
	if (storeMasterKey(key_index,secTMK) <0) {
		dspClear();
		usrInfo(infLodMstKeyFail);
		return (-1);
	}

	//Store TMK to Acq table
	if (store_acqTMK(TMK) <0) {
		dspClear();
		usrInfo(infLodMstKeyFail);
		return (-1);
	}

	//Store TAK to Acq Table
	dspClear();
	usrInfo(infLodTAKKey);
	stdDED(buff,TAK, TMK);
	stdDED(&buff[8],&TAK[8], TMK);
#ifdef _DEBUG_MK
	hex_dump_char("--TAK--",buff,KEY_LEN);
#endif
	if (store_acqTAKLogon(buff)<0) {
		dspClear();
		usrInfo(infLodTAKKeyFail);
		return (-1);
	}
	dspClear();
	usrInfo(infLodTAKSuccess);

	return (0);


lblKO:
	return (-1);

}
#else //USE_SEC_AREA
int storeKeyFromTMS(void) //@@SIMAS-SW_PIN
{

	int ret,outLen;
   	byte buff[100],sessBuf[lenKEY],key_index,TMK[lenKEY],secTMK[lenKEY],TAK[lenKEY],TMK_REFNO[lenTMKRefNo+1],sessBuf2[lenKEY];
   	char tmp[lenKEY_hex*2+1],tmp1[lenKEY_hex+1],tmp2[lenKEY_hex+1];

   	//Retrieve Data from registry
	memset(tmp1,0x00,sizeof(tmp1));
	MAPGET(regTMK,tmp1,lblKO);
	memset(tmp2,0x00,sizeof(tmp2));
	MAPGET(regTAKLogon,tmp2,lblKO);
	memset(tmp,0x00,sizeof(tmp));
	memcpy(tmp,tmp1,lenKEY_hex);
	memcpy(&tmp[lenKEY_hex],tmp2,lenKEY_hex);

	//Store TMS Reference Number
	memset(TMK_REFNO, 0, 8);
	strdec2bin(tmp,24,TMK_REFNO,&outLen);
	if (store_acqTMKRefNo(TMK_REFNO) <0) {
		dspClear();
		usrInfo(infLodMstKeyFail);
		return (-1);
	}
	memcpy(sessBuf2,TMK_REFNO,8);
	sessBuf2[8]=TMK_REFNO[7]; sessBuf2[9]=TMK_REFNO[6]; sessBuf2[10]=TMK_REFNO[5]; sessBuf2[11]=TMK_REFNO[4];
	sessBuf2[12]=TMK_REFNO[3]; sessBuf2[13]=TMK_REFNO[2]; sessBuf2[14]=TMK_REFNO[1]; sessBuf2[15]=TMK_REFNO[0];


	//Decode TMK
	strdec2bin(&tmp[24],48,TMK,&outLen);
	stdDED(buff, TMK, sessBuf2);
	stdDED(&buff[8], &TMK[8], sessBuf2);
	memcpy(TMK, buff, lenKEY);

	//Decode TAK
	strdec2bin(&tmp[72],48,TAK,&outLen);

	//Store TMK
	dspClear();
	usrInfo(infLodMstKey);

	//Store TMK to Acq table
	if (store_acqTMK(TMK) <0) {
		dspClear();
		usrInfo(infLodMstKeyFail);
		return (-1);
	}

	//Store TAK to Acq Table
	dspClear();
	usrInfo(infLodTAKKey);
	stdDED(buff,TAK, TMK);
	stdDED(&buff[8],&TAK[8], TMK);
#ifdef _DEBUG_MK
	hex_dump_char("--TAK--",buff,KEY_LEN);
#endif
	if (store_acqTAKLogon(buff)<0) {
		dspClear();
		usrInfo(infLodTAKKeyFail);
		return (-1);
	}
	dspClear();
	usrInfo(infLodTAKSuccess);

	return (0);
lblKO:
	return (-1);

}
#endif //USE_SEC_AREA


#endif //SIMAS_TMS


//@@AS-SIMAS - End

#ifdef KEY_TEST
int req4Key(void)
{
	#define KEY_LEN	16
	int ret;
   	byte buff[50],sessBuf[KEY_LEN],key_index,TMK[KEY_LEN],TPK[KEY_LEN],TMK_REFNO[8];

	memset(TMK_REFNO, 0, 8);

	memcpy(TMK_REFNO,"003033",8);

#if defined (SST_DEBUG) || defined (SAT_DEBUG)
	memcpy(sessBuf,"\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11",KEY_LEN);
#else
  	memcpy(sessBuf,"\x11\x1A\x1A\x18\x00\x00\x00\x13\x07\x05\x15\x08\x05\x1E\x08\x13",KEY_LEN);
#endif
	memcpy(TMK,"\xA1\x2A\x13\x67\x94\xB5\x62\x31\x02\x3D\x83\x4F\x19\x86\x61\xD0",KEY_LEN);
	stdEDE(buff, TMK, sessBuf);
	stdEDE(&buff[8], &TMK[8], sessBuf);
	memcpy(TMK, buff, KEY_LEN);
	memcpy(TPK,"\x61\xBC\x08\x0A\x53\x53\xE8\x38\x21\xD4\xF6\xD9\xD7\xB2\xD5\x22",KEY_LEN);


#if defined (SST_DEBUG) || defined (SAT_DEBUG)
	if (loadTransportTest() < 0)
		return (-1);
#endif

	dspClear();
	usrInfo(infLodMstKey);
	key_index=1;
	if (storeMasterKey(key_index,TMK) <0) {
		dspClear();
		usrInfo(infLodMstKeyFail);
		return (-1);
	}

	 //@@AS-SIMAS - start
#if defined (SST_DEBUG) || defined (SAT_DEBUG)
	memcpy(sessBuf,"\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11",KEY_LEN);
#else
   memcpy(sessBuf,"\x11\x1A\x1A\x18\x00\x00\x00\x13\x07\x05\x15\x08\x05\x1E\x08\x13",KEY_LEN);
#endif
	stdDED(buff,TMK, sessBuf);
	stdDED(&buff[8],&TMK[8], sessBuf);
	memcpy(TMK,buff,KEY_LEN);
#ifdef _DEBUG_MK
	hex_dump_char("--TMK--",TMK,KEY_LEN);
#endif
	if (store_acqTMK(TMK) <0) {
		dspClear();
		usrInfo(infLodMstKeyFail);
		return (-1);
	}
	if (store_acqTMKRefNo(TMK_REFNO) <0) {
		dspClear();
		usrInfo(infLodMstKeyFail);
		return (-1);
	}
    //@@AS-SIMAS - end

	dspClear();
	usrInfo(infLodTAKKey);
	stdDED(buff,TPK, TMK);
	stdDED(&buff[8],&TPK[8], TMK);
#ifdef _DEBUG_MK
	hex_dump_char("--TAK--",buff,KEY_LEN);
#endif
	if (store_acqTAKLogon(buff)<0) {
		dspClear();
		usrInfo(infLodTAKKeyFail);
		return (-1);
	}
	dspClear();
	usrInfo(infLodTAKSuccess);

   	return ret;

	///===============================================
}
#else

int req4Key(void)
{
	#define KEY_LEN	16
	int ret;
   	byte rndBuf[20],buff[50],i,sessBuf[KEY_LEN],TMK_TPK[KEY_LEN*2+9],key_index,TMK[KEY_LEN],TPK[KEY_LEN],TMK_REFNO[8];
   	char serialNum[20 + 1];
   	char sap[16 + 1];
	char dvc;


   	if(injectOpenCom()==-1)
   		return -1;

   	gen_random_data(8,rndBuf);

   	memset(serialNum, 0, sizeof(serialNum));
   	memset(sap, 0, sizeof(sap));
	memset(&dvc, 0, sizeof(dvc));
	getSapSer(sap, serialNum, dvc);




	for (i=0;i<8;i++)
		sessBuf[i] = serialNum[2+i] ^ rndBuf[i];


	for (i=0;i<8;i++)
		sessBuf[i+8] = rndBuf[i] ^ 0x0F;


	ret=injectSendBufCom(sessBuf, KEY_LEN);

   	if(ret <=0)
   	{
	  injectCloseCom();
      return -1;
   	}

	ret=injectRecvCom(TMK_TPK,&i, 30);
	if(ret <=0)
	{
	  injectCloseCom();
	  return -1;
	}

	injectCloseCom();

#ifdef _DEBUG_MK
	hex_dump_char("--BEFORE DECRYPT--",TMK_TPK,sizeof(TMK_TPK));
#endif //_DEBUG_MK

	memcpy(sessBuf,&serialNum[2],8);
	memcpy(&sessBuf[8],rndBuf,8);

	//Start to store key
	key_index = TMK_TPK[0];

	//stdEDE()
	stdDED(buff,&TMK_TPK[1], sessBuf);
	stdDED(&buff[8],&TMK_TPK[9], sessBuf);
	stdDED(&buff[16],&TMK_TPK[17], sessBuf);
	stdDED(&buff[24],&TMK_TPK[25], sessBuf);
	stdDED(&buff[32],&TMK_TPK[33], sessBuf); //@@AS-SIMAS


#ifdef _DEBUG_MK
	hex_dump_char("--AFTER DECRYPT--",buff,KEY_LEN*2+8);
#endif //_DEBUG_MK
	memcpy(TMK_REFNO,buff,8);
	memcpy(TMK,&buff[8],KEY_LEN);
	memcpy(TPK,&buff[8+KEY_LEN],KEY_LEN);

#if defined (SST_DEBUG) || defined (SAT_DEBUG)
	if (loadTransportTest() < 0)
		return (-1);
#endif

	dspClear();
	usrInfo(infLodMstKey);
	key_index=1;
	if (storeMasterKey(key_index,TMK) <0) {
		dspClear();
		usrInfo(infLodMstKeyFail);
		return (-1);
	}

	 //@@AS-SIMAS - start
#if defined (SST_DEBUG) || defined (SAT_DEBUG)
	memcpy(sessBuf,"\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11",KEY_LEN);
#else
   memcpy(sessBuf,"\x11\x1A\x1A\x18\x00\x00\x00\x13\x07\x05\x15\x08\x05\x1E\x08\x13",KEY_LEN);
#endif
	stdDED(buff,TMK, sessBuf);
	stdDED(&buff[8],&TMK[8], sessBuf);
	memcpy(TMK,buff,KEY_LEN);
#ifdef _DEBUG_MK
	hex_dump_char("--TMK--",TMK,KEY_LEN);
#endif
	if (store_acqTMK(TMK) <0) {
		dspClear();
		usrInfo(infLodMstKeyFail);
		return (-1);
	}
	if (store_acqTMKRefNo(TMK_REFNO) <0) {
		dspClear();
		usrInfo(infLodMstKeyFail);
		return (-1);
	}
    //@@AS-SIMAS - end

	dspClear();
	usrInfo(infLodTAKKey);
	stdDED(buff,TPK, TMK);
	stdDED(&buff[8],&TPK[8], TMK);
#ifdef _DEBUG_MK
	hex_dump_char("--TAK--",buff,KEY_LEN);
#endif
	if (store_acqTAKLogon(buff)<0) {
		dspClear();
		usrInfo(infLodTAKKeyFail);
		return (-1);
	}
	dspClear();
	usrInfo(infLodTAKSuccess);

   	return ret;
}

#endif

