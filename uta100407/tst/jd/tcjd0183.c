//Unitary UTA test case tcab0183.
//Functions testing of GPRS GSM plugin:
//  pgnImeiImsi
//  pgnSgnLvl
//  pgnBatLvl

//#include <unicapt.h>
#include "sys.h"
#include "tc.h"

/****
#include "gmaDefines.h"

#include "LogoFplus.c"

#define NVMLOGO 128

static void init(void)
{
	nvmStart();
	//save the logo file
	(void) nvmRemove(NVMLOGO);
	nvmSave(NVMLOGO, BMP_LogoFplus, 0, sizeof(BMP_LogoFplus));
}
*****/

void tcjd0183(void)
{
	static int oldLevel = -2;
	char buf[100];
	int ret;
	byte docked;
	
//    char imei[15+1];
//    char imsi[15+1];
/****
	char imei[20];
	char imsi[20];

    byte rssi;
    byte sta;
****/
/*
//    memset(imei,0,sizeof(imei));
//    memset(imsi,0,sizeof(imsi));

	memset(imei,'*',sizeof(imei));
	memset(imsi,'*',sizeof(imsi));

	memset(buf,0,sizeof(buf));
    
    ret= pgnImeiImsi(imei,imsi);
    //CHECK(ret>=0,lblKO);
    
    if(ret>=0){
    	ret= tcDspAN(imei, sizeof(imei));
		ret= tcDspAN(imsi, sizeof(imsi));
		
//    	ret= dspLS(0,imei); CHECK(ret>=0,lblKO);
//    	ret= dspLS(1,imsi); CHECK(ret>=0,lblKO);
    }else{
    	ret= tcDspS("pgnImeiImsi KO"); CHECK(ret>=0,lblKO);
    }
    tmrPause(1);
    
    rssi= 0;
    sta= 0;
    ret= pgnSgnLvl(&rssi,&sta); 
    //CHECK(ret>=0,lblKO);
    if(ret>=0){
	    num2dec(buf,rssi,3);
	    if(sta) strcat(buf," connected");
	    else strcat(buf," NO GPRS");
    }else{
    	strcpy(buf,"pgnSgnLvl KO");
    }
    ret= tcDspS(buf); CHECK(ret>=0,lblKO);
    tmrPause(1);
*/

	tcDspTitle(tcStrParam(), 0);

//	if (tcLoop() == 1)
//		init();

	ret = pgnBatLvl(&docked);

	tcDspFN("RETURNED BAT.LEVEL = %d", ret);

	if (ret != oldLevel)
	{
		sprintf(buf, "(LOOP = %d) LEVEL = %d AT :", tcLoop(), ret);
		tcPrtDateTime(buf);
		oldLevel = ret;
	}
	else if (tcLoop() % 10 == 0)
	{
		tcPrtDateTime("$$$$$$$$$$$$$$$$$$$$$$$$");
		tcPrtDateTime("***********************");
		sprintf(buf, "(LOOP = %d) AT :", tcLoop());
		tcPrtDateTime(buf);
		tcPrtDateTime("***********************");
		tcPrtDateTime("$$$$$$$$$$$$$$$$$$$$$$$");
	}

	tmrPause(3);
/*****
	tmrPause(1);
	infoScreen(0); // Graphic display
	ret = pgnBatLvl(&docked);
	tmrPause(1);
	infoScreen(1); // Text display
	ret = pgnBatLvl(&docked);
	tmrPause(1);
	infoScreen(0);
	ret = pgnBatLvl(&docked);
	tmrPause(1);
	infoScreen(1);
    //CHECK(ret>=0,lblKO);
*****/
}

