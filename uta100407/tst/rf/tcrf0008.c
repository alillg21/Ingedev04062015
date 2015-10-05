#include <string.h>
#include "tst.h"
#include "log.h"
#include "stdio.h"

#ifdef __TEST__
static code const char *srcFile= __FILE__;

//Integration test case tcrf0008.
//Parsing SPDH response;
//Using database;
//Testing new database fields for Full Download transaction.

/* 
	FID V related db fields:
		- traIsMoreMail
		- traDownlKey

	FID W reated new database fields:
		DID a	- appTrmLoc
		DID h	- ??
		DID f	- appSRIName, appSRIAddr, appSRICity, appSRIState, appSRICntry, appSRIPhone
		DID i	- appPinPadChar
		DID q	- appRetID
	*/ 

#define CHK CHECK(ret>=0,lblKO)


static int prepare01(tBuffer *rsp){
    int ret;
    byte fs,gs,rs,etx,lf;	
    fs= cFS; 
    rs= cRS; 
	gs= cGS;
    etx= cETX; 
	lf= cLF;

    ret= bufApp(rsp,(byte *)"9.",0); CHK; //Device Type
    ret= bufApp(rsp,(byte *)"00",0); CHK; //Transmission Number
    ret= bufApp(rsp,(byte *)"12345000        ",0); CHK; //Terminal ID
    ret= bufApp(rsp,(byte *)"      ",0); CHK; //Employee ID
    ret= bufApp(rsp,(byte *)"050504",0); CHK; //Current Date
    ret= bufApp(rsp,(byte *)"122003",0); CHK; //Current Time
    ret= bufApp(rsp,(byte *)"A",0); CHK; //Message Type
    ret= bufApp(rsp,(byte *)"O",0); CHK; //Message SubType
    ret= bufApp(rsp,(byte *)"90",0); CHK; //Transaction Code
    ret= bufApp(rsp,(byte *)"0",0); CHK; //Processing Flag 1
    ret= bufApp(rsp,(byte *)"0",0); CHK; //Processing Flag 2
    ret= bufApp(rsp,(byte *)"0",0); CHK; //Processing Flag 3
    ret= bufApp(rsp,(byte *)"881",0); CHK; //Response Code

/* 1.recieved datas
x02
9
x2E0012345000..............050413144305AO90000881
x1CV0021.0000000000
x1CW0
x1D0.50000000000509999999991-4212700...........1-4212700...........421-2200............0,999999....110110MM......
x1D1.5600000000058999999999........................................421-2200............0,999999....110110........
x03x0A
*/
    ret= bufApp(rsp,&fs,1); CHK; //FS
    ret= bufApp(rsp,(byte *)"V",0); CHK; //Mail/Download Key
    ret= bufApp(rsp,(byte *)"0021 0000000000",0); CHK;
    ret= bufApp(rsp,&fs,1); CHK; //FS
    ret= bufApp(rsp,(byte *)"W",0); CHK; //Mail/Download Text
    ret= bufApp(rsp,(byte *)"0",0); CHK;
	ret= bufApp(rsp,&gs,1); CHK; //GS
    ret= bufApp(rsp,(byte *)"0",0); CHK; // DID 0
    ret= bufApp(rsp,(byte *)" 50000000000509999999991-4212700           1-4212700           421-2200            0,999999    110110MM      ",0); CHK;
	ret= bufApp(rsp,&gs,1); CHK; //GS
	ret= bufApp(rsp,(byte *)"1",0); CHK; // DID 1
	ret= bufApp(rsp,(byte *)" 5600000000058999999999                                        421-2200            0,999999    110110        ",0); CHK;
	ret= bufApp(rsp,&etx,1); CHK; //ETX
	ret= bufApp(rsp,&lf,1); CHK; //line feed

	return bufLen(rsp);
lblKO:
    trcErr(ret);
	return -1;
}

static int prepare05(tBuffer *rsp){
    int ret;
    byte fs,gs,rs,etx,lf;	
    fs= cFS; 
    rs= cRS; 
	gs= cGS;
    etx= cETX; 
	lf= cLF;

    ret= bufApp(rsp,(byte *)"9.",0); CHK; //Device Type
    ret= bufApp(rsp,(byte *)"00",0); CHK; //Transmission Number
    ret= bufApp(rsp,(byte *)"12345000        ",0); CHK; //Terminal ID
    ret= bufApp(rsp,(byte *)"      ",0); CHK; //Employee ID
    ret= bufApp(rsp,(byte *)"050504",0); CHK; //Current Date
    ret= bufApp(rsp,(byte *)"122003",0); CHK; //Current Time
    ret= bufApp(rsp,(byte *)"A",0); CHK; //Message Type
    ret= bufApp(rsp,(byte *)"O",0); CHK; //Message SubType
    ret= bufApp(rsp,(byte *)"90",0); CHK; //Transaction Code
    ret= bufApp(rsp,(byte *)"0",0); CHK; //Processing Flag 1
    ret= bufApp(rsp,(byte *)"0",0); CHK; //Processing Flag 2
    ret= bufApp(rsp,(byte *)"0",0); CHK; //Processing Flag 3
    ret= bufApp(rsp,(byte *)"881",0); CHK; //Response Code

/* 5.recieved datas
x02
9
x2E0012345000..............050413144326AO90000881
x1CV002c.0000000000
x1CW0
x1D8.70000000000799999999991-4212700...........1-4212700...........421-2200............0,999999....110110MM......
x1D9.8000000000099999999999........................................421-2200............0,999999....110110........
x1Da.CIM.(25)
x1Db.VAROS.(13)
x1Dc.TERM.OWNER.(25)
x03Y
*/
    ret= bufApp(rsp,&fs,1); CHK; //FS
    ret= bufApp(rsp,(byte *)"V",0); CHK; //Mail/Download Key
    ret= bufApp(rsp,(byte *)"002c 0000000000",0); CHK;
    ret= bufApp(rsp,&fs,1); CHK; //FS
    ret= bufApp(rsp,(byte *)"W",0); CHK; //Mail/Download Text
    ret= bufApp(rsp,(byte *)"0",0); CHK;
	ret= bufApp(rsp,&gs,1); CHK; //GS
    ret= bufApp(rsp,(byte *)"8",0); CHK;
    ret= bufApp(rsp,(byte *)" 70000000000799999999991-4212700           1-4212700           421-2200            0,999999    110110MM      ",0); CHK;
	ret= bufApp(rsp,&gs,1); CHK; //GS
	ret= bufApp(rsp,(byte *)"9",0); CHK;
	ret= bufApp(rsp,(byte *)" 8000000000099999999999                                        421-2200            0,999999    110110        ",0); CHK;
	ret= bufApp(rsp,&gs,1); CHK; //GS
	ret= bufApp(rsp,(byte *)"a",0); CHK;
	ret= bufApp(rsp,(byte *)" CIM (25)",0); CHK;
	ret= bufApp(rsp,&gs,1); CHK; //GS
	ret= bufApp(rsp,(byte *)"b",0); CHK;
	ret= bufApp(rsp,(byte *)" VAROS (13)",0); CHK;
	ret= bufApp(rsp,&gs,1); CHK; //GS
	ret= bufApp(rsp,(byte *)"c",0); CHK;
	ret= bufApp(rsp,(byte *)" TERM OWNER (25)",0); CHK;
	ret= bufApp(rsp,&etx,1); CHK; //ETX
	ret= bufApp(rsp,&lf,1); CHK; //line feed

	return bufLen(rsp);
lblKO:
    trcErr(ret);
	return -1;
}

static int prepare06(tBuffer *rsp){
    int ret;
    byte fs,gs,rs,etx,lf;	
    fs= cFS; 
    rs= cRS; 
	gs= cGS;
    etx= cETX; 
	lf= cLF;

    ret= bufApp(rsp,(byte *)"9.",0); CHK; //Device Type
    ret= bufApp(rsp,(byte *)"00",0); CHK; //Transmission Number
    ret= bufApp(rsp,(byte *)"12345000        ",0); CHK; //Terminal ID
    ret= bufApp(rsp,(byte *)"      ",0); CHK; //Employee ID
    ret= bufApp(rsp,(byte *)"050504",0); CHK; //Current Date
    ret= bufApp(rsp,(byte *)"122003",0); CHK; //Current Time
    ret= bufApp(rsp,(byte *)"A",0); CHK; //Message Type
    ret= bufApp(rsp,(byte *)"O",0); CHK; //Message SubType
    ret= bufApp(rsp,(byte *)"90",0); CHK; //Transaction Code
    ret= bufApp(rsp,(byte *)"0",0); CHK; //Processing Flag 1
    ret= bufApp(rsp,(byte *)"0",0); CHK; //Processing Flag 2
    ret= bufApp(rsp,(byte *)"0",0); CHK; //Processing Flag 3
    ret= bufApp(rsp,(byte *)"880",0); CHK; //Response Code

/* 6.recieved datas
x02
9
x2E0012345000..............050413144332AO90000880
x1CV002r.0000000000
x1CW0
x1Df.N123456789123456789-.....12121212306....N123456789123456789-.....12121212306....URESHELY
x1Dg.BDE1CC66DB2EEB8E
x1Dh.9DB8C6A5A9DD033F
x1Di.F
x1Dp.111111111000001000000000100000
x1Dq.RET01
x1Dr.GBC.RTx2E
x03>
*/
    ret= bufApp(rsp,&fs,1); CHK; //FS
    ret= bufApp(rsp,(byte *)"V",0); CHK; //Mail/Download Key
    ret= bufApp(rsp,(byte *)"002c 0000000000",0); CHK;
    ret= bufApp(rsp,&fs,1); CHK; //FS
	ret= bufApp(rsp,(byte *)"W",0); CHK; //Mail/Download Text
    ret= bufApp(rsp,(byte *)"0",0); CHK;
	ret= bufApp(rsp,&gs,1); CHK; //GS
	ret= bufApp(rsp,(byte *)"f",0); CHK;
    ret= bufApp(rsp,(byte *)" N123456789123456789-     12121212306    N123456789123456789-     12121212306    URESHELY",0); CHK;
	ret= bufApp(rsp,&gs,1); CHK; //GS
	ret= bufApp(rsp,(byte *)"g",0); CHK;
	ret= bufApp(rsp,(byte *)" BDE1CC66DB2EEB8E",0); CHK;
	ret= bufApp(rsp,&gs,1); CHK; //GS
	ret= bufApp(rsp,(byte *)"h",0); CHK;
	ret= bufApp(rsp,(byte *)" 9DB8C6A5A9DD033F",0); CHK;
	ret= bufApp(rsp,&gs,1); CHK; //GS
	ret= bufApp(rsp,(byte *)"i",0); CHK;
	ret= bufApp(rsp,(byte *)" F",0); CHK;
	ret= bufApp(rsp,&gs,1); CHK; //GS
	ret= bufApp(rsp,(byte *)"p",0); CHK;
	ret= bufApp(rsp,(byte *)" 111111111000001000000000100000",0); CHK;
	ret= bufApp(rsp,&gs,1); CHK; //GS	
	ret= bufApp(rsp,(byte *)"q",0); CHK;
	ret= bufApp(rsp,(byte *)" RET01",0); CHK;
	ret= bufApp(rsp,&gs,1); CHK; //GS
	ret= bufApp(rsp,(byte *)"r",0); CHK;
	ret= bufApp(rsp,(byte *)" GBC RTx2E",0); CHK;
	ret= bufApp(rsp,&etx,1); CHK; //ETX
	ret= bufApp(rsp,&lf,1); CHK; //line feed

	return bufLen(rsp);
lblKO:
    trcErr(ret);
	return -1;
}

static int printRsp(void){
    int ret;
    char buf[256];
	//byte tmp[256];

	sprintf(buf, "\x1B\x1A\x1B\x17""Testing db elements\x1B\x18");
	ret= prtS(buf); CHK;
	sprintf(buf, "\x1B\x1A\x1B\x17""for Full Download\x1B\x18");
	ret= prtS(buf); CHK;	
	ret= mapGet(rptSeparator,buf,256); CHK;    
	ret= prtS(buf); CHK;
    
	ret= prtS("RspCod:"); CHK;
	ret= mapGet(traRspCod,buf,256); CHK;
    ret= prtS(buf); CHK;
	ret= mapGet(rptSpaces,buf,256); CHK;
    ret= prtS(buf); CHK;
    
	// FID V related db fields
	
	sprintf(buf, "\x1B\x1A""FID 'V' related fields");
	ret= prtS(buf); CHK;
    
	ret= prtS("=> IsMoreMail:"); CHK;
    ret= mapGet(traIsMoreMail,buf,256); CHK;
    ret= prtS(buf); CHK;
	ret= prtS("=> DownlKey:"); CHK;
    ret= mapGet(traDownlKey,buf,256); CHK;
    ret= prtS(buf); CHK;
	ret= mapGet(rptSpaces,buf,256); CHK;
    ret= prtS(buf); CHK;
        
	// FID W reated new database fields
	
	sprintf(buf, "\x1B\x1A""FID 'W' new fields");
	ret= prtS(buf); CHK;
	
	ret= prtS("=> TrmLoc:"); CHK;
    ret= mapGet(appTrmLoc,buf,256); CHK;
    ret= prtS(buf); CHK;
	ret= prtS("=> SRIName:"); CHK;
    ret= mapGet(appSRIName,buf,256); CHK;
    ret= prtS(buf); CHK;
	ret= prtS("=> SRIAddr:"); CHK;
    ret= mapGet(appSRIAddr,buf,256); CHK;
    ret= prtS(buf); CHK;
	ret= prtS("=> SRICity:"); CHK;
    ret= mapGet(appSRICity,buf,256); CHK;
    ret= prtS(buf); CHK;
	ret= prtS("=> SRIState:"); CHK;
    ret= mapGet(appSRIState,buf,256); CHK;
    ret= prtS(buf); CHK;
	ret= prtS("=> SRICntry:"); CHK;
    ret= mapGet(appSRICntry,buf,256); CHK;
    ret= prtS(buf); CHK;
	ret= prtS("=> SRIPhone:"); CHK;
    ret= mapGet(appSRIPhone,buf,256); CHK;
    ret= prtS(buf); CHK;
	ret= prtS("=> PinPadChar:"); CHK;
    ret= mapGet(appPinPadChar,buf,256); CHK;
    ret= prtS(buf); CHK;
	ret= prtS("=> RetID:"); CHK;
    ret= mapGet(appRetID,buf,256); CHK;
    ret= prtS(buf); CHK;

	ret= mapGet(rptSeparator,buf,256); CHK;    
	ret= prtS(buf); CHK;
    
    return 1;
lblKO:
    trcErr(ret);
	return -1;
}

static int waitforKeypress(void) {
	char key;
	int ret ;

	ret= dspLS(2,"press a key..."); CHECK(ret>=0,lblKO);
	ret= tmrStart(0,20*100);
	ret= kbdStart(1); CHECK(ret>=0,lblKO);
	while(tmrGet(0)){
		key= kbdKey(); //retrieve a key if pressed
		CHECK(key!='\xFF',lblKO); //error during key waiting
		if(key) break; //quit the loop if a key pressed
	}
	ret= kbdStop(); CHECK(ret>=0,lblKO);
	goto lblEnd;
lblKO:
    trcErr(ret);
    dspLS(0,"KO!");
	return -1;
lblEnd:
	ret= dspClear(); CHECK(ret>=0,lblKO);
    return 1;
}

void tcrf0008(void){
    int ret;
    byte dRsp[2048];
    tBuffer bRsp;
	
	ret= dspClear(); CHECK(ret>=0,lblKO);
    
	dspLS(0,"  MEMORY RESET  ");    
	dftReset(); //fill database by default values
    mapInit(); //re-initialise map tables
    traReset();	// clear transaction datas from db (nvm)
	dspLS(1,"      DONE     ");
    ret= dspClear(); CHECK(ret>=0,lblKO);
    
	ret= dspLS(0,"prepare 1of6..."); CHECK(ret>=0,lblKO);
    bufInit(&bRsp,dRsp,512);
	ret= prepare01(&bRsp); CHECK(ret>=0,lblKO);
    ret= dspLS(1,"parse01..."); CHECK(ret>=0,lblKO);
    ret= rspAut(bufPtr(&bRsp),bufLen(&bRsp)); CHECK(ret>=0,lblKO);
    ret= printRsp(); CHECK(ret>=0,lblKO);
    
	waitforKeypress();

	ret= dspLS(0,"prepare 5of6..."); CHECK(ret>=0,lblKO);
    bufInit(&bRsp,dRsp,512);
	ret= prepare05(&bRsp); CHECK(ret>=0,lblKO);
    ret= dspLS(1,"parse05..."); CHECK(ret>=0,lblKO);
    ret= rspAut(bufPtr(&bRsp),bufLen(&bRsp)); CHECK(ret>=0,lblKO);
    ret= printRsp(); CHECK(ret>=0,lblKO);
    
	waitforKeypress();

	ret= dspLS(0,"prepare 6of6..."); CHECK(ret>=0,lblKO);
    bufInit(&bRsp,dRsp,512);
	ret= prepare06(&bRsp); CHECK(ret>=0,lblKO);
    ret= dspLS(1,"parse06..."); CHECK(ret>=0,lblKO);
    ret= rspAut(bufPtr(&bRsp),bufLen(&bRsp)); CHECK(ret>=0,lblKO);
    ret= printRsp(); CHECK(ret>=0,lblKO);
    
	goto lblEnd;

lblKO:
    trcErr(ret);
    dspLS(0,"KO!");
lblEnd:
    tmrStop(1);
    tmrPause(3);
}
#endif

