//
// unitary test case tctd0007
//
// author:      Tamas Dani
// date:        01.09.2005
// description: test for communication
//

#include <string.h>
#include "miltypes.h"
#include "comcst.h"
#include "log.h"

extern byte calculateLRC(byte *buffer, word len);

static const char *srcFile= __FILE__;
#define CHK CHECK(ret>=0,lblKO)
#define nextLRCByte(currLRC, nextByte) ((byte)(currLRC ^ nextByte))

//#define TEST_CHANEL chnCom1
#define TEST_CHANEL chnUSB

void tctd0007(void){
	int ret;
	char msgSend[128]= "hello\x03"; // ETX at the end
	char msgRecv[256]= "";
	tBuffer buf;
	byte dat[128];

	ret= dspStart();              CHK;
	ret= dspLS(0,"comStart...");  CHK; 
	ret= comStart(TEST_CHANEL);   CHK;

	while(1){
		int key= 0;
		dspClear();
		dspLS(0,"press a key:");
		dspLS(1,"1 - SEND    ");
		dspLS(2,"2 - RECEIVE ");
		dspLS(3,"3 - EXIT    ");

		tmrStart(0,30*100); //30 seconds timeout
		kbdStart(1);

		while(tmrGet(0)){
			key= kbdKey();
			if(key) break;
		}
		kbdStop();
		if(tmrGet(0)==0) continue;

		tmrStop(0);
		dspClear();

		switch(key){
		case '1':
			dspLS(0,"comSendBuf");
			ret= comSendBuf((byte *)msgSend,strlen(msgSend));

			if (ret==strlen(msgSend)) dspLS(1,"OK");
			else dspLS(1,"FAILED");

			break;
		case '2':
			dspLS(0,"comRecvBuf");
			bufInit(&buf,dat,sizeof(dat));
			ret= comRecvBuf(&buf,(byte *)"\x03",10);
			strcpy(msgRecv,(char *)bufPtr(&buf));
			dspLS(2,"received:");
			dspLS(3,msgRecv);

			if (ret==6) dspLS(1,"OK");
			else dspLS(1,"FAILED");

			break;
		default :
			dspClear();
			goto lblEnd;
			break;
		}
		tmrPause(4);
	}

lblKO:
	trcErr(ret);
	dspLS(1,"KO!");
	tmrPause(3);
lblEnd:
	tmrPause(2);
	dspLS(2,"calling comStop()");
	tmrPause(2);
	comStop();
	dspStop();
}
