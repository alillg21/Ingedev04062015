#include <unicapt.h>
#include "sys.h"
#include "tst.h"

#define L_BUF 1024
static card trans_amount;

//tags to print
static byte tag9F9109[2]; //card type
static byte tag9A[3]; //date
static byte tag9F21[3]; //time
static byte tag9F12[16]; //app prefered name
static word len9F12;
static byte tag50[16]; //app label
static word len50;
static byte tag4F[16]; //AID
static word len4F;
static byte tag5A[10]; //PAN
static byte tag9F02[6]; //txn amount
static byte tag9F8101[2]; //txn result
static byte tag95[5]; //TVR
static byte tag9B[2]; //TSI
static byte tag9F8102[6]; //last apdu cmd response

static word buildPrelimBuffer (byte *buf) {
	char dateTime[12+1];
    byte prelimData[93] = {0x9A,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x08,0x07,
                            0x14,0x00,0x21,0x9F,0x00,0x00,0x03,0x00,0x00,0x00,
                            0x11,0x35,0x37,0x00,0x9C,0x00,0x00,0x00,0x01,0x00,
                            0x00,0x00,0x00,0x00,0x00,0x00,0x41,0x9F,0x00,0x00,
                            0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x81,0x00,
                            0x00,0x00,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
                            0x2A,0x5F,0x00,0x00,0x02,0x00,0x00,0x00,0x09,0x78,
                            0x00,0x00,0x36,0x5F,0x00,0x00,0x01,0x00,0x00,0x00,
                            0x02,0x00,0x00,0x00,0x14,0x91,0x9F,0x00,0x01,0x00,
                            0x00,0x00,0x0A};
                             
	memcpy ((void *)buf, (void *)prelimData, sizeof (prelimData));

	//Push the amount, date and time into the data                                                 
	                                                 
	// amount                                          
	buf[56]=trans_amount>>24;             
	buf[57]=trans_amount>>16;             
	buf[58]=trans_amount>>8;              
	buf[59]=trans_amount;                 

	getDateTime	(dateTime);																												 
	//date                                                                            
	hex2bin(&buf[8], &dateTime[0], 1);
	hex2bin(&buf[9], &dateTime[2], 1);
	hex2bin(&buf[10], &dateTime[4], 1);
	                                                 
	//time                                             
	hex2bin(&buf[20], &dateTime[6], 1);
	hex2bin(&buf[21], &dateTime[8], 1);
	hex2bin(&buf[22], &dateTime[10], 1);     

	return sizeof(prelimData);
}

static word buildCompleteBuffer (byte *buf)
{
    byte completeData[54] = {0x16,0x84,0x9F,0x00,0x04,0x00,0x00,0x00,0x00,0x00,
                              0x00,0x00,0x17,0x84,0x9F,0x00,0x02,0x00,0x00,0x00,
                              0x09,0x78,0x00,0x00,0x1B,0x84,0x9F,0x00,0x02,0x00,
                              0x00,0x00};
                              
	memcpy ((void *)buf, (void *)completeData, sizeof (completeData));
	return sizeof(completeData);
}

static word buildGetDataBuffer (byte *buf)
{
    uint8 index1List[64] = {0x10, 0x90, 0xBF, 0x00, 0x38, 0x00, 0x00, 0x00,
                            0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x20, 0x5F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x9B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x9A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x21, 0x9F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x02, 0x9F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x4F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    
    memcpy ((void *)buf, (void *)index1List, sizeof (index1List));
	return sizeof(index1List);
}

static word buildOnlineResultBuffer (uint8 *buf)
{
    uint8 OnlineData[54] = {0x01, 0x81, 0x9F, 0x00, 0x02, 0x00, 0x00, 0x00,
                            0x00, 0x01};
                              
	memcpy ((void *)buf, (void *)OnlineData, sizeof (OnlineData));
	return sizeof(OnlineData);
}

void parseRsp(tQueue *qRsp) {
	int ret;
	word len;	
	card tag;			
	byte buf[256];	
	
	while(queLen(qRsp)) {
	    memset(buf, 0, 256);

	    ret = queGetTlv(qRsp, &tag, &len, buf);
	    VERIFY(ret == len);
	    VERIFY(len <= 256);

	    switch (tag) {
			case 0x9F9109:
				memcpy(tag9F9109, buf, (byte) ret);
				break;
			case 0x009A:
				memcpy(tag9A, buf, (byte) ret);
				break;
			case 0x9F21:
				memcpy(tag9F21, buf, (byte) ret);
				break;
			case 0x9F12:
				memcpy(tag9F12, buf, (byte) ret);
				len9F12 = ret;
				break;
			case 0x0050:
				memcpy(tag50, buf, (byte) ret);
				len50 = ret;
				break;
			case 0x004F:
				memcpy(tag4F, buf, (byte) ret);
				len4F = ret;
				break;
			case 0x005A:
				memcpy(tag5A, buf, (byte) ret);
				break;
			case 0x9F02:
				memcpy(tag9F02, buf, (byte) ret);
				break;
			case 0x9F8101:
				memcpy( tag9F8101, buf, (byte) ret);
				break;
			case 0x0095:
				memcpy(tag95, buf, (byte)ret);
				break;
			case 0x009B:
				memcpy(tag9B, buf, (byte)ret);
				break;				
			case 0x9F8102:
				memcpy(tag9F8102, buf, (byte) ret);
				break;						      		
	       default:
	           break;
	    }
	}
}

void resetTags(void) {
	memset(tag9F9109, 0, sizeof(tag9F9109));
	memset(tag9A, 0, sizeof(tag9A));
	memset(tag9F21, 0, sizeof(tag9F21));
	memset(tag9F12, 0, sizeof(tag9F12));
	memset(tag50, 0, sizeof(tag50));
	memset(tag4F, 0, sizeof(tag4F));
	memset(tag5A, 0, sizeof(tag5A));
	memset(tag9F02, 0, sizeof(tag9F02));
	memset(tag9F8101, 0, sizeof(tag9F8101));
	memset(tag95, 0, sizeof(tag95));
	memset(tag9B, 0, sizeof(tag9B));
	memset(tag9F8102, 0, sizeof(tag9F8102));
}

void printReceipt(void) {
	char tmp[128];
	char buf[dspW+1];
	char prnBuf[25];
	word tagData;
	
	prtS("************************");
	prtS("*     Teluim Pass+     *");
	prtS("*    CLESS MV DEMO     *");

 	tagData = ((tag9F9109[0] * 256) + tag9F9109[1]);
	switch (tagData) {			
		case 0x0002:
			prtS("*      Mag Stripe      *");
			break;
		case 0x0003:
			prtS("*        M/Chip        *");
			break;
		case 0x0004:
			prtS("*         MSD          *");
			break;
		case 0x0005:
			prtS("*        qVSDC         *");
			break;
		default:
			prtS("*     Unknown Card     *");
			break;				
	}

	sprintf(prnBuf, "* %02x/%02x/%02x    %02x:%02x:%02x *", 
		tag9A[2], tag9A[1], tag9A[0], tag9F21[0], tag9F21[1], tag9F21[2]);
	prtS(prnBuf);

	prtS("************************");
	
	if (tag9F12[0] != 0) {
		 prtS((char *)tag9F12);
	}
	else {
		prtS((char *)tag50);
	}

	memset(tmp, 0, sizeof(tmp));
	bin2hex(tmp, tag4F, len4F);
	prtS(tmp);

	memset(tmp, 0, sizeof(tmp));
	bin2hex(tmp, tag5A, sizeof(tag5A));
	sprintf(prnBuf, "PAN: %s", tmp);
	prtS(prnBuf);

	memset(tmp, 0, sizeof(tmp));
	memset(buf, 0, sizeof(buf));
	bin2hex(tmp, tag9F02, sizeof(tag9F02));
	fmtAmt(buf, tmp, 2, 0);
	sprintf(prnBuf, "Amount: EUR %s", buf);
	prtS(prnBuf);
	
	tagData = ((tag9F8101[0] * 256) + tag9F8101[1]);
	switch (tagData) {			
		case 0x0001:
			prtES("\x1B\x17\x1B\x1A", "APPROVED");
			break;
		case 0x0002:
			prtES("\x1B\x17\x1B\x1A", "DECLINED");
			break;
		default:
			prtES("\x1B\x17\x1B\x1A", "BAD RESULT");
			break;				
	}

	memset(tmp, 0, sizeof(tmp));
	bin2hex(tmp, tag95, sizeof(tag95));
	sprintf(prnBuf, "TVR: %s", tmp);
	prtS(prnBuf);

	memset(tmp, 0, sizeof(tmp));
	bin2hex(tmp, tag9B, sizeof(tag9B));
	sprintf(prnBuf, "TSI: %s", tmp);
	prtS(prnBuf);	

	switch (tag9F8102[2] & 0xF0) {
		case 0x00:
			prtS("Cryptogram: AAC");
		  	break;
		  
		case 0x40:
			prtS("Cryptogram: TC");
			break;
		  
		case 0x80:
			prtS("Cryptogram: ARQC");
			break;
		  
		default:    /* unknown */
			prtS("Cryptogram: Unknown");
			break;
	}
}

int tcmp0018(byte * dRsp) {
	#ifdef __CLSMV__
	int ret;
	int state = IDLE;
	char amt[12 + 1];
	char traAmt[dspW + 1];
	byte cmd[256]; 
	tBuffer bRsp;
	tQueue qRsp;	
	word cmdLen;

	//dspStart();
	//prtStart();
	ret = clsStart();
	#ifdef _ING_SIMULATION
	tmrPause(4); //wait for cls reader to start properly
	#endif

	//Initialise CLESS MV
	while (state != FINISHED) {
		bufInit(&bRsp, dRsp, L_BUF);
    	queInit(&qRsp, &bRsp);
			
		ret = clsMVTransaction(state, cmd, cmdLen, &qRsp);
		state = ret;		
	}

	//Amount Entry
	trans_amount = 0;
    dspClear();
    ret = dspLS(0, "enterAmt");
    CHECK(ret >= 0, lblKO);	
    strcpy(amt, "0");
    ret = enterAmt(1, amt, 2);		        
	if(!(amt[0]) && (ret == kbdVAL)){
		Beep();
		trans_amount = 1234; //default amount
	}        
    CHECK(ret >= 0, lblKO);	
	ret = dec2num(&trans_amount, amt, 12);
	
    dspClear();
    dspLS(0, "Total Amount");	
    fmtAmt(traAmt, amt, 2, 0);   //format 2-decimal amount
	dspLS(1, traAmt);	
	dspLS(3, "Present Card");	

	//CLESS MV Trasaction Processing
	resetTags();	
	state = IDLE;
	while (state != FINISHED) {
		memset(cmd, 0, sizeof(cmd));
		switch (state) {
			case TXN_READY:	
			case PRELIMINARY_TXN_COMPLETE:				
				cmdLen = buildPrelimBuffer(cmd);
				break;
					
			case ONLINE_PROC:
				cmdLen= buildOnlineResultBuffer (cmd);
				break;

			case COMPLETE_TXN:
				cmdLen= buildCompleteBuffer (cmd);
				break;

			case CLSCARD_REMOVED:
				cmdLen= buildGetDataBuffer(cmd);	
				break;
				
			default:
				break;
		}

		bufInit(&bRsp, dRsp, L_BUF);
		queInit(&qRsp, &bRsp);
		
		ret = clsMVTransaction(state, cmd, cmdLen, &qRsp);
		state = ret;		

		parseRsp(&qRsp);
		
	}
	printReceipt();
		
	goto lblEnd;
	
	lblKO:
		ret = -1;
	lblEnd:
		clsStop();
		//dspStop();
		//prtStop();
		tmrPause(1);
		return ret;
	#else
		return -1;
	#endif
}

