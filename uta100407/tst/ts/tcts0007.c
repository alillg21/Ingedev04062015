#include <string.h>
#include <stdio.h>
#include "sys.h"
#include "tst.h"

const unsigned char cardData2[]={ 0x09,0x01,0x20,0x30,0x01,0x00,0x00,0x00,0x03,0xE8,0x00,0x10,0x00,
						0x10,0x29,0x71,0xFF,0xC4,0x5D,0x1A,0x05,0x0E,0xED,0xCB,0xC1,0xBE,
						0x3C,0x95,0xD1,0xD4,0xF3,0x49,0x38,0x04,0xDB,0x01,0x87,0x34,0xBE,
						0xDC,0xA2,0xDB,0xD0,0x70,0xE5,0x84,0xBD,0x6C,0xF4,0x8C,0x3D,0x8A };//Clear Tk EF CD AB 89 01 23 45 67 01 AB CD 67 23 67 45 89
unsigned char encDbKey[]={0xFC,0x11,0x56,0xE4,0x38,0x4B,0x77,0x53,0xC0,0x68,0x63,0x09,0x08,0x88,0x6C,0xC8}; //Clear Mk 0123456789ABCDEF0123456789ABCDEF
unsigned char encWkKeyECB[]={0x56,0xCC,0x09,0xE7,0xCF,0xDC,0x4C,0xEF,0x56,0xCC,0x09,0xE7,0xCF,0xDC,0x4C,0xEF}; //0123456789ABCDEF0123456789ABCDEF encrypted by 0123456789ABCDEF 


// test case for Tellium type of key injection on UC32

void tcts0007(void) {
    int ret;
	byte kcv[8];
	byte destTk,mykey,myWkKey;
	char message[48];
	byte dataout[24];
    

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);
    
    cryStart('M');
    destTk=0x40;
    mykey=0x44;
    myWkKey=0x48;

	ret = cryVerify(destTk,kcv);
	sprintf(message,"DblKeyVerif = %d",ret);
	prtS(message);
	ret= cryLoadTK(destTk,cardData2);
	sprintf(message,"LoadSkmtTk = %d",ret);
	prtS(message);
	ret = cryVerify(destTk,kcv);
	sprintf(message,"DblKeyVerif = %d",ret);
	prtS(message);
	sprintf(message,"Tk KCV = %02X%02X%02X%02X",kcv[0],kcv[1],kcv[2],kcv[3]); //should be 3804DBD7
	prtS(message);
// added code
	ret = cryLoadDKuDK(mykey,destTk,encDbKey,'g');
	sprintf(message,"LoadDblKey = %d",ret);
	prtS(message);
	memset(kcv,0,sizeof(kcv));
	ret = cryVerify(mykey,kcv);
	sprintf(message,"DblKeyVerif = %d",ret);
	prtS(message);
	sprintf(message,"Master KCV = %02X%02X%02X%02X",kcv[0],kcv[1],kcv[2],kcv[3]); // should be D5D44FF7
	prtS(message);
// added code end	
// more added code

	ret = cryLoadDKuDK(myWkKey,mykey,encWkKeyECB,'M');
	sprintf(message,"LoadDblKey(WKECB) = %d",ret);
	prtS(message);
	memset(kcv,0,sizeof(kcv));
	ret = cryVerify(myWkKey,kcv);
	sprintf(message,"DblKeyVerif = %d",ret);
	prtS(message);
	sprintf(message,"Final KCV = %02X%02X%02X%02X",kcv[0],kcv[1],kcv[2],kcv[3]); // should be D5D44FF7
	prtS(message);
	
// more added code end
	


    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
  lblEnd:
    tmrPause(3);
    prtStop();
    dspStop();                  //close resource
	cryStop('M');
}

