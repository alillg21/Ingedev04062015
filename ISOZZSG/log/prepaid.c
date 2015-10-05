
#include "SDK30.h"
#include "OEM_Public.h"
#include "sys.h"
#undef __USE_DEL_INTERFACE__
#include "TlvTree.h"
#include "LinkLayer.h"
#include "oem_dgprs.h"
#ifdef __CHN_GPRS__
#include "gprs_lib.h"
#endif

#ifdef __CHN_SSL__
#include <string.h>
#include <SSL_.h>
#include <X509_.h>
#endif

#ifdef __CHN_FTP__
#include <FTP_.h>
#endif
#ifdef __CHN_TCP__
#include<IP_.h>
#endif

#ifdef __CHN_SCK__
#include<IP_.h>
#endif

#include <string.h>
#include "str.h"
#include "stdlib.h"
#include "lineEncryption.h"
#include "prepaid.h"
#include "bri.h"
#include "sys.h"
#include "log.h"


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
#ifdef __CHN_SCK__
        int hdlSck;
        struct sockaddr_in peeraddr_in; // For peer socket address.
#endif
#ifdef __CHN_GPRS__
        struct {                //chn= chnGprs
            unsigned int cTimeout;
            unsigned int cValue;
        } gprs;
#endif
#ifdef __CHN_SSL__
        SSL_HANDLE hdlSsl;
        SSL_PROFILE_HANDLE hdlProfile;
        char SslProfName[PROFILE_NAME_SIZE + 1];
#endif
    } prm;
    TLV_TREE_NODE hCfg;
    TLV_TREE_NODE hPhyCfg;
    TLV_TREE_NODE hDLinkCfg;
    TLV_TREE_NODE hTransCfg;
} tComChn;

extern byte txnAsli; //@agmr

tComChn prepaidCom;
byte prepaidRespCode;
//byte prepaidSamUid[UID_LEN];

//int isAksesKeReader(byte txnType)
//{
//    switch(txnType)
//    {
//        case trtPrepaidInfoDeposit:
//        case trtPrepaidPayment:
//        case trtTopUp:
//        case trtPrepaidSettlement:
////        case trtDeposit:
//            return 1;
//        default:
//            return 0;   
//    }
//}

static int prepaidComSetCom(const char *init) 
{

    int ret;
    byte datasize;              //data bits: 7 or 8
    byte parity;                //odd,even,none
    byte stopbits;              //1,2
    unsigned long sgmBaudRate = LL_PHYSICAL_V_BAUDRATE_1200;
    unsigned char physConf;

    trcFS("prepadiComSetCom %s\n", init);
    VERIFY(init);
    VERIFY(prepaidCom.chn == chnCom1 || prepaidCom.chn == chnCom2 || prepaidCom.chn == chnCom3);

    prepaidCom.prm.separator = '|';    //common for all types of chn
    prepaidCom.prm.conn_delay = 4 * 100;   //4 Second delay
    prepaidCom.prm.send_timeout = 30 * 100;
    prepaidCom.prm.ifconn = FALSE;

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

    prepaidCom.hCfg = TlvTree_New(LL_TAG_LINK_LAYER_CONFIG);

    prepaidCom.hPhyCfg =
        TlvTree_AddChild(prepaidCom.hCfg, LL_TAG_PHYSICAL_LAYER_CONFIG, NULL, 0);

//	physConf = LL_PHYSICAL_V_COM0;
    physConf =
        (prepaidCom.chn == chnCom1 ? LL_PHYSICAL_V_COM0 :
         prepaidCom.chn == chnCom1 ? LL_PHYSICAL_V_COM1 : LL_PHYSICAL_V_COM2);
    TlvTree_AddChildInteger(prepaidCom.hPhyCfg, LL_PHYSICAL_T_LINK, physConf,
                            LL_PHYSICAL_L_LINK);

    CHECK(sgmBaudRate, lblKOConfigure);
    TlvTree_AddChildInteger(prepaidCom.hPhyCfg, LL_PHYSICAL_T_BAUDRATE, sgmBaudRate,
                            LL_PHYSICAL_L_BAUDRATE);

    TlvTree_AddChildInteger(prepaidCom.hPhyCfg, LL_PHYSICAL_T_BITS_PER_BYTE, datasize,
                            LL_PHYSICAL_L_BITS_PER_BYTE);

    TlvTree_AddChildInteger(prepaidCom.hPhyCfg, LL_PHYSICAL_T_STOP_BITS, stopbits,
                            LL_PHYSICAL_L_STOP_BITS);

    TlvTree_AddChildInteger(prepaidCom.hPhyCfg, LL_PHYSICAL_T_PARITY, parity,
                            LL_PHYSICAL_L_PARITY);

//tambahan ++                            
    TlvTree_AddChildInteger(prepaidCom.hPhyCfg, LL_PHYSICAL_T_FLOW_CTRL, LL_PHYSICAL_V_NO_FLOW_CTRL,
                            LL_PHYSICAL_L_FLOW_CTRL);
//    prepaidCom.prm.hdl = NULL;                            
//tambahan --
    ret = LL_Configure(&prepaidCom.prm.hdl, prepaidCom.hCfg);
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

int openCom(void)
{
	int ret;
	
//	if (IsICT220())
//	    prepaidCom.chn = chnCom1;
//	else
	    prepaidCom.chn = chnCom1;

	prepaidCom.hCfg = NULL;
	prepaidCom.hDLinkCfg = NULL;
	prepaidCom.hPhyCfg = NULL;
	prepaidCom.hTransCfg = NULL;
	
//	if(prepaidComSetCom("8N19600") == -1)
	if(prepaidComSetCom("8N138400") == -1)
		return -1;

    ret = LL_Connect(prepaidCom.prm.hdl);
    if(ret != LL_ERROR_OK && ret != LL_ERROR_ALREADY_CONNECTED)
    	return -1;
    					
	return 0;	
}


int closeCom(void)
{
	int ret;
	
	LL_Configure(&prepaidCom.prm.hdl, NULL);
	if(prepaidCom.prm.hdl) 
	{
		ret = LL_Disconnect(prepaidCom.prm.hdl);
		if(ret!=LL_ERROR_OK)
			return -1;
		prepaidCom.prm.hdl = 0;
	}
	return 0;
}

#ifdef PREPAID
int checkResponseMessage(byte command, byte* msg, word *msgLen)
{
    int len=0;
	int i=0;
	byte isBegin=0, isEnd=0;

    //periksa panjang minimal data
    if(*msgLen < 14)
        return -1;

    //delete DLE
    while(i<*msgLen)
	{
		if(isBegin == 0)
		{	
			if(msg[i]==0x10 && msg[i+1]==0x02)
			{
				isBegin = 1;
				i++;
			}
		}
		else
		{
			if(msg[i]==0x10)
			{
				if(msg[i+1]==0x03)
				{
					isEnd=1;
					break;
				}
				else
				{	    
					memmove(&msg[i],&msg[i+1],*msgLen-(i+1));
					(*msgLen)--;
				}	
			}	
		}
		i++;
	}

    len = (msg[9]*0x100 + msg[10]);

#ifdef DEBUG_PREPAID_DATA
    debugData(0,"sesudah DLE",msg,*msgLen);
#endif
     prepaidRespCode=msg[11];
#ifdef DEBUG_PREPAID_DATA
    {
    	char buf[50];

    	sprintf(buf,"***** RESP CODE = %02X *****",prepaidRespCode);
    	debugData(0,buf,NULL,0);
    }
#endif    
    return len;
}

int buildMessage(byte command, byte* data, unsigned int dataLen, byte* msg, unsigned int msgLen)
{
	byte *ptr,*idx;
	byte lrc=0;
	int i ;
	byte t;
	
	
	ptr = msg;

	
	
	//START - ID
	if(ptr+2 >= msg+msgLen) //periksa panjang msg mencukupi atau tidak
	{    
	    prepaidRespCode=PREPAID_DATA_ERROR;
        return -1;
    }
	memcpy(ptr,"\x10\x02",2);
	ptr+=2;

	//HEADER
	if(ptr+7 >= msg+msgLen) //periksa panjang msg mencukupi atau tidak
	{    
	    prepaidRespCode=PREPAID_DATA_ERROR;
        return -1;
    }

	memcpy(ptr,"\x08\x00\x00\x00\x00\x00\x00",7);
	ptr+=7;

	//LENGTH = command 1 byte + data dataLen byte
	if(ptr+2 >= msg+msgLen) //periksa panjang msg mencukupi atau tidak
	{    
	    prepaidRespCode=PREPAID_DATA_ERROR;
        return -1;
    }	
	
	*ptr++ = (byte)((dataLen+1)/0x100);
    *ptr++ = (byte)((dataLen+1)%0x100);
   
    //data   
	if(ptr+dataLen+1 >= msg+msgLen) //periksa panjang msg mencukupi atau tidak
	{    
	    prepaidRespCode=PREPAID_DATA_ERROR;
        return -1;
    }   
    
    *ptr++ = command;
    if(dataLen!=0 || data!=NULL)
    {	
   	    memcpy(ptr,data,dataLen);
   	    ptr+= dataLen;
    }
   
    //LRC
	if(ptr+1 >= msg+msgLen) //periksa panjang msg mencukupi atau tidak
	{    
	    prepaidRespCode=PREPAID_DATA_ERROR;
        return -1;
    }   


	
		
    for(i=2;i<ptr-msg;i++)
    {
   	    t=msg[i];
   	    lrc^=msg[i];
    }
	
    *ptr++ = lrc;
      
    //Add 0x10(DLE character)
    idx = msg+2;
    while(idx < ptr)
    {
        switch(*idx)
        {
        	case 0x10:
        		if(ptr+1 >= msg+msgLen) //periksa panjang msg mencukupi atau tidak
        		    return -1;
        		memmove(idx+1,idx,ptr-idx);
        		*idx = 0x10;
        		ptr++;
        		idx++; //dibawah ada idx++ lagi, karena memang maju 2 byte
        		break;
        	default:
        		break;	
        }
        idx++;
    } 
    
    //END - ID
    if(ptr+2 >= msg+msgLen) //periksa panjang msg mencukupi atau tidak
	{   
	    prepaidRespCode=PREPAID_DATA_ERROR; 
        return -1;
    }
    memcpy(ptr,"\x10\x03",2);
    ptr+=2;
    	
	
#ifdef DEBUG_PREPAID_DATA  	
    debugData(0,"BUILD MESSAGE",msg,ptr-msg);
#endif	


	
    return ptr-msg;	
}

//int sendReceiveMessage(byte command, byte* msg, word *msgLen, word msgBufferSize)
//{
////buffer send dipakai untuk buffer receive juga
//	int ret;
//	
//	
//	openCom();
//	
//    ret = LL_Send(prepaidCom.prm.hdl, *msgLen, msg, LL_INFINITE);
//    if(ret != *msgLen)
//    {	
//    	ret = LL_GetLastError(prepaidCom.prm.hdl);
//    	goto lblKO;
//    }
//    	
//    ttestall(0, 100);           //to make sure sending is finished before receiving
//    	
//	memset(msg,0,msgBufferSize);
//	*msgLen = 0;
//
//    if(command == SALE_TRANS)
//        updatePrepaidRefNumber();
//
//	ret = LL_Receive(prepaidCom.prm.hdl, msgBufferSize, msg, 10*100);
//    if(ret == 0)
//    {
//    	ret = LL_GetLastError(prepaidCom.prm.hdl);
//    	goto lblKO;
//    }
//    ttestall(0, 100);
//#ifdef DEBUG_PREPAID_DATA   
//    debugData(0,"Recv",msg,ret);
//#endif   
//    if(ret <=0) 
//        goto lblKO;
//    *msgLen = ret;
//   
//	ret = checkResponseMessage(command, msg,msgLen);
//	goto lblEnd;
//	
//lblKO:
//    ret = -1;
//lblEnd:
//    closeCom();
//    return ret;
//}

int sendReceiveMessage(byte command, byte* msg, word *msgLen, word msgBufferSize)
{
//buffer send dipakai untuk buffer receive juga
	int ret;
	int idx;
	int len;
	word lenData;
	
	openCom();
	
    ret = LL_Send(prepaidCom.prm.hdl, *msgLen, msg, LL_INFINITE);
    if(ret != *msgLen)
    {	
    	ret = LL_GetLastError(prepaidCom.prm.hdl);
    	goto lblKO;
    }
    	
    ttestall(0, 100);           //to make sure sending is finished before receiving
    	
	memset(msg,0,msgBufferSize);
	*msgLen = 0;

    if(command == SALE_TRANS)
        updatePrepaidRefNumber();

    idx = 0;
    
    tmrStart(0, 20 * 100);
    while(tmrGet(0))
    {
	    ret = LL_Receive(prepaidCom.prm.hdl, msgBufferSize-idx, msg+idx, 10*100);
	    ttestall(0, 100);
        if(ret <= 0)
        {
    	    ret = LL_GetLastError(prepaidCom.prm.hdl);
    	    goto lblKO;
        }
        
        lenData = msg[9]*0x100;
        if(msg[9] == 0x10)
        {
            lenData += msg[11];
        }
        else
        {
            lenData += msg[10];
        }
        
        len = ret;
        idx+=len;
        if((memcmp(&msg[idx-2],"\x10\x03",2) == 0) && 
           (idx >= lenData+12 ))
        {
            
//            if(msg[len-3] != 0x10)
                break;
        }
    }
#ifdef DEBUG_PREPAID_DATA   
    debugData(0,"Recv",msg,ret);
#endif   
    if(ret <=0) 
        goto lblKO;
    //*msgLen = ret;
    *msgLen = idx;
   
	ret = checkResponseMessage(command, msg,msgLen);
	goto lblEnd;
	
lblKO:
    ret = -1;
lblEnd:
    closeCom();
    return ret;
}

int processToReader(byte command, byte* data, unsigned int dataLen, byte* msg, unsigned int msgLen)
{
    int ret;
    word len;
    
    memset(msg,0,msgLen);

	if((ret = buildMessage(command,data,dataLen,msg,msgLen))==-1)
	    return FAIL;

	len = (unsigned int)ret;
	if((ret = sendReceiveMessage(command,msg,&len,msgLen)) == -1)
	    return FAIL;
	
	return ret;    
}

int samInit(PREPAID_T *pData)
{
	int ret;
	byte msg[1024];
	byte *ptr;
    byte data[258];
    byte cmd = SAM_INIT;

#ifdef DEBUG_PREPAID_DATA
    debugData(0,"************************",NULL,0);	
    debugData(0,"INIT",NULL,0);	
    debugData(0,"************************",NULL,0);	
#endif
	
	memset(msg,0,sizeof(msg));
	memset(data,0,sizeof(data));
	ptr = data;

    decString2Hex(msg,3, pData->amountMin, AMOUNT_LEN_L);
    memcpy(ptr,msg,3);
    ptr+=3;
    
    memset(msg,0,sizeof(msg));
    decString2Hex(msg,4, pData->amountMax, AMOUNT_LEN_L);
    memcpy(ptr,msg,4);    
    ptr+=4;

    if((ret=processToReader(cmd, data, ptr-data, msg, sizeof(msg))) == -1)
        return FAIL;
	if(ret < 1) //1 byte command
	    return FAIL;

	pData->responseCode=prepaidRespCode;
	if(pData->responseCode!=0x30)
        return FAIL;

    return SUCCESS;
}

int infoSaldo(PREPAID_T *pData)
{
	int ret;
//	word len;
	byte msg[1024];
	byte *ptr;
    byte data[258];
    byte cmd = INF_SALDO;

#ifdef DEBUG_PREPAID_DATA	
    debugData(0,"************************",NULL,0);	
    debugData(0,"INFO",NULL,0);	
    debugData(0,"************************",NULL,0);
#endif	
	memset(msg,0,sizeof(msg));
	memset(data,0,sizeof(data));
	ptr = data;
		
	if((ret=processToReader(cmd, NULL, 0, msg, sizeof(msg))) == -1)
        return FAIL;	
	if(ret < 1) //1 byte command
	    return FAIL;
	pData->responseCode=prepaidRespCode;
	if(pData->responseCode!=PREPAID_SUCCESS)
        return FAIL;
        
    ptr = msg+12;
    memcpy(pData->cardNumber,ptr,CARD_NUMBER_LEN);
    ptr+=CARD_NUMBER_LEN;
    
    memcpy(pData->uid,ptr,UID_LEN);
    ptr+=UID_LEN;
    
    memcpy(pData->amount,ptr,AMOUNT_LEN_S);
    ptr+=AMOUNT_LEN_S;
    
#ifdef DEBUG_PREPAID_DATA
    debugData(0,"Card Number:",pData->cardNumber,CARD_NUMBER_LEN);
    debugData(0,"Amount:",pData->amount,AMOUNT_LEN_S);
    debugData(0,"Uid:",pData->uid,UID_LEN);
    ttestall(0,100);
#endif    
    return SUCCESS;
}

int infoInquiry(PREPAID_T *pData)
{
	int ret;
	byte msg[1024];
	byte *ptr;
    byte data[258];
    byte cmd = INFO_INQURIY;
    byte buf[25];

#ifdef DEBUG_PREPAID_DATA	
    debugData(0,"************************",NULL,0);	
    debugData(0,"INFO INQUIRY",NULL,0);	
    debugData(0,"************************",NULL,0);	
#endif	
	memset(msg,0,sizeof(msg));
	memset(data,0,sizeof(data));
	ptr = data;
		
    //date bcd ddmmyy
    hex2bin(buf, pData->date, 3);
    *ptr++ = buf[2];
    *ptr++ = buf[1];
    *ptr++ = buf[0];
    
    //Time bcd hhmmss
    hex2bin(buf, pData->time, 3);
    memcpy(ptr,buf,3);
    ptr+=3;          
    
    memset(msg,0,sizeof(msg));	
	if((ret=processToReader(cmd, data, ptr-data, msg, sizeof(msg))) == -1)
        return FAIL;	
	if(ret < 1) //1 byte command
	    return FAIL;
	pData->responseCode=prepaidRespCode;
	if(pData->responseCode!=PREPAID_SUCCESS)
        return FAIL;
        
    ptr = msg+12;
    memcpy(pData->cardNumber,ptr,CARD_NUMBER_LEN);
    ptr+=CARD_NUMBER_LEN;
    
    memcpy(pData->random,ptr,RANDOM_LEN);
    ptr+=RANDOM_LEN;
    
//    ptr+=18; //belum jelas ini apa
    
    memcpy(pData->amount,ptr,AMOUNT_LEN_S);
    ptr+=AMOUNT_LEN_S;
    
#ifdef DEBUG_PREPAID_DATA
    debugData(0,"Card Number:",pData->cardNumber,CARD_NUMBER_LEN);
    debugData(0,"Amount:",pData->amount,AMOUNT_LEN_S);
    debugData(0,"Random:",pData->random,RANDOM_LEN);
    ttestall(0,100);
#endif    
    return SUCCESS;
}

int displayAmount(PREPAID_T *pData)
{
	int ret;
	byte msg[1024];
	byte *ptr;
    byte data[258];
    byte cmd = DISPLAY_AMOUNT;

#ifdef DEBUG_PREPAID_DATA	
    debugData(0,"************************",NULL,0);	
    debugData(0,"DISPLAY AMOUNT",NULL,0);	
    debugData(0,"************************",NULL,0);	
#endif	
	memset(msg,0,sizeof(msg));
	memset(data,0,sizeof(data));
	ptr = data;
		
    //amount string dengan tanpa desimal
    memcpy(ptr,pData->amount,AMOUNT_LEN_S);
    ptr+=AMOUNT_LEN_S;
    
    //card number bcd
    memcpy(ptr,pData->cardNumber,CARD_NUMBER_LEN);
    ptr+=AMOUNT_LEN_S;
    			
	if((ret=processToReader(cmd, data, ptr-data, msg, sizeof(msg))) == -1)
        return FAIL;

#ifdef DEBUG_PREPAID_DATA
    ttestall(0,100);
#endif    
    return SUCCESS;
}

int prepaidSale(PREPAID_T *pData)
{
	int ret;
//	word len;
	byte msg[1024];
	byte *ptr;
    byte data[258];
    byte cmd = SALE_TRANS;
    byte buf[25];

#ifdef DEBUG_PREPAID_DATA	
    debugData(0,"************************",NULL,0);	
    debugData(0,"SALE",NULL,0);	
    debugData(0,"************************",NULL,0);	
#endif

	memset(msg,0,sizeof(msg));
	memset(data,0,sizeof(data));
	ptr = data;

    //Mid string
    memcpy(ptr,pData->mid,MID_LEN);
    ptr+=MID_LEN;
    
    //Tid string
    memcpy(ptr,pData->tid,TID_LEN);
    ptr+=TID_LEN;  
 
    //Date bcd yymmdd
    hex2bin(buf, pData->date, 3);
    memcpy(ptr,buf,3);
    ptr+=3; 
    
    //Time bcd hhmmss
    hex2bin(buf, pData->time, 3);
    memcpy(ptr,buf,3);
    ptr+=3;          

    //Transaction type
    *ptr++ = 0xEB;
    
    //amount hex (3 byte lsb..msb)
    memset(buf,0,sizeof(buf));
    decString2Hex(buf,3, pData->amount, AMOUNT_LEN_L-2);
    memcpy(ptr,buf,3);
    ptr+=3;    
    
    //amount string dengan desimal
    memset(buf,0,sizeof(buf));
    memcpy(buf,pData->amount,AMOUNT_LEN_L);
    
    memcpy(ptr,pData->amount,AMOUNT_LEN_L);
    ptr+=AMOUNT_LEN_L;
    
    //date string yymmdd
    memcpy(ptr,pData->date,DATE_LEN);
    ptr+=DATE_LEN;
    
    //time string hhmmss
    memcpy(ptr,pData->time,TIME_LEN);
    ptr+=TIME_LEN;
    
    //procode
    memcpy(ptr,PROCODE,6);
    ptr+=6;
    
    //ref number
    memset(buf,0,sizeof(buf));
    if((ret = getPrepaidRefNumber(buf))==0)
        return FAIL;
    memcpy(pData->refNumber,buf,REF_NUMBER_LEN);
    memcpy(ptr,buf,REF_NUMBER_LEN);
    ptr+=REF_NUMBER_LEN;    
    
    //batch number
    if((ret = getPrepaidBatchNumber(buf))==0)
        return FAIL;
          
    memcpy(pData->batchNumber,buf,BATCH_NUMBER_LEN);    
    memcpy(ptr,buf,BATCH_NUMBER_LEN);
    ptr+=BATCH_NUMBER_LEN;

   memcpy(ptr,"\xFF\xFF\xFF\xFF",4);
   ptr+=4;
	
    //send receive
    if((ret=processToReader(cmd, data, ptr-data, msg, sizeof(msg))) == -1)
        return FAIL;
    
	if(ret < 1) //1 byte command
	    return FAIL;

    ptr = msg+12;

    memcpy(pData->cardNumber,ptr,CARD_NUMBER_LEN);
    ptr+=CARD_NUMBER_LEN;

    memcpy(pData->uid,ptr,UID_LEN);
    ptr+=UID_LEN;

    memcpy(pData->amount,ptr,AMOUNT_LEN_S);
    ptr+=AMOUNT_LEN_S;
    
    memcpy(pData->hash,ptr,HASH_LEN);
    ptr+=HASH_LEN;    
    
#ifdef DEBUG_PREPAID_DATA
    debugData(0,"Card Number:",pData->cardNumber,CARD_NUMBER_LEN);
    debugData(0,"Amount:",pData->amount,AMOUNT_LEN_S);
    debugData(0,"uid:",pData->uid,UID_LEN);
    debugData(0,"Hash:",pData->hash,HASH_LEN);
    ttestall(0,100);
#endif        

	pData->responseCode=prepaidRespCode;
	if(pData->responseCode!=PREPAID_SUCCESS)
        return FAIL;

    return SUCCESS;
}

int topUp(PREPAID_T *pData)
{
	int ret;
	byte msg[1024];
	byte *ptr;
    byte data[258];
    byte cmd = TOP_UP;
    byte buf[25];

#ifdef DEBUG_PREPAID_DATA	
    debugData(0,"************************",NULL,0);	
    debugData(0,"TOP UP",NULL,0);	
    debugData(0,"************************",NULL,0);	
#endif
	memset(msg,0,sizeof(msg));
	memset(data,0,sizeof(data));
	ptr = data;

    //Mid string
    memcpy(ptr,pData->mid,MID_LEN);
    ptr+=MID_LEN;

    //Tid string
    memcpy(ptr,pData->tid,TID_LEN);
    ptr+=TID_LEN;
    
    //date bcd ddmmyy
    hex2bin(buf, pData->date, 3);
    *ptr++ = buf[2];
    *ptr++ = buf[1];
    *ptr++ = buf[0];
    
    //Time bcd hhmmss
    hex2bin(buf, pData->time, 3);
    memcpy(ptr,buf,3);
    ptr+=3;          
    
    //transaction type
    *ptr++ = 0xEC;

    //amount hex (3 byte lsb..msb)
    memset(buf,0,sizeof(buf));
    decString2Hex(buf,3, pData->amount, AMOUNT_LEN_L-2);
    memcpy(ptr,buf,3);
    ptr+=3;    
    
    //keyHost hex
    memcpy(ptr,pData->keyHost,KEY_HOST_LEN);
    ptr+=KEY_HOST_LEN;

    //send receive
    if((ret=processToReader(cmd, data, ptr-data, msg, sizeof(msg))) == -1)
        return FAIL;
    
	if(ret < 1) //1 byte command
	    return FAIL;

    ptr = msg+12;

    memcpy(pData->cardNumber,ptr,CARD_NUMBER_LEN);
    ptr+=CARD_NUMBER_LEN;
    
    memcpy(pData->uid,ptr,UID_LEN);
    ptr+=UID_LEN;
    
    memcpy(pData->amount,ptr,AMOUNT_LEN_S);
    ptr+=AMOUNT_LEN_S;
    
#ifdef DEBUG_PREPAID_DATA
    debugData(0,"Card Number:",pData->cardNumber,CARD_NUMBER_LEN);
    debugData(0,"Amount:",pData->amount,AMOUNT_LEN_S);
    debugData(0,"Uid:",pData->uid,UID_LEN);
    ttestall(0,100);
#endif        

	pData->responseCode=prepaidRespCode;
	if(pData->responseCode!=PREPAID_SUCCESS)
        return FAIL;

    return SUCCESS;
}

int prepaidAktivasi(PREPAID_T *pData)
{
	int ret;
	byte msg[1024];
	byte *ptr;
    byte data[258];
    byte cmd = TOP_UP;
    byte buf[25];

#ifdef DEBUG_PREPAID_DATA	
    debugData(0,"************************",NULL,0);	
    debugData(0,"AKTIVASI",NULL,0);	
    debugData(0,"************************",NULL,0);	
#endif
	memset(msg,0,sizeof(msg));
	memset(data,0,sizeof(data));
	ptr = data;

    //Mid string
    memcpy(ptr,pData->mid,MID_LEN);
    ptr+=MID_LEN;

    //Tid string
    memcpy(ptr,pData->tid,TID_LEN);
    ptr+=TID_LEN;
    
    //date bcd ddmmyy
    hex2bin(buf, pData->date, 3);
    *ptr++ = buf[2];
    *ptr++ = buf[1];
    *ptr++ = buf[0];
    
    //Time bcd hhmmss
    hex2bin(buf, pData->time, 3);
    memcpy(ptr,buf,3);
    ptr+=3;          
    
    //transaction type
    *ptr++ = 0xEF;

    //amount hex (3 byte lsb..msb)
    memset(buf,0,sizeof(buf));
    decString2Hex(buf,3, pData->amount, AMOUNT_LEN_L-2);
    memcpy(ptr,buf,3);
    ptr+=3;    
    
    //keyHost hex
    memcpy(ptr,pData->keyHost,KEY_HOST_LEN);
    ptr+=KEY_HOST_LEN;

    //send receive
    if((ret=processToReader(cmd, data, ptr-data, msg, sizeof(msg))) == -1)
        return FAIL;
    
	if(ret < 1) //1 byte command
	    return FAIL;

    ptr = msg+12;

    memcpy(pData->cardNumber,ptr,CARD_NUMBER_LEN);
    ptr+=CARD_NUMBER_LEN;
    
    memcpy(pData->uid,ptr,UID_LEN);
    ptr+=UID_LEN;
    
    memcpy(pData->amount,ptr,AMOUNT_LEN_S);
    ptr+=AMOUNT_LEN_S;
    
#ifdef DEBUG_PREPAID_DATA
    debugData(0,"Card Number:",pData->cardNumber,CARD_NUMBER_LEN);
    debugData(0,"Amount:",pData->amount,AMOUNT_LEN_S);
    debugData(0,"Uid:",pData->uid,UID_LEN);
    ttestall(0,100);
#endif

	pData->responseCode=prepaidRespCode;
	if(pData->responseCode!=PREPAID_SUCCESS)
        return FAIL;

    return SUCCESS;
}

int redeemInquiry(PREPAID_T *pData)
{
    return infoSaldo(pData);
}

int redeem(PREPAID_T *pData)
{
	int ret;
//	word len;
	byte msg[1024];
	byte *ptr;
    byte data[258];
    byte cmd = REDEEM;

#ifdef DEBUG_PREPAID_DATA	
    debugData(0,"************************",NULL,0);	
    debugData(0,"REDEEM",NULL,0);	
    debugData(0,"************************",NULL,0);	
#endif	
	memset(msg,0,sizeof(msg));
	memset(data,0,sizeof(data));
	ptr = data;
		
	if((ret=processToReader(cmd, NULL, 0, msg, sizeof(msg))) == -1)
        return FAIL;	
	if(ret < 1) //1 byte command
	    return FAIL;
	    
	pData->responseCode=prepaidRespCode;
	if(pData->responseCode!=PREPAID_SUCCESS)
        return FAIL;

#ifdef DEBUG_PREPAID_DATA
    ttestall(0,100);
#endif    
    
    return SUCCESS;
}

int infoKartu(PREPAID_T *pData)
{
	int ret;
//	word len;
	byte msg[1024];
	byte *ptr;
    byte data[258];
    byte cmd = INF_KARTU;

#ifdef DEBUG_PREPAID_DATA	
    debugData(0,"************************",NULL,0);	
    debugData(0,"INFO KARTU",NULL,0);	
    debugData(0,"************************",NULL,0);
#endif	
	memset(msg,0,sizeof(msg));
	memset(data,0,sizeof(data));
	ptr = data;
		
	if((ret=processToReader(cmd, NULL, 0, msg, sizeof(msg))) == -1)
        return FAIL;	
	if(ret < 1) //1 byte command
	    return FAIL;
	pData->responseCode=prepaidRespCode;
	if(pData->responseCode!=PREPAID_SUCCESS)
        return FAIL;
        
    ptr = msg+12;
    memcpy(pData->cardNumber,ptr,CARD_NUMBER_LEN);
    ptr+=CARD_NUMBER_LEN;
    
    memcpy(pData->issueDate,ptr,ISSUE_DATE_LEN);
    ptr+=ISSUE_DATE_LEN;
    
    memcpy(pData->issueCabang,ptr,ISSUE_CABANG_LEN);
    ptr+=ISSUE_CABANG_LEN;
    
    memcpy(pData->statusKartu,ptr,STATUS_KARTU_LEN);
    ptr+=STATUS_KARTU_LEN;    
    
    
#ifdef DEBUG_PREPAID_DATA
    debugData(0,"Card Number:",pData->cardNumber,CARD_NUMBER_LEN);
    debugData(0,"Issue Date:",pData->issueDate,ISSUE_DATE_LEN);
    debugData(0,"Issue Cabang:",pData->issueCabang,ISSUE_CABANG_LEN);
    debugData(0,"Status Kartu:",pData->statusKartu,STATUS_KARTU_LEN);
    ttestall(0,100);
#endif    
    return SUCCESS;    
}

int transLog(PREPAID_TRANS_LOG_T *pData)
{
	int ret;
//	word len;
	byte msg[1024];
	byte *ptr;
    byte data[258];
    byte cmd = TRANS_LOG;
    card temp;
    int i;

#ifdef DEBUG_PREPAID_DATA	
    debugData(0,"************************",NULL,0);	
    debugData(0,"TANS LOG",NULL,0);	
    debugData(0,"************************",NULL,0);
#endif	
	memset(msg,0,sizeof(msg));
	memset(data,0,sizeof(data));
	ptr = data;
		
	if((ret=processToReader(cmd, NULL, 0, msg, sizeof(msg))) == -1)
        return FAIL;	
	if(ret < 1) //1 byte command
	    return FAIL;
	pData->responseCode=prepaidRespCode;
	if(pData->responseCode!=PREPAID_SUCCESS)
        return FAIL;
        
    ptr = msg+12;
    memcpy(pData->cardNumber,ptr,CARD_NUMBER_LEN);
    ptr+=CARD_NUMBER_LEN;
    
    memcpy(pData->totalLogLen,ptr,TOTAL_LOG_LEN_LEN);
    ptr+=TOTAL_LOG_LEN_LEN;
    
    temp = (pData->totalLogLen[1])*0x100;
    temp += pData->totalLogLen[0];
    temp = temp/32; // banyaknya record
    pData->totalRec = temp;
    
    for(i=0;i<temp;i++)
    {
        memcpy(pData->log[i].tid,ptr,TID_LEN);
        ptr+=TID_LEN;
        memcpy(pData->log[i].mid,ptr,MID_LEN);
        ptr+=MID_LEN;
        memcpy(pData->log[i].date,ptr,DATE_LEN);
        ptr+=DATE_HEX_LEN; 
        memcpy(pData->log[i].time,ptr,TIME_LEN);
        ptr+=TIME_HEX_LEN;
        pData->log[i].transType = *ptr++;
        memcpy(pData->log[i].amountHex,ptr,AMOUNT_HEX_LEN);
        ptr+=AMOUNT_HEX_LEN; 
        memcpy(pData->log[i].currentBalance,ptr,AMOUNT_HEX_LEN);
        ptr+=AMOUNT_HEX_LEN; 
        memcpy(pData->log[i].lastUpdateBalance,ptr,AMOUNT_HEX_LEN);
        ptr+=AMOUNT_HEX_LEN;
    }

#ifdef DEBUG_PREPAID_DATA
    debugData(0,"Card Number:",pData->cardNumber,CARD_NUMBER_LEN);
    debugData(0,"Total Log Len:",pData->totalLogLen,TOTAL_LOG_LEN_LEN);
    for(i=0;i<temp;i++)
    {
        debugData(0,"REC :",(char*)&pData->log[i],32);
    }
    ttestall(0,100);
#endif    
    return SUCCESS;    
}

//++ @agmr - brizzi2
int reaktivasiInquiry(PREPAID_T *pData)
{
	int ret;
	byte msg[1024];
	byte *ptr;
    byte data[258];
    byte cmd = REAKTIVASI_INQUIRY;
    byte buf[25];

#ifdef DEBUG_PREPAID_DATA	
    debugData(0,"************************",NULL,0);	
    debugData(0,"REAKTIVASI INQUIRY",NULL,0);	
    debugData(0,"************************",NULL,0);
#endif	
	memset(msg,0,sizeof(msg));
	memset(data,0,sizeof(data));
	memset(buf,0,sizeof(buf));
	ptr = data;
		
    //date bcd ddmmyy
    hex2bin(buf, pData->date, 3);
    *ptr++ = buf[2];
    *ptr++ = buf[1];
    *ptr++ = buf[0];		
		
	if((ret=processToReader(cmd, NULL, 0, msg, sizeof(msg))) == -1)
        return FAIL;	
	if(ret < 1) //1 byte command
	    return FAIL;
	pData->responseCode=prepaidRespCode;
	if(pData->responseCode!=PREPAID_SUCCESS)
        return FAIL;
        
    ptr = msg+12;
    memcpy(pData->cardNumber,ptr,CARD_NUMBER_LEN);
    ptr+=CARD_NUMBER_LEN;
    
    memcpy(pData->uid,ptr,UID_LEN);
    ptr+=UID_LEN;
    
    memcpy(pData->amount,ptr,AMOUNT_LEN_S);
    ptr+=AMOUNT_LEN_S;   
    
    memcpy(pData->lastTransDate,ptr,LAST_TRANS_DATE_LEN/2);
    ptr+=LAST_TRANS_DATE_LEN/2;     

#ifdef DEBUG_PREPAID_DATA
    debugData(0,"Card Number:",pData->cardNumber,CARD_NUMBER_LEN);
    debugData(0,"Uid:",pData->uid,UID_LEN);
    debugData(0,"Balance:",pData->amount,AMOUNT_LEN_S);
    debugData(0,"Last Trans date:",pData->lastTransDate,LAST_TRANS_DATE_LEN/2);
    ttestall(0,100);
#endif    
    return SUCCESS;       
}

int reaktivasi(PREPAID_T *pData)
{
	int ret;
	byte msg[1024];
	byte *ptr;
    byte data[258];
    byte cmd = REAKTIVASI_TRANS;
    byte buf[25];

#ifdef DEBUG_PREPAID_DATA	
    debugData(0,"************************",NULL,0);	
    debugData(0,"REAKTIVASI",NULL,0);
    debugData(0,"************************",NULL,0);	
#endif

	memset(msg,0,sizeof(msg));
	memset(data,0,sizeof(data));
	memset(buf,0,sizeof(buf));
	ptr = data;

    //Mid string
    memcpy(ptr,pData->mid,MID_LEN);
    ptr+=MID_LEN;
    
    //Tid string
    memcpy(ptr,pData->tid,TID_LEN);
    ptr+=TID_LEN;  
 
    //Date bcd yymmdd
    hex2bin(buf, pData->date, 3);
    memcpy(ptr,buf,3);
    ptr+=3; 

    //Time bcd hhmmss
    hex2bin(buf, pData->time, 3);
    memcpy(ptr,buf,3);
    ptr+=3;          

    //Transaction type
    *ptr++ = 0x5F;
    
    //amount hex (3 byte lsb..msb)
    memset(buf,0,sizeof(buf));
    decString2Hex(buf,3, pData->amount, AMOUNT_LEN_L-2);
    memcpy(ptr,buf,3);
    ptr+=3;    
    
    //amount string dengan desimal
    memset(buf,0,sizeof(buf));
    memcpy(buf,pData->amount,AMOUNT_LEN_L);
    
    memcpy(ptr,pData->amount,AMOUNT_LEN_L);
    ptr+=AMOUNT_LEN_L;
    
    //date string yymmdd
    memcpy(ptr,pData->date,DATE_LEN);
    ptr+=DATE_LEN;
    
    //time string hhmmss
    memcpy(ptr,pData->time,TIME_LEN);
    ptr+=TIME_LEN;
    
    //procode
    memcpy(ptr,PROCODE,6);
    ptr+=6;
    
    //ref number
    memset(buf,0,sizeof(buf));
    if((ret = getPrepaidRefNumber(buf))==0)
        return FAIL;
    memcpy(pData->refNumber,buf,REF_NUMBER_LEN);
    memcpy(ptr,buf,REF_NUMBER_LEN);
    ptr+=REF_NUMBER_LEN;    
    
    //batch number
    if((ret = getPrepaidBatchNumber(buf))==0)
        return FAIL;
          
    memcpy(pData->batchNumber,buf,BATCH_NUMBER_LEN);    
    memcpy(ptr,buf,BATCH_NUMBER_LEN);
    ptr+=BATCH_NUMBER_LEN;
    
    memcpy(ptr,"\xFF\xFF\xFF\xFF",4);
    ptr+=4;

    //send receive
    if((ret=processToReader(cmd, data, ptr-data, msg, sizeof(msg))) == -1)
        return FAIL;
    
	if(ret < 1) //1 byte command
	    return FAIL;

    ptr = msg+12;
     
#ifdef DEBUG_PREPAID_DATA

#endif        

	pData->responseCode=prepaidRespCode;
	if(pData->responseCode!=PREPAID_SUCCESS)
        return FAIL;

    return SUCCESS;      
}

int voidBrizzi(PREPAID_VOID_T *pData)
{
	int ret;
	byte msg[1024];
	byte *ptr;
    byte data[258];
    byte cmd = VOID_BRIZZI;
    byte buf[25];

#ifdef DEBUG_PREPAID_DATA	
    debugData(0,"************************",NULL,0);	
    debugData(0,"VOID BRIZZI",NULL,0);	
    debugData(0,"************************",NULL,0);	
#endif
	memset(msg,0,sizeof(msg));
	memset(data,0,sizeof(data));
	ptr = data;

    //Mid string
    memcpy(ptr,pData->mid,MID_LEN);
    ptr+=MID_LEN;

    //Tid string
    memcpy(ptr,pData->tid,TID_LEN);
    ptr+=TID_LEN;
    
    //date bcd ddmmyy
    hex2bin(buf, pData->date, 3);
    *ptr++ = buf[2];
    *ptr++ = buf[1];
    *ptr++ = buf[0];
    
    //Time bcd hhmmss
    hex2bin(buf, pData->time, 3);
    memcpy(ptr,buf,3);
    ptr+=3;          
    
    //transaction type
    *ptr++ = 0xED;

    //amount hex (3 byte lsb..msb)
    memset(buf,0,sizeof(buf));
    decString2Hex(buf,3, pData->amount, AMOUNT_LEN_L-2);
    memcpy(ptr,buf,3);
    ptr+=3;    
    
    //keyHost hex
    memcpy(ptr,pData->keyHost,KEY_HOST_LEN);
    ptr+=KEY_HOST_LEN;

    //send receive
    if((ret=processToReader(cmd, data, ptr-data, msg, sizeof(msg))) == -1)
        return FAIL;
    
	if(ret < 1) //1 byte command
	    return FAIL;

    ptr = msg+12;

    memcpy(pData->cardNumber,ptr,CARD_NUMBER_LEN);
    ptr+=CARD_NUMBER_LEN;
    
    memcpy(pData->uid,ptr,UID_LEN);
    ptr+=UID_LEN;
    
    memcpy(pData->balance,ptr,AMOUNT_LEN_S);
    ptr+=AMOUNT_LEN_S;
    
#ifdef DEBUG_PREPAID_DATA
    debugData(0,"Card Number:",pData->cardNumber,CARD_NUMBER_LEN);
    debugData(0,"Balance:",pData->amount,AMOUNT_LEN_S);
    debugData(0,"Uid:",pData->uid,UID_LEN);
    ttestall(0,100);
#endif        

	pData->responseCode=prepaidRespCode;
	if(pData->responseCode!=PREPAID_SUCCESS)
        return FAIL;

    return SUCCESS;    
}

int ubahTanggal(char *pData)
{
	int ret;
//	word len;
	byte msg[1024];
	byte *ptr;
    byte data[258];
    byte cmd = UBAH_TANGGAL;

#ifdef DEBUG_PREPAID_DATA	
    debugData(0,"************************",NULL,0);	
    debugData(0,"UBAH TANGGAL",NULL,0);	
    debugData(0,"************************",NULL,0);
#endif	
	memset(msg,0,sizeof(msg));
	memset(data,0,sizeof(data));
	ptr = data;
	
	memcpy(data,pData,3);
	ptr+=3;	
	if((ret=processToReader(cmd, data, ptr-data, msg, sizeof(msg))) == -1)
        return FAIL;	
	if(ret < 1) //1 byte command
	    return FAIL;

	if(prepaidRespCode!=PREPAID_SUCCESS)
        return FAIL;
            
#ifdef DEBUG_PREPAID_DATA

#endif    
    return SUCCESS;    
}
//-- @agmr - brizzi2


int updatePrepaidRefNumber()
{
    card ref;
    int ret;
    
    ret = mapGetCard(regPrepaidRefNum,ref);
    if(ret < 0)
        return -1;
    if(ref >= 999999)
        ref = 1;
    else
        ref+=1;
    ret = mapPutCard(regPrepaidRefNum,ref);
    if(ret < 0)
        return -1;
    return 1;
}

int getPrepaidRefNumber(byte* refNum)
{
    card ref;
    int ret;
        
    ret = mapGetCard(regPrepaidRefNum,ref);
    if(ret < 0)
        return -1;

    num2dec(refNum, ref, REF_NUMBER_LEN);
	return 1;
}

int getPrepaidBatchNumber(byte* batchNum)
{
    byte batch;
    int ret;

    ret = mapGetByte(regPrepaidBatchNum,batch);
    if(ret < 0)
        return -1;
    
    num2dec(batchNum, batch, BATCH_NUMBER_LEN);
	return 1;
}

void xx_hex_dump(byte mode,char* title,byte* data, int len)
{
    char buf[500];
    
	memcpy(buf,"\x1B\x1A",2);
	sprintf(buf+2,"%s",title);
	prtS(buf);
    ShowData(data,len,0,0,15);
}

void decString2Hex(byte* hex, int hexLen, byte* dec, int decLen )
{
/*
    len = destination len max 4 byte
*/    
    card amt;
    
    dec2num(&amt,dec,decLen);// contoh :12ab34cd
    
    if(hexLen>3)
        hex[3] = amt/0x1000000;//->12 
    amt = amt%0x1000000;   //ab34cd
    if(hexLen>2)
        hex[2] = amt/0x10000;  //-> ab
    amt = amt%0x10000;     //34cd
    if(hexLen>1)
        hex[1] = amt/0x100;    //->34
    amt = amt%0x100;       //cd
    if(hexLen>0)
        hex[0] = amt;
    
    //memcpy(ptr,buf,3);
    //ptr+=3;    
}

void pmtBrizzi(void)
{
    int ret;
    word key, txnType,nextTxn;
    char traName[dspW + 1];
    char tid[9],mid[16];
    byte isInquiry=0;

    trcS("pmtBrizzi Beg\n");
    
    ret = mapGetWord(traMnuItm, key);
    CHECK(ret >= 0, lblEnd);
    VERIFY(key);

    memset(tid,0,sizeof(tid));
    memset(mid,0,sizeof(mid));
    
    ret = getPrepaidTidMid(tid,mid);
    if(ret<0)
        goto lblEnd;
    
    MAPPUTBYTE(traAcqIdx,(byte)ret,lblEnd);
    mapMove(acqBeg, (word)ret);
        
    ret = mapPut(acqTID,tid,8);
    if(ret<0)
        goto lblEnd;
    
    ret = mapPut(acqMID,mid,15);
    if(ret<0)
        goto lblEnd;

    switch(key)
    {
        case mnuInitialize:
            pmtPrepaidInit();     
            return;
	  case mnuSaldo:
	  	pmtPrepaidInitSaldo();  
	  	return;
        case mnuPrepaidInfoSaldo:
            pmtPrepaidInfoSaldo();       
            return;
        case mnuInfoDeposit:
            strcpy(traName,"INFO DEPOSIT");
            txnType = trtPrepaidInfoDeposit;  
            break;
        case mnuPrepaidPayment:
            strcpy(traName,"PAYMENT");
            txnType = trtPrepaidPayment;
            break;
	 case mnuPrepaidPaymentDisc:
	     strcpy(traName,"PAYMENT DISC");
            txnType = trtPrepaidPaymentDisc;
	     break;
        case mnuTopUpOnline:
            strcpy(traName,"TOP UP");
            MAPPUTBYTE(traInquiryFlag,1,lblEnd);
            txnType = trtPrepaidTopUp;
            break;
        case mnuTopUpDeposit:
            strcpy(traName,"TOP UP DEPOSIT");
            txnType = trtPrepaidDeposit;
            break;
        case mnuAktivasiDeposit:
            strcpy(traName,"AKTIVASI DEPOSIT");
            txnType = trtPrepaidAktivasi;
            break;
        case mnuPrepaidRedeem:
            strcpy(traName,"REDEEM");
            isInquiry = 1;
            txnType = trtPrepaidRedeemInquiry;
            nextTxn = trtPrepaidRedeem;
            break;
        //++@agmr - brizzi2
        case mnuPrepaidReaktivasi:
            strcpy(traName,"REAKTIVASI");
            isInquiry = 1;
            txnType = trtPrepaidReaktivasiInquiry;
            nextTxn = trtPrepaidReaktivasi;
            break; 
        case mnuPrepaidVoid:
            strcpy(traName,"VOID PEMBAYARAN");
            isInquiry = 1;
            txnType = trtPrepaidVoidInquiry;
            nextTxn = trtPrepaidVoid;
            break;
//--@agmr - brizzi2
        case mnuPrepaidInfoKartu:
            pmtPrepaidInfoKartu();
            return;
        case mnuPrepaidPrintLog:
        	pmtPrepaidPrintLog();
        	return;
        case mnuPrepaidSettlement:
            strcpy(traName,"PREPAID SETTLEMENT");
            txnType = trtPrepaidSettlement;            
            break;            
    }
    
    txnAsli = 0;
    MAPPUTBYTE(traInquiryFlag,isInquiry,lblEnd);
    mapMove(rqsBeg, (word) (txnType - 1));
    trcFS("traName: %s\n", traName);    
    MAPPUTSTR(traTransType, traName, lblEnd);
    MAPPUTSTR(traTypeName, traName, lblEnd);
    MAPPUTBYTE(traTxnType, txnType, lblEnd);
    MAPPUTBYTE(traNextTxn,nextTxn,lblEnd);
    MAPPUTBYTE(traEntMod,'t',lblEnd);
    MAPPUTBYTE(regLocType, 'T', lblEnd);

    pmtPayment(key);
        
    goto lblEnd;

lblEnd:
	return;
}

//int prepaidTrans(byte txnType)
//{
//    int ret=0;
//    byte isInquiry;
//
//    switch(txnType)
//    {
//      	case trtPrepaidInfoDeposit:
//      	    ret = pmtPrepaidInfoDeposit();
//      	    if(ret < 0)
//      	    	goto lblKO;
//            break;
//                    
//	    case trtPrepaidTopUp:
//	        MAPGETBYTE(traInquiryFlag,isInquiry,lblKO);
//	        if(isInquiry)
//	        {
//	    	    ret = pmtPrepaidInquiry();
//	      	    if(ret < 0)
//	      	    	goto lblKO;
//	    	    MAPPUTBYTE(traInquiryFlag,0,lblKO);
//	    	}
//	    	else 
//	    	{
//	    	    ret = pmtPrepaidTopUp();
//	      	    if(ret < 0)
//	      	    	goto lblKO;
//	    	}
//	        break;
//  	    case trtPrepaidPayment:
//            ret = pmtPrepaidPayment();
//      	    if(ret < 0)
//      	    	goto lblKO;
//            break;
//
//        case trtPrepaidRedeem:
//      	case trtPrepaidAktivasi:
//            break;
//            
//        default:
//            ret=0;
//            break;
//    }
//    goto lblEnd;
//
//lblKO:
//	ret = -1;
//lblEnd:
//	return ret;
//}

int autoSamInit()
{
    int ret;
    PREPAID_T pData;
    byte tmp;
    char minAmt[lenMinPrepaid + 1];
    char maxAmt[lenMaxPrepaid + 1]; 

    memset(&pData,0,sizeof(pData));
    memset(minAmt,0,sizeof(minAmt));
    memset(maxAmt,0,sizeof(maxAmt));

    ret = mapGet(appMinPrepaid,minAmt,lenMinPrepaid);
    CHECK(ret >= 0, lblKO);
    ret = mapGet(appMaxPrepaid,maxAmt,lenMaxPrepaid);
    CHECK(ret >= 0, lblKO);
	
    fmtPad(minAmt, -AMOUNT_LEN_L, '0');
    fmtPad(maxAmt, -AMOUNT_LEN_L, '0');
    memcpy(pData.amountMin,minAmt,AMOUNT_LEN_L);
    memcpy(pData.amountMax,maxAmt,AMOUNT_LEN_L);

    mapGetByte(appIsPrepaidEnable,tmp);
    if(tmp == 0) 
        return 1;
    
    ret = samInit(&pData);
    if(ret == FAIL)
        goto lblKO; 
//    usrInfo(infSamInitSuccess); 
    goto lblEnd;
    
lblKO:
    usrInfo(infSamInitFail);
    return -1;
lblEnd:
    return 1;    
}

void pmtPrepaidInitSaldo()
{
    int ret;
    PREPAID_T pData;
    char minAmt[lenMinPrepaid + 1];
    char maxAmt[lenMaxPrepaid + 1];    
    
    memset(&pData,0,sizeof(pData));
    memset(minAmt,0,sizeof(minAmt));
    memset(maxAmt,0,sizeof(maxAmt));
    ret = selectPrepaidInit(minAmt,maxAmt);
    if(ret < 0)
        goto lblKO;
    if(ret == 0)
        goto lblEnd;        
    fmtPad(minAmt, -AMOUNT_LEN_L, '0');
    fmtPad(maxAmt, -AMOUNT_LEN_L, '0');
    memcpy(pData.amountMin,minAmt,AMOUNT_LEN_L);
    memcpy(pData.amountMax,maxAmt,AMOUNT_LEN_L);

    ret = samInit(&pData);
    if(ret == FAIL)
        goto lblKO; 
    usrInfo(infSamInitSuccess); 
    goto lblEnd;
lblKO:
    usrInfo(infSamInitFail);
    return;
lblEnd:
    return;
}


void pmtPrepaidInit()
{
    int ret;
    PREPAID_T pData;
    char minAmt[lenMinPrepaid + 1];
    char maxAmt[lenMaxPrepaid + 1];    
    
    memset(&pData,0,sizeof(pData));
    memset(minAmt,0,sizeof(minAmt));
    memset(maxAmt,0,sizeof(maxAmt));
    strcpy(minAmt, "20000");
    strcpy(maxAmt, "20000000");
    fmtPad(minAmt, -AMOUNT_LEN_L, '0');
    fmtPad(maxAmt, -AMOUNT_LEN_L, '0');
    memcpy(pData.amountMin,minAmt,AMOUNT_LEN_L);
    memcpy(pData.amountMax,maxAmt,AMOUNT_LEN_L);

    ret = samInit(&pData);
    if(ret == FAIL)
        goto lblKO; 
    usrInfo(infSamInitSuccess); 
    goto lblEnd;
lblKO:
    usrInfo(infSamInitFail);
    return;
lblEnd:
    return;
}

int pmtPrepaidInfoSaldo()
{
    int ret;
    PREPAID_T pData;
    char buf[50];
    char amount[50];
    char str[50];

    usrInfo(infDekatkanKartu);
    
    memset(&pData,0,sizeof(pData));
    ret = infoSaldo(&pData);
    if(ret == FAIL)
    {
        usrInfo(infProcessToReaderFailWait);
        ret = -100;
        goto lblEnd;
    }
    dspClear();

    memset(buf,0,sizeof(buf));
	strcpy(buf,"Rp. ");
	fmtAmt(buf+4, pData.amount, 0, ",.");
    dspLS(1,"Saldo:");
    dspLS(2,buf);
    memset(amount,0,sizeof(amount));
    sprintf(amount,"Saldo %18s",buf);
    
	ret = usrInfo(infConfirmPrint);
	if(ret != kbdVAL)
		goto lblEnd;
		
    MAPPUTSTR(rptBuf1,"",lblKO);
    MAPPUTSTR(rptBuf2,"",lblKO);
    MAPPUTSTR(rptBuf3,"",lblKO);

    //simpan card Number
    memset(buf,0,sizeof(buf));
    bin2hex(buf,pData.cardNumber,CARD_NUMBER_LEN);
    MAPPUTSTR(traPan,buf,lblKO);
    MAPPUTSTR(traBlockPan,buf,lblKO);
 
    MAPPUTSTR(traCardType, "BRIZZI", lblKO);
    MAPPUTSTR(traChipSwipe, " (Fly)", lblKO);
    
    //judul
    MAPPUTSTR(rptBuf1,"INFORMASI SALDO",lblKO);
    
    //No. Kartu
    memset(buf,0,sizeof(buf));
    ret = mapGet(traPan,buf,sizeof(buf));
    CHECK(ret>=0,lblKO);

    sprintf(str,"No. Kartu      : %s",buf);
    MAPPUTSTR(rptBuf2,str,lblKO);
        
    //saldo
    MAPPUTSTR(rptBuf3,amount,lblKO);
    
    ret = rptReceipt(rloPrepaidInfoSaldo);
    CHECK(ret>=0,lblKO);
    goto lblEnd;

lblKO:
    ret = -1;
lblEnd:
	return ret;
}

int pmtPrepaidInfoDeposit()
{
    int ret;
    PREPAID_T pData;
    char amtStr[25];
    char buf[30];
    char amount[50];
    int temp;
    byte acquirerId;
    char str[50];

    usrInfo(infDekatkanKartu);
    
    memset(&pData,0,sizeof(pData));
    
    //date time
    memset(buf,0,sizeof(buf));
    MAPGET(traDatTim, buf, lblKO);
    memcpy(pData.date,buf+2,DATE_LEN);
    memcpy(pData.time,buf+8,TIME_LEN);
        
    ret = infoInquiry(&pData);
    if(ret == FAIL)
    {
        usrInfo(infProcessToReaderFailWait);
        ret =  -100;
        goto lblEnd; 
    }

    //simpan card Number
    memset(buf,0,sizeof(buf));
    bin2hex(buf,pData.cardNumber,CARD_NUMBER_LEN);
    MAPPUTSTR(traPan,buf,lblKO);
    
    //simpan sisa Saldo
    memset(buf,0,sizeof(buf));
    memcpy(buf,pData.amount,AMOUNT_LEN_L);
    mapPut(TRA_SISA_SALDO,buf,LEN_TRA_SISA_SALDO);
    
    ret = briGetAcq(DEBIT_CARD, &acquirerId);
    if(ret == FAIL)
        goto lblKO; 
    
    acquirerId--;
    
	MAPPUTBYTE(traAcqIdx, acquirerId, lblKO);
	mapMove(acqBeg, (word) acquirerId);        

    ret = onlSession();
    CHECK(ret >= 0, lblEnd);    

    dspClear();

     //++ @agmr - brizzi2
    ret = valRspCod();
    CHECK(ret > 0, lblEnd);
    //-- @agmr - brizzi2

    memset(amtStr,0,sizeof(amtStr));
    MAPGET(TRA_SALDO_DEPOSIT,amtStr,lblKO);
    temp = strlen(amtStr);
    if(temp>2)
    {
    	amtStr[temp-2]=0;
    }
	strcpy(buf,"Rp. ");
	fmtAmt(buf+4, amtStr, 0, ",.");
    dspLS(1,"Saldo deposit:");
    dspLS(2,buf);
    sprintf(amount,"Deposit %16s",buf);
//    usrInfo(infPressAnyKey);
    ret = usrInfo(infConfirmPrint);
	if(ret != kbdVAL)
		goto lblEnd;

    MAPPUTSTR(rptBuf1,"",lblKO);
    MAPPUTSTR(rptBuf2,"",lblKO);
    MAPPUTSTR(rptBuf3,"",lblKO);

    //simpan card Number
    memset(buf,0,sizeof(buf));
    bin2hex(buf,pData.cardNumber,CARD_NUMBER_LEN);
    MAPPUTSTR(traPan,buf,lblKO);
    MAPPUTSTR(traBlockPan,buf,lblKO);
 
    MAPPUTSTR(traCardType, "BRIZZI", lblKO);
    MAPPUTSTR(traChipSwipe, " (Fly)", lblKO);
    
    //judul
    MAPPUTSTR(rptBuf1,"INFORMASI DEPOSIT",lblKO);
    
    //No. Kartu
    memset(buf,0,sizeof(buf));
    ret = mapGet(traPan,buf,sizeof(buf));
    CHECK(ret>=0,lblKO);

    sprintf(str,"No. Kartu      : %s",buf);
    MAPPUTSTR(rptBuf2,str,lblKO);
        
    //saldo
    MAPPUTSTR(rptBuf3,amount,lblKO);
    
    ret = rptReceipt(rloPrepaidInfoSaldo);
    CHECK(ret>=0,lblKO); 
    
    goto lblEnd;
lblKO:
    ret = -1;
lblEnd:
	return ret;
}

int pmtPrepaidPaymentDisc()
{
	char buf[100];
    PREPAID_T pData;
    int ret;
    char tid[9],mid[16];
    
    memset(&pData,0,sizeof(pData));
    memset(tid,0,sizeof(tid));
    memset(mid,0,sizeof(mid));
    
    ret = getPrepaidTidMid(tid,mid);
    if(ret < 0)
        goto lblKO;
    
   //mid
    memcpy(pData.mid,mid+7,MID_LEN);
    
    //tid
    memcpy(pData.tid,tid,TID_LEN);
    
    //date time
    memset(buf,0,sizeof(buf));
    MAPGET(traDatTim, buf, lblKO);
    memcpy(pData.date,buf+2,DATE_LEN);
    memcpy(pData.time,buf+8,TIME_LEN);
    
    //amount
    memset(buf,0,sizeof(buf));
	ret = pmtAmountInput();
	CHECK(ret > 0, lblKO);
	MAPGET(traAmt, buf, lblKO);
	strcat(buf,"00");  
	
	fmtPad(buf,-AMOUNT_LEN_L,'0');
    memcpy(pData.amount,buf,AMOUNT_LEN_L);
    
    ret = usrInfo(infDekatkanKartu);
    ret=prepaidSale(&pData);
    if(ret==FAIL)
    {
        usrInfo(infProcessToReaderFailWait);
        ret = -100;
        goto lblEnd;
    }
    
    //simpan card Number
    memset(buf,0,sizeof(buf));
    bin2hex(buf,pData.cardNumber,CARD_NUMBER_LEN);
    MAPPUTSTR(traPan,buf,lblKO);
    MAPPUTSTR(TRA_NOMOR_KARTU_BRIZZI,buf,lblKO);
    
    //simpan proc code transaksi
    MAPPUTSTR(TRA_PROCODE,PROCODE,lblKO);
    
    //simpan prepaid ref number transaksi
    memset(buf,0,sizeof(buf));
    memcpy(buf,pData.refNumber,REF_NUMBER_LEN);
    MAPPUTSTR(TRA_REF_NUMBER,buf,lblKO);
    
    //update ref number
    if(updatePrepaidRefNumber()<0)
        goto lblKO;
    
    //simpan hash
    memset(buf,0,sizeof(buf));
    memcpy(buf,pData.hash,HASH_LEN);
    ret=mapPut(TRA_HASH,buf,LEN_TRA_HASH);
    if(ret<0)
        goto lblKO;
    
    //simpan batch Number
    memset(buf,0,sizeof(buf));
    memcpy(buf,pData.batchNumber,BATCH_NUMBER_LEN);
    MAPPUTSTR(TRA_PREPAID_BATCH_NUM,buf,lblKO);
    
    //simpan sisa Saldo
    memset(buf,0,sizeof(buf));
    memcpy(buf,pData.amount,AMOUNT_LEN_L);
    MAPPUTSTR(TRA_SISA_SALDO,buf,lblKO);
//    mapPut(TRA_SISA_SALDO,buf,LEN_TRA_SISA_SALDO);

	ret = valBin();
	CHECK(ret > 0, lblKO);
	    
    goto lblEnd;
    
lblKO:
    if(ret != -100) //@agmr
        ret = -1;
lblEnd:
    return ret;    

}


int pmtPrepaidPayment()
{
    char buf[100], buf1[100];
    PREPAID_T pData;
    int ret;
    char tid[9],mid[16];
#ifdef _USE_ECR
	byte appIsECRbyte;
#endif
    //char buf[25];
    
    memset(&pData,0,sizeof(pData));
    memset(tid,0,sizeof(tid));
    memset(mid,0,sizeof(mid));
    
    ret = getPrepaidTidMid(tid,mid);
    if(ret < 0)
        goto lblKO;
    
   //mid
    memcpy(pData.mid,mid+7,MID_LEN);
    
    //tid
    memcpy(pData.tid,tid,TID_LEN);
    
    //date time
    memset(buf,0,sizeof(buf));
    MAPGET(traDatTim, buf, lblKO);
    memcpy(pData.date,buf+2,DATE_LEN);
    memcpy(pData.time,buf+8,TIME_LEN);
    
    //amount
    #ifdef _USE_ECR
	MAPGETBYTE(appIsECR,appIsECRbyte,lblKO);
	if(appIsECRbyte == 1){
		memset(buf,0,sizeof(buf));
		memset(buf1,0,sizeof(buf1));
		mapGet(appAmtECR,buf1,sizeof(buf1));
		sprintf(buf, "%d", atoi(buf1));
		MAPPUTSTR(traAmt, buf, lblKO);
		strcat(buf,"00");
	}else{
    #endif
		memset(buf,0,sizeof(buf));
		ret = pmtAmountInput();
		CHECK(ret > 0, lblKO);
		MAPGET(traAmt, buf, lblKO);
		strcat(buf,"00");
    #ifdef _USE_ECR
	}
    #endif
	
	fmtPad(buf,-AMOUNT_LEN_L,'0');
    memcpy(pData.amount,buf,AMOUNT_LEN_L);
    
    ret = usrInfo(infDekatkanKartu);
    ret=prepaidSale(&pData);
    if(ret==FAIL)
    {
        usrInfo(infProcessToReaderFailWait);
        ret = -100;
        goto lblEnd;
    }
    
    //simpan card Number
    memset(buf,0,sizeof(buf));
    bin2hex(buf,pData.cardNumber,CARD_NUMBER_LEN);
    MAPPUTSTR(traPan,buf,lblKO);
    MAPPUTSTR(TRA_NOMOR_KARTU_BRIZZI,buf,lblKO);
    
    //simpan proc code transaksi
    MAPPUTSTR(TRA_PROCODE,PROCODE,lblKO);
    
    //simpan prepaid ref number transaksi
    memset(buf,0,sizeof(buf));
    memcpy(buf,pData.refNumber,REF_NUMBER_LEN);
    MAPPUTSTR(TRA_REF_NUMBER,buf,lblKO);
    
    //update ref number
    if(updatePrepaidRefNumber()<0)
        goto lblKO;
    
    //simpan hash
    memset(buf,0,sizeof(buf));
    memcpy(buf,pData.hash,HASH_LEN);
    ret=mapPut(TRA_HASH,buf,LEN_TRA_HASH);
    if(ret<0)
        goto lblKO;
    
    //simpan batch Number
    memset(buf,0,sizeof(buf));
    memcpy(buf,pData.batchNumber,BATCH_NUMBER_LEN);
    MAPPUTSTR(TRA_PREPAID_BATCH_NUM,buf,lblKO);
    
    //simpan sisa Saldo
    memset(buf,0,sizeof(buf));
    memcpy(buf,pData.amount,AMOUNT_LEN_L);
    MAPPUTSTR(TRA_SISA_SALDO,buf,lblKO);
//    mapPut(TRA_SISA_SALDO,buf,LEN_TRA_SISA_SALDO);

	ret = valBin();
	CHECK(ret > 0, lblKO);
	    
    goto lblEnd;
    
lblKO:
    if(ret != -100) //@agmr
        ret = -1;
lblEnd:
    return ret;    
}

int pmtPrepaidInquiry()
{
    int ret;
    PREPAID_T pData;
    char buf[100];

    usrInfo(infDekatkanKartu);
    
    memset(&pData,0,sizeof(pData));
    
    //date time
    memset(buf,0,sizeof(buf));
    MAPGET(traDatTim, buf, lblKO);
    memcpy(pData.date,buf+2,DATE_LEN);
    memcpy(pData.time,buf+8,TIME_LEN);
        
    ret = infoInquiry(&pData);
    if(ret == FAIL)
    {
        usrInfo(infProcessToReaderFailWait);
        ret = -100;
        goto lblEnd; 
    }

    //simpan balance amount
    memset(buf,0,sizeof(buf));
    memcpy(buf,pData.amount,AMOUNT_LEN_L);
    mapPut(TRA_SISA_SALDO,buf,LEN_TRA_SISA_SALDO);
    mapPut(TRA_SALDO_AWAL,buf,LEN_TRA_SALDO_AWAL); //@agmr - dipakai untuk aktivasi
    
    //simpan random
    memset(buf,0,sizeof(buf));
    memcpy(buf,pData.random,KEY_HOST_LEN);
    ret=mapPut(TRA_KEY_HOST,buf,LEN_TRA_KEY_HOST);
    if(ret<0)
        goto lblKO;   
        
    //simpan nomor kartu
    memset(buf,0,sizeof(buf));
    bin2hex(buf,pData.cardNumber,CARD_NUMBER_LEN);
    ret = mapPut(TRA_NOMOR_KARTU_BRIZZI,buf,LEN_TRA_NOMOR_KARTU_BRIZZI);  
    if(ret<0)
        goto lblKO;
 
    dspClear();
    
    goto lblEnd;
lblKO:
    ret = -1;
lblEnd:
	return ret;
}

int pmtPrepaidTopUp()
{
    char buf[100], buf1[100];
    PREPAID_T pData;
    int ret;
    char tid[9],mid[16];
	byte appIsECRbyte;
//    char amtStr[25];
//    int temp;

    usrInfo(infDekatkanKartu);

#ifdef TEST_PREPAID_REVERSAL
    dspClear();
    dspLS(2,"BIKIN REVERSAL ??");
	if(acceptable_key(kbdVAL_kbdANN, 1000) == kbdVAL)
	    goto lblKO;
#endif

    memset(&pData,0,sizeof(pData));
    memset(tid,0,sizeof(tid));
    memset(mid,0,sizeof(mid));
    
    ret = getPrepaidTidMid(tid,mid);
    if(ret < 0)
        goto lblKO;
    
   //mid
    memcpy(pData.mid,mid+7,MID_LEN);
    
    //tid
    memcpy(pData.tid,tid,TID_LEN);
    
    //date time
    memset(buf,0,sizeof(buf));
    MAPGET(traDatTim, buf, lblKO);
    memcpy(pData.date,buf+2,DATE_LEN);
    memcpy(pData.time,buf+8,TIME_LEN);

	//amount
    #ifdef _USE_ECR
	MAPGETBYTE(appIsECR,appIsECRbyte,lblKO);
	if(appIsECRbyte == 1){
		memset(buf,0,sizeof(buf));
		memset(buf1,0,sizeof(buf1));
		mapGet(appAmtECR,buf1,sizeof(buf1));
		sprintf(buf, "%d", atoi(buf1));
		MAPPUTSTR(traAmt, buf, lblKO);
		strcat(buf,"00");
	}else{
    #endif
    	memset(buf,0,sizeof(buf));
		MAPGET(traAmt, buf, lblKO);
		strcat(buf,"00"); 
    #ifdef _USE_ECR
	}
    #endif
	
	fmtPad(buf,-AMOUNT_LEN_L,'0');
    memcpy(pData.amount,buf,AMOUNT_LEN_L);
    
    //key host
    memset(buf,0,sizeof(buf));
//    MAPGET(TRA_KEY_HOST,buf,LEN_TRA_KEY_HOST);
    ret = mapGet(TRA_KEY_HOST,buf,LEN_TRA_KEY_HOST);
    if(ret<0)
    	goto lblEnd;
    memcpy(pData.keyHost,buf,KEY_HOST_LEN);
                
    ret = topUp(&pData);
    if(ret == FAIL) //NANTI DIBUKA !!!
    {
        usrInfo(infProcessToReaderFail);
        ret = -100;
        goto lblEnd;
    }
//    memcpy(pData.cardNumber,"\x12\x34\x56\x78\x90\x12\x12\x12",16); //nanti hapus

    //simpan card Number
    memset(buf,0,sizeof(buf));
    bin2hex(buf,pData.cardNumber,CARD_NUMBER_LEN);
    MAPPUTSTR(TRA_NOMOR_KARTU_BRIZZI,buf,lblKO);    
//    MAPPUTSTR(traPan,buf,lblKO);
    
    //simpan sisa Saldo
    memset(buf,0,sizeof(buf));
    memcpy(buf,pData.amount,AMOUNT_LEN_L);
    mapPut(TRA_SISA_SALDO,buf,LEN_TRA_SISA_SALDO);
    
    dspClear();

    goto lblEnd;
lblKO:
//    usrInfo(infProcessToReaderFail);
    ret = -1;
lblEnd:
	return ret;    
}

int pmtPrepaidAktivasiDeposit()
{
    char buf[100];
    PREPAID_T pData;
    int ret;
    char tid[9],mid[16];

    usrInfo(infDekatkanKartu);
#ifdef TEST_PREPAID_REVERSAL
    dspClear();
    dspLS(2,"BIKIN REVERSAL ??");
	if(acceptable_key(kbdVAL_kbdANN, 1000) == kbdVAL)
	    goto lblKO;
#endif
    memset(&pData,0,sizeof(pData));
    memset(tid,0,sizeof(tid));
    memset(mid,0,sizeof(mid));
    
    ret = getPrepaidTidMid(tid,mid);
    if(ret < 0)
        goto lblKO;

   //mid
    memcpy(pData.mid,mid+7,MID_LEN);
    
    //tid
    memcpy(pData.tid,tid,TID_LEN);
    
    //date time
    memset(buf,0,sizeof(buf));
    MAPGET(traDatTim, buf, lblKO);
    memcpy(pData.date,buf+2,DATE_LEN);
    memcpy(pData.time,buf+8,TIME_LEN);
    
    //amount
    memset(buf,0,sizeof(buf));
	MAPGET(TRA_SALDO_DEPOSIT, buf, lblKO);
	
	fmtPad(buf+2,-AMOUNT_LEN_L,'0');
    memcpy(pData.amount,buf+2,AMOUNT_LEN_L);
    
    //key host
    memset(buf,0,sizeof(buf));
    ret = mapGet(TRA_KEY_HOST,buf,LEN_TRA_KEY_HOST);
    if(ret<0)
    	goto lblEnd;
    memcpy(pData.keyHost,buf,KEY_HOST_LEN);
                
    ret = prepaidAktivasi(&pData);
    if(ret == FAIL) //NANTI DIBUKA !!!
    {
        usrInfo(infProcessToReaderFail);
        ret = -100;
        goto lblEnd;
    }

    //simpan card Number
    memset(buf,0,sizeof(buf));
    bin2hex(buf,pData.cardNumber,CARD_NUMBER_LEN);
    MAPPUTSTR(traPan,buf,lblKO);
    
    //simpan sisa Saldo
    memset(buf,0,sizeof(buf));
    memcpy(buf,pData.amount,AMOUNT_LEN_L);
    mapPut(TRA_SISA_SALDO,buf,LEN_TRA_SISA_SALDO);
    
    dspClear();

    goto lblEnd;
lblKO:
    ret = -1;
lblEnd:
	return ret;        
}

int pmtPrepaidRedeemInquiry()
{
    char buf[25];
    PREPAID_T pData;
    int ret;

    usrInfo(infDekatkanKartu);               
    ret = redeemInquiry(&pData);
    if(ret == FAIL)
    {
        usrInfo(infProcessToReaderFail);
        ret = -100;
        goto lblEnd;
    }
    
    //simpan card Number
    memset(buf,0,sizeof(buf));
    bin2hex(buf,pData.cardNumber,CARD_NUMBER_LEN);
    MAPPUTSTR(TRA_NOMOR_KARTU_BRIZZI,buf,lblKO);
    MAPPUTSTR(traPan,buf,lblKO);

    //simpan sisa saldo
    memset(buf,0,sizeof(buf));
    memcpy(buf,pData.amount,AMOUNT_LEN_L);
    mapPut(TRA_SISA_SALDO,buf,LEN_TRA_SISA_SALDO);
        
    dspClear();
    goto lblEnd;
lblKO:
    ret = -1;
lblEnd:
	return ret;       
}

int pmtPrepaidRedeem()
{
//    char buf[25];
    PREPAID_T pData;
    int ret = SUCCESS;

//    usrInfo(infDekatkanKartu); 
    
#ifdef TEST_PREPAID_REVERSAL
    dspClear();
    dspLS(2,"BIKIN REVERSAL ??");
	if(acceptable_key(kbdVAL_kbdANN, 1000) == kbdVAL)
	    goto lblKO;
#endif

    ret = redeem(&pData); //NANTI DIBUKA !!!!!
    if(ret == FAIL)
    {
        usrInfo(infProcessToReaderFail);
        ret = -100;
        goto lblEnd;
    }
    
    dspClear();
    goto lblEnd;
    ret = -1;
lblEnd:
	return ret;   
}

int pmtPrepaidInfoKartu()
{
    char buf[50];
    char str[50];
    PREPAID_T pData;
    int ret;
    char tid[9],mid[16];

    usrInfo(infDekatkanKartu);

    memset(&pData,0,sizeof(pData));
    memset(tid,0,sizeof(tid));
    memset(mid,0,sizeof(mid));

    ret = infoKartu(&pData);
    if(ret == FAIL)
    {        
        usrInfo(infProcessToReaderFailWait);
        ret = -100;
        goto lblEnd;
    }
    
    MAPPUTSTR(rptBuf1,"",lblKO);
    MAPPUTSTR(rptBuf2,"",lblKO);
    MAPPUTSTR(rptBuf3,"",lblKO);
    MAPPUTSTR(rptBuf4,"",lblKO);

    //simpan card Number
    memset(buf,0,sizeof(buf));
    bin2hex(buf,pData.cardNumber,CARD_NUMBER_LEN);
    MAPPUTSTR(traPan,buf,lblKO);
    MAPPUTSTR(traBlockPan,buf,lblKO);
 
    MAPPUTSTR(traCardType, "BRIZZI", lblKO);
    MAPPUTSTR(traChipSwipe, " (Fly)", lblKO);
    
    //judul
    MAPPUTSTR(rptBuf1,"INFORMASI STATUS KARTU",lblKO);
    
    //No. Kartu
    memset(buf,0,sizeof(buf));
    ret = mapGet(traPan,buf,sizeof(buf));
    CHECK(ret>=0,lblKO);
    
    sprintf(str,"No. Kartu      : %s",buf);
    MAPPUTSTR(rptBuf2,str,lblKO);
    
    //Issu date
    memset(buf,0,sizeof(buf));
    bin2hex(buf,pData.issueDate,3);
    sprintf(str,"Issue Date     : %c%c/%c%c/%c%c",buf[0],buf[1],buf[2],buf[3],buf[4],buf[5]);
    MAPPUTSTR(rptBuf3,str,lblKO);
    
    //Issue Cabang
    memset(buf,0,sizeof(buf));
    bin2hex(buf,pData.issueCabang,2);
    sprintf(str,"Issue Cabang   : %s",buf);
    MAPPUTSTR(rptBuf4,str,lblKO);
    
    //Status Kartu
    memset(buf,0,sizeof(buf));
    if(memcmp(pData.statusKartu,"aa",2) == 0)
        strcpy(buf,"ACTIVE");
    else
        strcpy(buf,"NOT ACTIVE");
        
    sprintf(str,"Status Kartu   : %s",buf);    
    MAPPUTSTR(rptBuf5,str,lblKO);

    ret = rptReceipt(rloPrepaidInfoKartu);
    CHECK(ret>=0,lblKO);

    goto lblEnd;

lblKO:
    ret = -1;
lblEnd:
	return ret;
}

int pmtPrepaidPrintLog()
{
    char buf[50];
    char str[50];
    int i;
    int ret;
//    int temp;
    char date[20];
	char time[20];
	char tid[20];
    char trans[20];
    char amount[20];
    card amt;
    PREPAID_TRANS_LOG_T pData;

    memset(&pData,0,sizeof(pData));
    ret = transLog(&pData);
    CHECK(ret==SUCCESS,lblKO);
    
    MAPPUTSTR(rptBuf1,"",lblKO);
    MAPPUTSTR(rptBuf2,"",lblKO);
    MAPPUTSTR(rptBuf3,"",lblKO);
    MAPPUTSTR(rptBuf4,"",lblKO);
    MAPPUTSTR(rptBuf5,"",lblKO);
    MAPPUTSTR(rptBuf6,"",lblKO);
    
    MAPPUTSTR(rptBuf1,"PRINT LAST LOG",lblKO);    
    
    //simpan card Number
    memset(buf,0,sizeof(buf));
    bin2hex(buf,pData.cardNumber,CARD_NUMBER_LEN);
    MAPPUTSTR(traPan,buf,lblKO);
    MAPPUTSTR(traBlockPan,buf,lblKO);  
    
    MAPPUTSTR(traCardType, "BRIZZI", lblKO);
    MAPPUTSTR(traChipSwipe, " (Fly)", lblKO);
    
    sprintf(str,"No. Kartu Brizzi : %s",buf);
    MAPPUTSTR(rptBuf2,str,lblKO);

	
	
    memset(str,0,sizeof(str));
    memset(str,0x20,48);
    memcpy(str,"DATE",4);
	memcpy(str+10,"TIME",4);
	memcpy(str+21,"TID",3);
    memcpy(str+29,"TRANSAKSI",9);
    memcpy(str+41,"AMOUNT",6);

    ret = usrInfo(infPrinting);

//    sprintf(str,"%s%20s%20s","Date","Transaksi","Amount");
    MAPPUTSTR(rptBuf3,str,lblKO);

    ret = rptReceipt(rloPrepaidTransLog);
    CHECK(ret>=0,lblKO);
    
    for(i=0;i<pData.totalRec;i++)
    {
        memset(date,0,sizeof(date));
        memset(trans,0,sizeof(trans));
        memset(amount,0,sizeof(amount));
		memset(tid,0,sizeof(tid));
		bin2hex(tid,pData.log[i].tid,8);
        
        //Trans date
        memset(buf,0,sizeof(buf));
        bin2hex(buf,pData.log[i].date,3);
        sprintf(date,"%c%c/%c%c/%c%c",buf[0],buf[1],buf[2],buf[3],buf[4],buf[5]);

		//Trans time
        memset(buf,0,sizeof(buf));
        bin2hex(buf,pData.log[i].time,3);
        sprintf(time,"%c%c:%c%c:%c%c",buf[0],buf[1],buf[2],buf[3],buf[4],buf[5]);
		
        //Jenis transaksi
        memset(buf,0,sizeof(buf));
        switch(pData.log[i].transType)
        {
            case 0xeb:
                strcpy(trans,"PAYMENT");
                break;
            case 0xec:
                strcpy(trans,"TOPUP");
                break;
            case 0xef:
                strcpy(trans,"AKTIVASI");
                break;
	    //++ @agmr - brizzi2
            case 0xed:
                strcpy(trans,"VOID");
                break;
            case 0x5f:
                strcpy(trans,"REAKTIV");
                break;
//-- @agmr - brizzi2
        }
        
        //Amount
        memset(buf,0,sizeof(buf));
        memset(amount,0,sizeof(amount));

        //memcpy(pData.log[i].amountHex,"\xff\xff\xff",3);

        amt = pData.log[i].amountHex[0] + pData.log[i].amountHex[1]*0x100 + pData.log[i].amountHex[2]*0x10000;
        strcpy(amount,"Rp. ");
        num2dec(buf, amt, sizeof(buf)-4);
		fmtAmt(amount+4, buf, 0, ",.");

	    //gabung
	    memset(str,0,sizeof(str));
	    memset(str,0x20,48);
	    memcpy(str,date,strlen(date));
		memcpy(str+9,time,strlen(time));
		memcpy(str+18,tid,strlen(tid));
	    memcpy(str+27,trans,strlen(trans));
	    memcpy(str+36,amount,strlen(amount));
	    
	    MAPPUTSTR(rptBuf5,str,lblKO);
        
        ret = rptReceipt(rloPrepaidTransLogIsi);
        CHECK(ret>=0,lblKO);
    }
    
    ret = rptReceipt(rloPrepaidTransLogBawah);
    CHECK(ret>=0,lblKO);
        
    goto lblEnd;
lblKO:
    ret = -1;
lblEnd:
	return ret;              
}

//++ @agmr - brizzi2
int pmtPrepaidReaktivasiInquiry()
{
    char buf[25];
    PREPAID_T pData;
    int ret;

    usrInfo(infDekatkanKartu);  
    
    //date time
    memset(buf,0,sizeof(buf));
    MAPGET(traDatTim, buf, lblKO);
    memcpy(pData.date,buf+2,DATE_LEN);
    memcpy(pData.time,buf+8,TIME_LEN);    
                 
    ret = reaktivasiInquiry(&pData);
    if(ret == FAIL)
    {
        usrInfo(infProcessToReaderFail);
        ret = -100;
        goto lblEnd;
    }
    
    //simpan card Number
    memset(buf,0,sizeof(buf));
    bin2hex(buf,pData.cardNumber,CARD_NUMBER_LEN);
    MAPPUTSTR(TRA_NOMOR_KARTU_BRIZZI,buf,lblKO);
    MAPPUTSTR(traPan,buf,lblKO);

    //simpan sisa saldo
    memset(buf,0,sizeof(buf));
    memcpy(buf,pData.amount,AMOUNT_LEN_L);
    mapPut(TRA_SISA_SALDO,buf,LEN_TRA_SISA_SALDO);
        
    //simpan last trans date    
    memset(buf,0,sizeof(buf));
//    memcpy(buf,pData.lastTransDate,LAST_TRANS_DATE_LEN/2);
    bin2hex(buf,pData.lastTransDate,LAST_TRANS_DATE_LEN/2);
    mapPut(TRA_LAST_TRANS_DATE,buf,LAST_TRANS_DATE_LEN);
            
    dspClear();
    goto lblEnd;
lblKO:
    ret = -1;
lblEnd:
	return ret;       
}

int pmtPrepaidReaktivasi()
{
    char buf[100];
    PREPAID_T pData;
    int ret;
    char tid[9],mid[16];
    
//    usrInfo(infDekatkanKartu);

    memset(&pData,0,sizeof(pData));
    memset(tid,0,sizeof(tid));
    memset(mid,0,sizeof(mid));
    
    ret = getPrepaidTidMid(tid,mid);
    if(ret < 0)
        goto lblKO;
    
   //mid
    memcpy(pData.mid,mid+7,MID_LEN);
    
    //tid
    memcpy(pData.tid,tid,TID_LEN);
    
    //date time
    memset(buf,0,sizeof(buf));
    MAPGET(traDatTim, buf, lblKO);
    memcpy(pData.date,buf+2,DATE_LEN);
    memcpy(pData.time,buf+8,TIME_LEN);
    
    //amount
    memset(buf,0,sizeof(buf));
    MAPGET(TRA_SISA_SALDO,buf,lblKO);
	CHECK(ret > 0, lblKO);
	trimLeft(buf,0x20);
	trimLeft(buf,'0');
	strcat(buf,"00");  
	
	fmtPad(buf,-AMOUNT_LEN_L,'0');
    memcpy(pData.amount,buf,AMOUNT_LEN_L);
    
    ret = usrInfo(infDekatkanKartu);
    ret=reaktivasi(&pData);
    if(ret==FAIL)
    {
        usrInfo(infProcessToReaderFailWait);
        ret = -100;
        goto lblEnd;
    }

	ret = valBin();
	CHECK(ret > 0, lblKO);
	    
    goto lblEnd;
    
lblKO:
    if(ret != -100) //@agmr
        ret = -1;
lblEnd:
    return ret;    
}

int pmtPrepaidVoidInquiry()
{
    int ret;
    PREPAID_T pData;
    char buf[100], dumpNum[100];

	MAPGET(logNomorKartuBrizzi,dumpNum,lblKO);
    usrInfo(infDekatkanKartu);
    
    memset(&pData,0,sizeof(pData));
    
    //date time
    memset(buf,0,sizeof(buf));
    MAPGET(traDatTim, buf, lblKO);
    memcpy(pData.date,buf+2,DATE_LEN);
    memcpy(pData.time,buf+8,TIME_LEN);
        
    ret = infoInquiry(&pData);
    if(ret == FAIL)
    {
        usrInfo(infProcessToReaderFailWait);
        ret = -100;
        goto lblEnd; 
    }

	//simpan nomor kartu
    memset(buf,0,sizeof(buf));
    bin2hex(buf,pData.cardNumber,CARD_NUMBER_LEN);
	if(strcmp(buf, dumpNum) == 0)
    	ret = mapPut(TRA_NOMOR_KARTU_BRIZZI,buf,LEN_TRA_NOMOR_KARTU_BRIZZI);  
	else{
		usrInfo(infDifferentCard);
		goto lblKO;
	}
    if(ret<0)
        goto lblKO;

 
    //simpan balance amount
    memset(buf,0,sizeof(buf));
    memcpy(buf,pData.amount,AMOUNT_LEN_L);
    mapPut(TRA_SISA_SALDO,buf,LEN_TRA_SISA_SALDO);
    
    //simpan random
    memset(buf,0,sizeof(buf));
    memcpy(buf,pData.random,KEY_HOST_LEN);
    ret=mapPut(TRA_KEY_HOST,buf,LEN_TRA_KEY_HOST);
    if(ret<0)
        goto lblKO;   
        
    dspClear();
    
    goto lblEnd;
lblKO:
    ret = -1;
lblEnd:
	return ret;    
}

int pmtPrepaidVoid()
{
    char buf[100];
    PREPAID_VOID_T pData;
    int ret;
    char tid[9],mid[16];
//    char amtStr[25];
//    int temp;

    usrInfo(infDekatkanKartu);

#ifdef TEST_PREPAID_REVERSAL
    dspClear();
    dspLS(2,"BIKIN REVERSAL ??");
	if(acceptable_key(kbdVAL_kbdANN, 1000) == kbdVAL)
	    goto lblKO;
#endif

    memset(&pData,0,sizeof(pData));
    memset(tid,0,sizeof(tid));
    memset(mid,0,sizeof(mid));
    
    ret = getPrepaidTidMid(tid,mid);
    if(ret < 0)
        goto lblKO;
    
   //mid
    memcpy(pData.mid,mid+7,MID_LEN);
    
    //tid
    memcpy(pData.tid,tid,TID_LEN);
    
    //date time
    memset(buf,0,sizeof(buf));
    MAPGET(traDatTim, buf, lblKO);
    memcpy(pData.date,buf+2,DATE_LEN);
    memcpy(pData.time,buf+8,TIME_LEN);
    
    //amount
	MAPGET(traAmt, buf, lblKO);
	strcat(buf,"00");  
	
	fmtPad(buf,-AMOUNT_LEN_L,'0');
    memcpy(pData.amount,buf,AMOUNT_LEN_L);
    
    //key host
    memset(buf,0,sizeof(buf));
    ret = mapGet(TRA_KEY_HOST,buf,LEN_TRA_KEY_HOST);
    if(ret<0)
    	goto lblEnd;
    memcpy(pData.keyHost,buf,KEY_HOST_LEN);
                
    ret = voidBrizzi(&pData);
    if(ret == FAIL)
    {
        usrInfo(infProcessToReaderFail);
        ret = -100;
        goto lblEnd;
    }

    //simpan card Number
    memset(buf,0,sizeof(buf));
    bin2hex(buf,pData.cardNumber,CARD_NUMBER_LEN);
    MAPPUTSTR(TRA_NOMOR_KARTU_BRIZZI,buf,lblKO);    
    
    //simpan balance
    memset(buf,0,sizeof(buf));
    memcpy(buf,pData.balance,AMOUNT_LEN_S);
    mapPut(TRA_SISA_SALDO,buf,LEN_TRA_SISA_SALDO);
    
    dspClear();

    goto lblEnd;
lblKO:
//    usrInfo(infProcessToReaderFail);
    ret = -1;
lblEnd:
	return ret;   
}
//-- @agmr - brizzi2


int getPrepaidTidMid(char* tid, char* mid)
{
	int ret;
    int i;
    byte idx;
//    byte id;
    char buf1[9],buf2[16];
    char name[lenName+1];
    
    //simpan index acq yang sekarang
    MAPGETBYTE(traAcqIdx,idx,lblKO);
    
    for(i=0;i<dimAcq;i++)
    {
        mapMove(acqBeg,i);
        memset(name,0,sizeof(name));
		MAPGET(acqName, name, lblKO);
		
		if(name[0] == 0)
		    continue;
		    
		if(memcmp(name,"BRIZZI",6)==0)
		{
			memset(buf1,0,sizeof(buf1));
            MAPGET(acqTID,buf1,lblKO);
			memset(buf2,0,sizeof(buf2));
            MAPGET(acqMID,buf2,lblKO);

			memcpy(tid,buf1,8);
			memcpy(mid,buf2,15);
            
            MAPPUTBYTE(traAcqIdx,idx,lblKO);
            mapMove(acqBeg, (word) idx);
		    return i;
		}
	}
	return -1;
lblKO:
    return -1;    
}

int selectPrepaidInit(char* minAmt, char* maxAmt)
{
    byte Exp;
    int ret;
    
    dspClear();
    dspLS(0,"Min Amount:  Rp.");
    MAPGETBYTE(appExp, Exp, lblKO);
    ret = mapGet(appMinPrepaid,minAmt,lenMinPrepaid);
    CHECK(ret >= 0, lblKO);
    ret = mapGet(appMaxPrepaid,maxAmt,lenMaxPrepaid);
    CHECK(ret >= 0, lblKO);
    ret = enterAmt(1, minAmt, Exp);
    CHECK(ret >= 0, lblKO);

    if(ret == kbdANN || ret == kbdF1 || ret == 0)
    {
        return 0;
    }
    
    dspLS(2,"Max Amount:  Rp.");

    ret = enterAmt(3, maxAmt, Exp);
    CHECK(ret >= 0, lblKO);

    if(ret == kbdANN || ret == kbdF1 || ret == 0)
    {
        return 0;
    }    
    
    MAPPUTSTR(appMinPrepaid, minAmt, lblKO);
    MAPPUTSTR(appMaxPrepaid, maxAmt, lblKO);
    goto lblEnd;

lblKO:
	return -1;
lblEnd:
    return 1;
}

int KonfirmasiPrepaidRedeem()
{
    int ret;
    char buf[31];
    char str[50];
    char msg[6][22];
    int i=0;
    char amount[25];
    char judul[30];

    dspClear();

    memset(msg,0,sizeof(msg));
    
    strcpy(judul,"REDEEM");

   //No Kartu
    memset(buf,0,sizeof(buf));
    mapGet(TRA_NOMOR_KARTU_BRIZZI,buf,LEN_TRA_NOMOR_KARTU_BRIZZI);
//    strcpy(str,buf);
    trimRight(buf,0x20);
	strcpy(str,"No Kartu :");
	strcpy(msg[i++],str); 
	sprintf(str,"%21s",buf);   
    strcpy(msg[i++],str);

    //saldo kartu
	memset(amount,0,sizeof(amount));
	mapGet(TRA_SISA_SALDO,amount,sizeof(amount));
//	amount[AMOUNT_LEN_S]=0;
    memset(buf,0,sizeof(buf));
	strcpy(buf,"Rp. ");
	fmtAmt(buf+4, amount, 0, ",.");
	strcpy(str,"Saldo Kartu :");
	strcpy(msg[i++],str);
	sprintf(str,"%21s",buf);   
	strcpy(msg[i++],str);

    //saldo Deposit
	memset(amount,0,sizeof(amount));
	mapGet(TRA_SALDO_DEPOSIT,amount,sizeof(amount));
	amount[AMOUNT_LEN_S]=0;
    memset(buf,0,sizeof(buf));
	strcpy(buf,"Rp. ");
	fmtAmt(buf+4, amount, 0, ",.");
	strcpy(str,"Saldo Deposit :");
	strcpy(msg[i++],str);
	sprintf(str,"%21s",buf);
	strcpy(msg[i++],str);
    
    ret = scrollDisplay8(msg,i,6,judul,"BATAL      LANJUT");
    if(ret == kbdVAL || ret == kbdF4)
    {
    	ret = 1;
    	goto lblEnd;
    }	
    else
    {
        mapPut(traRspCod, "\x00\x00", 2);
    	goto lblBatal;
    }	
    
lblBatal:
    ret = 0;
    goto lblEnd;     
lblEnd:
    kbdStop();
    return ret;   	    
}

//++ @agmr - brizzi2
int KonfirmasiPrepaidReaktivasi()
{
    int ret;
    char buf[31];
    char str[50];
    char msg[10][22];
    int i=0;
    char amount[25];
    char judul[30];

    dspClear();

    memset(msg,0,sizeof(msg));
    
    strcpy(judul,"REAKTIVASI");

   //lama Pasif
    memset(buf,0,sizeof(buf));
    mapGet(TRA_LAMA_PASIF,buf,sizeof(buf));
    trimRight(buf,0x20);
    strcat(buf," bulan");
	strcpy(str,"Lama Pasif :");
	strcpy(msg[i++],str); 
	sprintf(str,"%21s",buf);   
    strcpy(msg[i++],str);

    //saldo kartu
	memset(amount,0,sizeof(amount));
	mapGet(TRA_SISA_SALDO,amount,sizeof(amount));
//	amount[AMOUNT_LEN_S]=0;
    memset(buf,0,sizeof(buf));
	strcpy(buf,"Rp. ");
	fmtAmt(buf+4, amount, 0, ",.");
	strcpy(str,"Saldo Kartu :");
	strcpy(msg[i++],str);
	sprintf(str,"%21s",buf);   
	strcpy(msg[i++],str);

    //saldo Deposit
	memset(amount,0,sizeof(amount));
	mapGet(TRA_SALDO_DEPOSIT,amount,sizeof(amount));
//	amount[AMOUNT_LEN_S]=0;
    memset(buf,0,sizeof(buf));
	strcpy(buf,"Rp. ");
	fmtAmt(buf+4, amount, 0, ",.");
	strcpy(str,"Saldo Deposit :");
	strcpy(msg[i++],str);
	sprintf(str,"%21s",buf);
	strcpy(msg[i++],str);
		
    //biaya admin
	memset(amount,0,sizeof(amount));
	mapGet(TRA_BIAYA_ADMIN,amount,sizeof(amount));
//	amount[AMOUNT_LEN_S]=0;
    memset(buf,0,sizeof(buf));
	strcpy(buf,"Rp. ");
	fmtAmt(buf+4, amount, 0, ",.");
	strcpy(str,"Biaya Admin :");
	strcpy(msg[i++],str);
	sprintf(str,"%21s",buf);
	strcpy(msg[i++],str);	
	
    //status kartu setelah reaktivasi
    memset(buf,0,sizeof(buf));
    mapGet(TRA_STATUS_KARTU,buf,sizeof(buf));
    if(memcmp(buf,"aa",2) == 0)
        strcpy(buf,"ACTIVE");
    if(memcmp(buf,"cl",2) == 0)
        strcpy(buf,"CLOSED");    
    strcpy(str,"Setelah reaktivasi :");
	strcpy(msg[i++],str);
    sprintf(str,"%21s",buf);
	strcpy(msg[i++],str);
    
    ret = scrollDisplay8(msg,i,6,judul,"BATAL      LANJUT");
    if(ret == kbdVAL || ret == kbdF4)
    {
    	ret = 1;
    	goto lblEnd;
    }	
    else
    {
//        mapPut(traRspCod, "\x00\x00", 2);
    	goto lblBatal;
    }	
    
lblBatal:
    ret = 0;
    goto lblEnd;     
lblEnd:
    kbdStop();
    return ret;   	    
}

int KonfirmasiPrepaidVoid()
{
    int ret;
    
   ret = pmtPrepaidVoidInquiry();
   CHECK(ret > 0, lblKO);
   ret = 1;
   goto lblEnd;
lblKO:
    ret = 0;
    goto lblEnd;     
lblEnd:
    return ret;   	    
}
//-- @agmr - brizzi2


int isPrepaidTransaction(byte isByTransType)
{
	byte type;
	int ret;
	word mnuItem;

    if(isByTransType == 1)
    {
	    MAPGETBYTE(traTxnType,type,lblKO);
    	if(type >= trtPrepaidInfoDeposit && type <= trtPrepaidRedeem)
    		return 1;
    	else 
    		return 0;
    }
    else
    {
        MAPGETWORD(traMnuItm,mnuItem,lblKO);
        switch(mnuItem)
        {
            case mnuPrepaidInfoSaldo:
            case mnuInfoDeposit:
            case mnuPrepaidPayment:
	     case mnuPrepaidPaymentDisc:
            case mnuTopUpOnline:
            case mnuTopUpDeposit:
            case mnuAktivasiDeposit:
            case mnuPrepaidRedeem:
	     //++@agmr - brizzi2
            case mnuPrepaidReaktivasi:
            case mnuPrepaidVoid:
	     //--@agmr - brizzi2   
            case mnuPrepaidInfoKartu:
            case mnuPrepaidSettlement:
                return 1;
            default:
                return 0;
        }
    }
    
lblKO:
	return -1;    
}

void prepaidReprint(int isLast)
{
    if(isLast == 1)
    {
        logDuplicata(BRIZZI_LOG);
    }
    else
    {
        logPrintTxn(BRIZZI_LOG);
    }
}

void prepaidDateSummary()
{
    int ret;
    char buf[20], buf1[15];
    card dd, mm, yy;
    
    dspClear();
    dspLS(0,"DDMMYYYY");
    
    dspLS(2,"  TEKAN [0K]   ");
    dspLS(3,"UNTUK HARI INI ");
    
    while(1)
    {	
    	memset(buf,0,sizeof(buf));
    	ret = enterPhr(1, buf, 9);
    	if(ret !=kbdVAL)
    		return;
        if(strlen(buf) == 0)
            break;
    	if(strlen(buf)<8)
    		continue;

    	dec2num(&dd,buf,2);
    	dec2num(&mm,buf+2,2);
    	dec2num(&yy,buf+4,4);
    	
    	if(mm>12 || mm<=0)
    	    continue;
    	
    	if(dd>31 || dd<=0)
    	    continue;
    	
    	break;   	
    }
    
    if(buf[0] == 0)
        buf1[0] = 0;
    else
    {
        memset(buf1,0,sizeof(buf1));
        memcpy(buf1,buf+4,4);
        memcpy(buf1+4,buf+2,2);
        memcpy(buf1+6,buf,2);
    }
	printPrepaidSummary(buf1);
}

void prepaidAllSummary()
{
    printPrepaidSummary(NULL); 
}

void prepaidDateDetail(byte isPaymentOnly)
{    
    int ret;
    char buf[20], buf1[15];
    card dd, mm, yy;
    
    dspClear();
    dspLS(0,"DDMMYYYY");
    
    dspLS(2,"  TEKAN [0K]   ");
    dspLS(3,"UNTUK HARI INI ");
    
    while(1)
    {	
    	memset(buf,0,sizeof(buf));
    	ret = enterPhr(1, buf, 9);
    	if(ret !=kbdVAL)
    		return;
        if(strlen(buf) == 0)
            break;    		
    	if(strlen(buf)<8)
    		continue;

    	dec2num(&dd,buf,2);
    	dec2num(&mm,buf+2,2);
    	dec2num(&yy,buf+4,4);
    	
    	if(mm>12 || mm<=0)
    	    continue;
    	
    	if(dd>31 || dd<=0)
    	    continue;
    	
    	break;   	
    }
    
    if(buf[0] == 0)
        buf1[0] = 0;
    else
    {
        memset(buf1,0,sizeof(buf1));
        memcpy(buf1,buf+4,4);
        memcpy(buf1+4,buf+2,2);
        memcpy(buf1+6,buf,2);
    }
    
	prepaidPrintDetail(buf1,isPaymentOnly);    	 
}

void prepaidAllDetail(byte isPaymentOnly)
{
    prepaidPrintDetail(NULL,isPaymentOnly);
}

int prepaidPrintDetail(char* dateTime,byte isPaymentOnly)
{
	// Operation variables
	word curMnuItem, trxNum;
	int ret = 0, ctr = 0;
	PREPAID_REPORT_TRANS_T detail;
	byte tanda = 0;
	char dt[20]; //current date time
	card tmp;
	char waktu[20];
	byte AcqId=1;
		
	trcS("prepaidPrintDetail: Beg\n");
	
	memset(waktu,0,sizeof(waktu));
	memset(dt,0,sizeof(dt));
	//getDateTime(char *YYMMDDhhmmss)
	getDateTime(dt+2); 
	dec2num(&tmp,dt+2,2);
	if(tmp < 70)
	    memcpy(dt,"20",2);
	else
	    memcpy(dt,"19",2);	

    if(dateTime != NULL)
    {
        if(dateTime[0] == 0) //dateTime yang di kirim = "" -> maka diartikan tanggal sekarang
        {
           memcpy(waktu,dt,14);
        }
        else
        {
        	memcpy(waktu,dateTime,14);
        }
    }
    //get acquirer
    MAPPUTWORD(traMnuItm, mnuPrepaidPayment, lblKO);
    ret = briGetAcq(DEBIT_CARD,&AcqId);
	if (ret <= 0)
		goto lblKO;
    AcqId--;
    mapMove(acqBeg, (word) AcqId);    
    
	//Get current transaction number
	MAPGETWORD(regTrxNo, trxNum, lblKO);

	//Computation loop
	for (ctr = 0; ctr < trxNum; ctr++)
	{
	    memset(&detail,0,sizeof(detail));
		mapMove(logBeg, ctr);
		MAPGETWORD(logMnuItm, curMnuItem, lblKO);
		MAPPUTWORD(traMnuItm,curMnuItem,lblKO);
        if(isPrepaidTransaction(0)==0)
            continue;
            
        if(curMnuItem == mnuPrepaidInfoSaldo ||
           curMnuItem == mnuInfoDeposit      ||
           curMnuItem == mnuPrepaidInfoKartu ||
           curMnuItem == mnuPrepaidSettlement
           )
           continue;

        if(isPaymentOnly)
        {
            if((curMnuItem != mnuPrepaidPayment) && (curMnuItem != mnuPrepaidPaymentDisc))
                continue;
        }
        
		MAPGET(logDatTim, detail.txnDateTime,lblKO);
		MAPGET(logTraAmt,detail.amt,lblKO);
		MAPGETBYTE(logBrizziDiscFlag,detail.discFlag,lblKO);
		MAPGET(logBrizziDiscVal,detail.discValue,lblKO);
		MAPGET(logtraTotalAmount,detail.discBefAmount,lblKO);
        	MAPGET(LOG_NOMOR_KARTU_BRIZZI,detail.kartuBrizzi,lblKO);
       	MAPGETCARD(logROC,detail.roc,lblKO);
        
    	if(dateTime != NULL)
    	{
    	    if(memcmp(waktu,detail.txnDateTime,8) != 0)
    	        continue;
    	}
    	
        getBriTransTypeName(NULL, detail.transName, curMnuItem); //@agmr BRI2
		printPrepaidRecDetail(tanda,&detail);		
        
        tanda=1;
	}

    if(tanda == 0)
        goto lblNoTransaction;
    
	prtES("\x1B\x20","------------------------------------------------");
	ret = rptReceipt(rloReceiptFeed);
	CHECK(ret >= 0, lblKO);

	ret = 1;
	goto lblEnd;

lblNoTransaction:
    ret = 0;
    usrInfo(infNoTran);
    goto lblEnd;
lblKO:
    ret = -1;
	usrInfo(infProcessingError);
	goto lblEnd;

lblEnd:
	trcFN("logCalcTot: ret=%d \n", ret);
	return ret;
}

void printPrepaidRecDetail(byte tanda,PREPAID_REPORT_TRANS_T *detail)
{   
/*
    tanda = 0 -> print header + detail
    tanda = 1 -> print detail saja    
*/
	char buf[48+1];
	char buf2[48+1];
	char buf3[50];
	char buf4[50];
	int ret;
	char fmtDatTim [50];
	word curMnuItem; //@agmr - brizzi2
	
	MAPPUTSTR(rptBuf1,"",lblKO);
	if(tanda == 0)
	{
    	strcpy(buf,"DETAIL REPORT");
    	MAPPUTSTR(rptJudul,buf,lblKO);
    	ret = rptReceipt(rloPrepaidDetailHeader);
    	CHECK(ret >= 0, lblKO);	
    	
    	memset(buf,0x20,48);
    	buf[48] = 0;
//    	sprintf(buf,"CARD NUMBER %48s","AMOUNT");
    	memcpy(buf,"CARD NUMBER",11);
    	memcpy(buf+36,"TOTAL AMOUNT",12);
        prtES("\x1B\x20",buf);
        strcpy(buf,"DATE TRANS                              TRACE NO");
        prtES("\x1B\x20",buf);
	 strcpy(buf,"AMOUNT                                  DISCOUNT");
        prtES("\x1B\x20",buf);
	  strcpy(buf,"TRANS TYPE");
        prtES("\x1B\x20",buf);
    	prtES("\x1B\x20","------------------------------------------------");   
	}
	
	prtES("\x1B\x20"," ");
	
	memset(buf,0x20,48);
	buf[48] = 0;
	
	memset(buf2,0,sizeof(buf2));
	memset(buf4,0,sizeof(buf4));
	//++ @agmr - brizzi2	
	MAPGETWORD(logMnuItm, curMnuItem, lblKO);
	if(curMnuItem == mnuPrepaidVoid)
	    strcpy(buf2,"-Rp. ");
	else
//-- @agmr - brizzi2
	strcpy(buf2,"Rp. ");
	memset(buf3,0,sizeof(buf3));
	if(strcmp(detail->transName, "PEMBAYARAN DISC") != 0)
		memcpy(buf3,detail->amt,lenAmt);
	else
		memcpy(buf3,detail->discBefAmount,lenAmt);
	//	fmtAmt(buf2+4, buf3, 0, ",.");//-- @agmr - brizzi2	
    fmtAmt(buf2+strlen(buf2), buf3, 0, ",.");//-- @agmr - brizzi2	

	sprintf(buf,"%48s",buf2);
	
    memset(buf3,0,sizeof(buf3));
    memcpy(buf3,detail->kartuBrizzi,NOMOR_KARTU_BRIZZI_LEN);
    memcpy(buf,buf3,strlen(buf3));
    prtES("\x1B\x20",buf);
    
    memset(buf2,0,sizeof(buf2));
    memcpy(buf2,detail->txnDateTime,lenDatTim);
    memset(fmtDatTim,0,sizeof(fmtDatTim));
	strFormatDatTimBRI2(buf2, fmtDatTim);
	
	memset(buf3,0,sizeof(buf3));
	sprintf(buf3,"%06lu",detail->roc);
//	num2bin(buf3, detail->roc, 6);
	memset(buf,0x20,sizeof(buf));
	buf[47]=0;
	memcpy(&buf[42],buf3,6);
	memcpy(buf,fmtDatTim,strlen(fmtDatTim));
    prtES("\x1B\x20",buf);

	if(strcmp(detail->transName, "PEMBAYARAN DISC") == 0){
		memset(buf,0,sizeof(buf));
		memset(buf2,0,sizeof(buf2));
		memset(buf3,0,sizeof(buf3));
		strcpy(buf2,"Rp. ");
		if(detail->discFlag == 1){
			sprintf(buf3,"%d",atoi(detail->discValue));
			fmtAmt(buf2+strlen(buf2), buf3, 0, ",.");
			sprintf(buf,"%48s",buf2);
		}else{
			sprintf(buf3,"%d %%",atoi(detail->discValue));
			sprintf(buf,"%48s",buf3);	
		}
		
		memset(buf2,0,sizeof(buf2));
		memset(buf3,0,sizeof(buf3));
		strcpy(buf2,"Rp. ");
    		memcpy(buf3,detail->amt,lenAmt);
		fmtAmt(buf2+strlen(buf2), buf3, 0, ",.");
    		memcpy(buf,buf2,strlen(buf2));
		
    		prtES("\x1B\x20",buf);	
	}
	else{
	}
    prtES("\x1B\x20",detail->transName);

lblKO:
	return;
}

//++ @agmr - brizzi2 - ganti seluruh fungsi
int printPrepaidSummary(char* dateTime)
{    
    TOTAL_T total[tTotalEnd];
    int ret;
    int i;
    char buf[50], buf2[30], buf3[30], buf4[30];
    char dt[20]; //current date time
    card tmp;
    char waktu[20];
    char *ptr;
    char totalAllAmount[20];
    byte AcqId;
    char strAmt[30]; //@agmr - brizzi2
    //word mnuItem; //@agmr - brizzi2
    
	memset(waktu,0,sizeof(waktu));
    memset(total,0,sizeof(total));
	memset(dt,0,sizeof(dt));
	
	getDateTime(dt+2); 
	dec2num(&tmp,dt+2,2);
	if(tmp < 70)
	    memcpy(dt,"20",2);
	else
	    memcpy(dt,"19",2);	
    	
    for(i=0;i<tTotalEnd;i++) //perbaikan untuk v2.05
    {
        memset((total[i]).totAmt,'0',lenTotAmt);
    }
    
    if(*dateTime == 0) //dateTime yang di kirim = "" -> maka diartikan tanggal sekarang
    {
       memcpy(waktu,dt,14);
    }
    else if(dateTime != NULL)
    {
    	memcpy(waktu,dateTime,14);
    }

    if(dateTime == NULL)
    	ptr = NULL;
    else
    	ptr = waktu;

    //get acquirer
    MAPPUTWORD(traMnuItm, mnuPrepaidPayment, lblKO);
    ret = briGetAcq(DEBIT_CARD,&AcqId);
	if (ret <= 0)
		goto lblKO;
    AcqId--;
    mapMove(acqBeg, (word) AcqId);    
    
    ret = briLogCalcTotal(BRIZZI_LOG, total, ptr);
    if(ret == -1)
        goto lblKO;
	
    if(ret == 0)
    {
        ret = 0;
        goto lblEnd;
    }
    
	memset(buf,0,sizeof(buf));
	
	MAPPUTSTR(rptBuf1,"",lblKO);
	if(dateTime != NULL)
	{
    	sprintf(buf,"REPORT DATE: %c%c/%c%c/%c%c%c%c     %c%c:%c%c:%c%c",
    	            dt[6],dt[7],dt[4],dt[5],dt[0],dt[1],dt[2],dt[3],dt[8],dt[9],dt[10],dt[11],dt[12],dt[13]);
    	MAPPUTSTR(rptBuf1,buf,lblKO);
    }

	strcpy(buf,"SUMMARY REPORT");
	MAPPUTSTR(rptJudul,buf,lblKO);
	ret = rptReceipt(rloPrepaidSummaryReport);
	CHECK(ret >= 0, lblKO);

    strcpy(totalAllAmount,"0");
    tmp = 0;
	prtES("\x1B\x20","------------------------------------------------");
	prtES("\x1B\x20","TRANSACTION          COUNT                 TOTAL");
	prtES("\x1B\x20","------------------------------------------------");
	for(i=0;i<tTotalEnd;i++) //perbaikan untuk v2.05
	{
	    if(total[i].count <= 0)
	        continue;
	    memset(buf3,0, sizeof(buf3));
	    memset(buf3,' ', 20);
	    
	    //trans name
	    memcpy(buf3,total[i].transName, strlen(total[i].transName));
	    
	    //total amount
	    memset(buf2,0,sizeof(buf2));
        memset(strAmt,0,sizeof(strAmt));
        if(i == PREPAID_VOID)
        {
            strcpy(buf2,"-Rp. ");
            strAmt[0] = '-';
        }
        else
        { 
            strcpy(buf2,"Rp. ");
        }
            
        fmtAmt(buf2+strlen(buf2), total[i].totAmt, 0, ",.");
	    strcat(strAmt,total[i].totAmt);
	    addStr(totalAllAmount,totalAllAmount,strAmt);
	    
	    memset(buf4,0, sizeof(buf4));
	    memset(buf4,' ', 21);
	    memcpy(buf4+21-strlen(buf2),buf2, strlen(buf2));
	    memset(buf,0,sizeof(buf));
        sprintf(buf,"%s %04i  %s",buf3, total[i].count, buf4);	
	    tmp+=total[i].count;
	    prtES("\x1B\x20",buf);  	    
	}
	prtES("\x1B\x20","------------------------------------------------");

	memset(buf3,0, sizeof(buf3));
	memset(buf3,' ', 20);
	memcpy(buf3,"TOTAL BRIZZI",12);
	    
	memset(buf2,0,sizeof(buf2));
	fmtAmt(buf2, totalAllAmount, 0, ",.");
    memset(buf4,0, sizeof(buf4));
    memset(buf4,' ', 21);
    memcpy(buf4+21-strlen(buf2),buf2, strlen(buf2));
    memset(buf,0,sizeof(buf));
    sprintf(buf,"%s %04i  %s",buf3,(int)tmp, buf4);
    prtES("\x1B\x20",buf);
	ret = rptReceipt(rloReceiptFeed);
	CHECK(ret >= 0, lblKO);
	
	ret = 1;
	goto lblEnd;
	
lblKO:
    ret = -1;
lblEnd:        
    return ret;
    
}
//-- @agmr - brizzi2 - ganti seluruh fungsi

/*
int printPrepaidSummary(char* dateTime)
{    
    TOTAL_T total[tTotalEnd];
    int ret;
    int i;
    char buf[50], buf2[30], buf3[30], buf4[30];
    char dt[20]; //current date time
    card tmp;
    char waktu[20];
    char *ptr;
    char totalAllAmount[20];
    byte AcqId;
    char strAmt[30]; //@agmr - brizzi2
    word mnuItem; //@agmr - brizzi2
    
    
	memset(waktu,0,sizeof(waktu));
    memset(total,0,sizeof(total));
	memset(dt,0,sizeof(dt));
	
	getDateTime(dt+2); 
	dec2num(&tmp,dt+2,2);
	if(tmp < 70)
	    memcpy(dt,"20",2);
	else
	    memcpy(dt,"19",2);	
    	
    for(i=0;i<tTotalEnd;i++) //perbaikan untuk v2.05
    {
        memset((total[i]).totAmt,'0',lenTotAmt);
    }
    
    if(*dateTime == 0) //dateTime yang di kirim = "" -> maka diartikan tanggal sekarang
    {
       memcpy(waktu,dt,14);
    }
    else if(dateTime != NULL)
    {
    	memcpy(waktu,dateTime,14);
    }

    if(dateTime == NULL)
    	ptr = NULL;
    else
    	ptr = waktu;

    //get acquirer
    MAPPUTWORD(traMnuItm, mnuPrepaidPayment, lblKO);
    ret = briGetAcq(DEBIT_CARD,&AcqId);
	if (ret <= 0)
		goto lblKO;
    AcqId--;
    mapMove(acqBeg, (word) AcqId);    
    
    ret = briLogCalcTotal(BRIZZI_LOG, total, ptr);
    if(ret == -1)
        goto lblKO;
	
    if(ret == 0)
    {
        ret = 0;
        goto lblEnd;
    }
    
	memset(buf,0,sizeof(buf));
	
	MAPPUTSTR(rptBuf1,"",lblKO);
	if(dateTime != NULL)
	{
    	sprintf(buf,"REPORT DATE: %c%c/%c%c/%c%c%c%c     %c%c:%c%c:%c%c",
    	            dt[6],dt[7],dt[4],dt[5],dt[0],dt[1],dt[2],dt[3],dt[8],dt[9],dt[10],dt[11],dt[12],dt[13]);
    	MAPPUTSTR(rptBuf1,buf,lblKO);
    }

	strcpy(buf,"SUMMARY REPORT");
	MAPPUTSTR(rptJudul,buf,lblKO);
	ret = rptReceipt(rloPrepaidSummaryReport);
	CHECK(ret >= 0, lblKO);

    strcpy(totalAllAmount,"0");
    tmp = 0;
	prtES("\x1B\x20","------------------------------------------------");
	prtES("\x1B\x20","TRANSACTION          COUNT                 TOTAL");
	prtES("\x1B\x20","------------------------------------------------");
	for(i=0;i<tTotalEnd;i++) //perbaikan untuk v2.05
	{
	    if(total[i].count <= 0)
	        continue;
	    memset(buf3,0, sizeof(buf3));
	    memset(buf3,' ', 20);
	    
	    //trans name
	    memcpy(buf3,total[i].transName, strlen(total[i].transName));
	    
	    //total amount
	    memset(buf2,0,sizeof(buf2));
		fmtAmt(buf2, total[i].totAmt, 0, ",.");
		addStr(totalAllAmount,totalAllAmount,total[i].totAmt);
		
	    memset(buf4,0, sizeof(buf4));
	    memset(buf4,' ', 17);
	    memcpy(buf4+17-strlen(buf2),buf2, strlen(buf2));
	    memset(buf,0,sizeof(buf));
	    sprintf(buf,"%s %04i  Rp. %s",buf3, total[i].count, buf4);
	    tmp+=total[i].count;
	    prtES("\x1B\x20",buf);
	}
	prtES("\x1B\x20","------------------------------------------------");

	memset(buf3,0, sizeof(buf3));
	memset(buf3,' ', 20);
	memcpy(buf3,"TOTAL BRIZZI",12);
	    
	memset(buf2,0,sizeof(buf2));
	fmtAmt(buf2, totalAllAmount, 0, ",.");
    memset(buf4,0, sizeof(buf4));
    memset(buf4,' ', 17);
    memcpy(buf4+17-strlen(buf2),buf2, strlen(buf2));
    memset(buf,0,sizeof(buf));
    sprintf(buf,"%s %04i  Rp. %s",buf3,(int)tmp, buf4);
    prtES("\x1B\x20",buf);
	ret = rptReceipt(rloReceiptFeed);
	CHECK(ret >= 0, lblKO);
	
	ret = 1;
	goto lblEnd;
	
lblKO:
    ret = -1;
lblEnd:        
    return ret;
    
}
*/

int prepaidBatchIncrement(void)
{
	int ret;
	byte currBat, newBat;
	char buf[25];

	trcS("prepaidBatchIncrement Beg\n");

	//increment batch number
	MAPGETBYTE(regPrepaidBatchNum, currBat, lblKO);
	if(currBat == 99)
	    newBat = 1;
	else
	    newBat = currBat+1;
	    
	MAPPUTBYTE(regPrepaidBatchNum,newBat,lblKO);
	
	memset(buf,0,sizeof(buf));
	MAPGET(acqName,buf,lblKO);
	if(memcmp(buf,"BRIZZI",6)==0)
	{
	    memset(buf,0,sizeof(buf));
	    sprintf(buf,"%06d",newBat);
	    MAPPUTSTR(acqCurBat,buf,lblKO);
	}
	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	ret = 0;
	goto lblEnd;
lblEnd:
	trcFN("logBatchIncrement: ret=%d \n", ret);
	return ret;
}

int prepaidSettlementReceipt()
{
    int ret;
    card count;
    char total[10];
    char buf[50];
    char amt[25];
    
    MAPPUTSTR(rptBuf1,"",lblKO);
    MAPPUTSTR(rptBuf2,"",lblKO);
    MAPPUTSTR(rptBuf3,"",lblKO);
    MAPPUTSTR(rptBuf4,"",lblKO);


    MAPPUTSTR(traCardType, "BRIZZI", lblKO);
    MAPPUTSTR(traChipSwipe, " (Fly)", lblKO);
    
    //judul
    MAPPUTSTR(rptBuf1,"SETTLEMENT REPORT",lblKO);
    
    //banyak transaksi
    memset(total,0,sizeof(total));
	MAPGETCARD(totAppSaleCnt, count, lblKO);
	sprintf(buf,"Jumlah : %d", (int)count);
	MAPPUTSTR(rptBuf2,buf,lblKO);
    
    //total transaksi
    memset(amt,0,sizeof(amt));
    memset(buf,0,sizeof(buf));
    ret = mapGet(totAppSaleAmt,buf,sizeof(buf));
    CHECK(ret >= 0,lblKO);
//	buf[strlen(buf) - 2] = 0;    
//    strcpy(amt,"Rp. ");
    fmtAmt(amt,buf,0,",.");    
    memset(buf,0,sizeof(buf));
    sprintf(buf,"Total  : Rp. %s",amt);
    MAPPUTSTR(rptBuf3,buf,lblKO);
    
    MAPPUTSTR(rptBuf4,"Settlement BERHASIL",lblKO);

    ret = rptReceipt(rloPrepaidSettlement);
    CHECK(ret>=0,lblKO);
    goto lblEnd;

lblKO:
	return -1;
lblEnd:
	return 1;
}

int doPrepaidReversal()
{
    int ret;
    
    ret = onlConnect2();
	CHECK(ret >= 0, lblKO);
	
	MAPPUTBYTE(revReversalFlag, 1, lblKO);
    ret = doBRIReversal();
    comHangStart();		
    comHangWait();            
    DisconnectFromHost();
    CHECK(ret > 0, lblKO);
    
    goto lblEnd;
lblKO:
	ret = -1;
	goto lblEnd;
lblEnd:
	return 1;    
}

int clearPrepaidReversalFlag()
{
	int ret;

    MAPPUTBYTE(revReversalFlag, 0, lblKO);
	mapReset(revBeg);
	return 1;
lblKO:
	return -1;
}

int getPrepaidNii(char* Nii)
{
    int ret;
    int idx;
    char name[50];
    char expectedName[30];
//    char Nii[lenNII + 1];
    byte currIdx=0;
    word mnuItem;
    byte tmp;

    //bukan transaksi prepaid..
    if(isPrepaidTransaction(0) == 0)
        return 1;
    
    MAPGETWORD(traMnuItm,mnuItem,lblKO);
    //prepaid settlement, NII dari AcqName = BRIZZI.
    //selain itu pakai AcqName = MINI ATM D
    if(mnuItem == mnuPrepaidSettlement
	|| mnuItem == mnuPrepaidVoid //@agmr - brizzi2
	)
        return 1;

    MAPGETBYTE(acqID,currIdx,lblKO);
    currIdx--;

    memset(expectedName,0,sizeof(expectedName));
    
    MAPGETBYTE(traKategoriKartu,tmp,lblKO);
    if(tmp == CREDIT_CARD)
        strcpy(expectedName,"MINI ATM C");
    else
        strcpy(expectedName,"MINI ATM D");
    memset(name,0,sizeof(name));

	for (idx = 0; idx < dimAcq; idx++)
	{
		ret = mapMove(acqBeg, idx);
		CHECK(ret >= 0, lblKO);
		MAPGET(acqName, name, lblKO);

		if(memcmp(expectedName, name, strlen(expectedName)) == 0)
		{
			MAPGET(acqNII, Nii, lblKO);
			ret = mapMove(acqBeg, currIdx);
			CHECK(ret >= 0, lblKO);
			return 1;
		}
	}
lblKO:
    return -1; 
}

int getPrepaidCardInfo(PREPAID_T *pData)
{
    int ret;
//    PREPAID_T pData;
    char buf[100];

    usrInfo(infDekatkanKartu);
    
//    memset(&pData,0,sizeof(pData));
    
    //date time
    memset(buf,0,sizeof(buf));
    MAPGET(traDatTim, buf, lblKO);
    memcpy(pData->date,buf+2,DATE_LEN);
    memcpy(pData->time,buf+8,TIME_LEN);
        
    ret = infoInquiry(pData);
    if(ret == FAIL)
    {
        usrInfo(infProcessToReaderFailWait);
        ret = -100;
        goto lblEnd; 
    }
        
    //simpan nomor kartu
//    memset(buf,0,sizeof(buf));
//    bin2hex(buf,pData.cardNumber,CARD_NUMBER_LEN);
//    ret = mapPut(TRA_NOMOR_KARTU_BRIZZI,buf,LEN_TRA_NOMOR_KARTU_BRIZZI);  
//    if(ret<0)
//        goto lblKO;
// 
//    dspClear();
    ret = 1;
    goto lblEnd;
lblKO:
    ret = -1;
lblEnd:
	return ret;
}

//++ @agmr - brizzi2
int getPrepaidVoidTrans()
{
	int ret;
//	word key;
	byte flag;
	word originalMenu=0;
	byte IssId,AcqId;
	char buf[30],buf1[30];

	trcS("getPrepaidVoidTrans Beg\n");

    txnAsli = 0;
	ret = scrDlg(infEnterTrace, traFindRoc);
	CHECK(ret >= 0, lblKO);
	CHECK(ret == kbdVAL, lblKO);

    ret = logLoad();
	CHECK(ret > 0, lblEnd);

    //++@agmr - kembaliin mnuItem
    MAPPUTWORD(traMnuItm,mnuPrepaidVoid,lblKO);
    //--@agmr - kembaliin mnuItem
    
//++ @agmr     
    MAPPUTBYTE(regLocType, 'T', lblKO); 
    
	MAPGETBYTE(traIssIdx, IssId, lblKO);
	MAPGETBYTE(traAcqIdx, AcqId, lblKO);
	mapMove(issBeg, (word) IssId);
	mapMove(acqBeg, (word) AcqId);
//-- @agmr      

	MAPGETBYTE(traVoidFlag, flag, lblKO);
	CHECK(flag != 1, lblAlreadyVoided);

    MAPGETWORD(logMnuItm, originalMenu,lblKO);
    if((originalMenu != mnuPrepaidPayment) && (originalMenu != mnuPrepaidPaymentDisc) )
    {
        ret = usrInfo(infNotPaymentTrans);
        ret = 0;
        goto lblKO;
    }
//	ret = pmtComputeTotAmt();
//	CHECK(ret > 0, lblKO);
  
    memset(buf,0,sizeof(buf));
    mapGet(traAmt,buf,sizeof(buf));
    fmtAmt(buf1,buf,0,",.");
    fmtPad(buf1,-16,' ');
    MAPPUTSTR(msgBuffer,buf1,lblKO);
	ret = usrInfo(infVoidTotal2);
	CHECK(ret > 0, lblKO);
	CHECK(ret == kbdVAL, lblKO);

	ret = 1;
	goto lblEnd;

	/*
lblDeclined:
    ret = -3; //@agmr - BRI
    goto lblEnd;
    */
lblAlreadyVoided:
	usrInfo(infAlreadyVoided);
	ret = 0;
	goto lblEnd;
lblKO:
	ret = 0;
	trcS("getPrepaidVoidTrans aborted\n");
lblEnd:
	trcFN("getPrepaidVoidTrans: ret=%d\n", ret);
	return ret;
}


int ubahTanggalKartuPrepaid()
{
    int ret;
    char buf[20], buf1[20];
    card dd, mm, yy;//, tm;
    
    
    dspClear();
    dspLS(0,"YYDDMM");

    memset(buf1,0,sizeof(buf1));
    while(1)
    {	
    	memset(buf,0,sizeof(buf));
    	ret = enterPhr(1, buf, 9);
    	if(ret !=kbdVAL)
    		return -1;
        if(strlen(buf) == 0)
            break;
    	if(strlen(buf)<6)
    		continue;
        
    	dec2num(&dd,buf,2);
    	dec2num(&mm,buf+2,2);
    	dec2num(&yy,buf+4,2);
    	if(mm>12 || mm<=0)
    	    continue;
    	
    	if(dd>31 || dd<=0)
    	    continue;
    	    
    	hex2bin(buf1,buf,3);
    	break;   	
    }
    
    
    ubahTanggal(buf1);
    return 0;
}

//-- @agmr - brizzi2


#ifdef DEBUG_PREPAID_DATA
void debugData(byte mode,char* title,byte* data, int len)
{
    char buf[500];
    byte res[5];
    int ret;
   
	MAPGETBYTE(appDebugDataFlag,res,lblDBA);
	if(res[0] == 0)
		return;
    
	memcpy(buf,"\x1B\x1A",2);
	sprintf(buf+2,"%s",title);
	prtS(buf);
    ShowData(data,len,mode,0,15);

lblDBA:
	return;
}
#endif //DEBUG_PREPAID_DATA

#ifdef TEST_BRI

void testPrepaidInit()
{
    PREPAID_T pData;
    
    memset(&pData,0,sizeof(pData));
    memcpy(pData.amountMin,"0000000010",AMOUNT_LEN_L);
    memcpy(pData.amountMax,"0000999999",AMOUNT_LEN_L);

    samInit(&pData);
}

void testInfoSaldo()
{
    PREPAID_T pData;
    
    memset(&pData,0,sizeof(pData));

    infoSaldo(&pData);    
}

void testPrepaidSale()
{
    PREPAID_T pData;
    int ret;
    
    memset(&pData,0,sizeof(pData));
    memcpy(pData.mid,"11100011",MID_LEN);
    memcpy(pData.tid,"00000222",TID_LEN);
    memcpy(pData.date,"110616",DATE_LEN);
    memcpy(pData.time,"154703",TIME_LEN);
    memcpy(pData.amount,"0000001000",AMOUNT_LEN_L);
    MAPPUTSTR(regPrepaidBatchNum,"03",lblKO);
    prepaidSale(&pData);
lblKO:
	return;
}
#endif //TEST_BRI

#ifdef TEST_PREPAID_SETTLEMENT
void resetPrepaidUploadedFlag()
{
	int ret;
	word trxNum;
	word i;

	MAPGETWORD(regTrxNo, trxNum, lblKO);

	for(i=0;i<trxNum;i++)
	{
		mapMove(logBeg, i);
        MAPPUTBYTE(logIsUploaded,0,lblKO);
    }
    
	return ;
lblKO:
	return;    
}
#endif //TEST_PREPAID_SETTLEMENT

#endif //PREPAID

