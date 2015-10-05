/* 
Test case connecting Globalsat BR-355 GPS to serial (I7910)
It is also OK for readers compatible with NMEA 0183 standard
*/



#include "pri.h"
#include <string.h>

#define CHK CHECK(ret>=0,lblKO);

static int printString(tBuffer* buf){
	char prt[prtW+1];
	char app[5];
	int idx, idx2;
	int len;
	char str[1024];

	bufGet(buf,(byte*)str);
//	if (*str==0) 
//		return 1;
//	for (idx=0;idx<bufLen(buf);idx++) str[idx]&=127;
	memset(app,0,sizeof(app));
	memset(prt,0,sizeof(prt));
	idx2=0;
	for (idx=0;idx<bufLen(buf);idx++){
		memset(app,0,sizeof(app));
		if (str[idx]>=32 && str[idx]<=126){
			app[0]=str[idx];
		}
		else {
			strcpy(app," x");
			bin2hex(app+2,(byte*)&str[idx],1);
		}
		len=strlen(app);
		if (idx2+len>prtW){
			prtS(prt);
			memset(prt,0,sizeof(prt));
			strcpy(prt,app);
			idx2=len;
		}
		else{
			strcat(prt,app);
			idx2+=len;
		}
	}
	if (idx2)prtS(prt);
	return 1;
}

static int tctsGpsTest(char* init){
	int ret;
	byte data[256];
	tBuffer gpsdata;
	char key;
	
	bufInit(&gpsdata,data,sizeof(data));
	
	ret = kbdStart(1);      //start keyboard waiting
        CHECK(ret >= 0, lblKO);
	dspClear();
	ret= utaGclStart(gclSer,init); CHK;
	ret= utaGclDial(); CHK;
	ret= utaGclDialWait(); CHK;
    ret = tmrStart(0, 30000);
    dspLS(1,"Press key to stop");
     while(tmrGet(0)) {
        key = kbdKey();     //retrieve a key if pressed
        CHECK(key != '\xFF', lblKO);    //error during key waiting
        if(key)
           break;          //quit the loop if a key pressed
		ret= utaGclRecvBuf(&gpsdata,(byte*)"\x0d\x0a",3); Receive from GPS
		CHECK(bufLen(&gpsdata)!=0,lblKO);
		printString(&gpsdata); //print received data
    }

	ret= 1;
	dspLS(1,"  TEST IS OK!");
	goto lblEnd;
lblKO:
	ret= 0;
	dspClear();
	dspLS(1,"  ERROR!");
lblEnd:
	kbdStop();
    tmrStop(0);
	utaGclStop();
	tmrPause(1);
	return ret;
}

int tcts0006(void){
	int ret;
	static char serialInit[256]= "8N14800N";
	char init[256];
	tInfo inf;
	int logic= 1;
	
	dspStart();
	dspClear();
	prtStart();
	
	memset(&inf,0,sizeof(inf));
	inf.cmd= 's';
	inf.dly= 255;
	strcpy(inf.msg[2]," <Green> = yes");
	
	dspLS(0,"Serial init str:");
	ret= enterTxt3(1,serialInit,dspW,0xff);
	CHECK(ret == kbdVAL,lblEnd);
	strcpy(inf.msg[1],"Attach the COM2");
	ret= info(&inf);
	if (ret == kbdVAL){
		strcpy(init,"COM2|");
		strcat(init,serialInit);
		logic= tctsGpsTest(init);		
	}

	goto lblEnd;
lblEnd:
	dspStop();
	prtStop();
	tmrPause(2);
	return logic;
}
