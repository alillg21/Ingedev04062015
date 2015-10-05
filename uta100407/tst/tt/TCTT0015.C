#include <string.h>
#include <stdio.h>
#include "pri.h"
#include "tst.h"

//Injection of  SAMA BDK (DUKPT Base Derivation Key) 0123456789ABDCDEFFEDCBA9876543210 into loc 0x12
//for Apacs 40 pinpad message

static void getKCV(byte *key,char *chk){ //calculate checksum
        byte dst[8]="";
        char buf[16+1]="";
        
        memset(chk,0,8+1);
        //memset(src,0,8);
        stdEDE(dst,(byte*)"\x00\x00\x00\x00\x00\x00\x00\x00",key);
        bin2hex((char *)buf,dst,8);
        memcpy(chk,buf,8);
}

static byte locMK= 0x12; //BDK location
static byte locTK= 0x04; //transport key location

static byte mc[16]= {0xFC,0xAC,0x1A,0x1B,0x0B,0x12,0x91,0x3D,0xC7,0x7C,0xB6,0x5C,0x1D,0xD4,0x5C,0x20};  
static byte *mk= (byte*) "\x6A\xC2\x92\xFA\xA1\x31\x5b\x4d\x85\x8A\xB3\xA3\xD7\xD5\x93\x3A"; //EF4AE1E53C52D0E3858AB3A3D7D5933A			  
//static byte *tk= (byte*) "\x11\x11\x11\x11\x11\x11\x11\x11"; //transport key


static int loadMK(void){ //load SAMA BDK into location 12
    	int ret;        
        byte crt[4];
        char chk[8+1];
    	char tmp[prtW+1];
    	char nooshin[20]="";

    	ret= cryLoadTestKey(locTK); CHECK(ret>=0,lblKO); ///<Inject the key 1111111111111111 into the location loc
      //  stdDES(mc,mk,tk); stdDES(mc+8,mk+8,tk); //calculate encrypted master key        
        ret= cryLoadDKuDK(locMK,locTK,mc,'d'); CHECK(ret>=0,lblKO); //load master key (d:DUKPT INitial Key)         
        ret= cryVerify(locMK,crt); CHECK(ret>=0,lblKO); //calculate the certificate

    	//present results
    	ret= bin2hex(tmp,&locMK,1); CHECK(ret==2,lblKO);
    	strcat(tmp,": "); VERIFY(strlen(tmp)==4);    
    	ret= bin2hex(tmp+4,crt,4); CHECK(ret==8,lblKO);        
    	ret= dspLS(0,tmp); CHECK(ret>=0,lblKO);
    	prtS(tmp);
    	
    	prtS("Verification code :");    	    	
		sprintf(nooshin,"%x",crt); 
		prtS(nooshin);
		    	

        getKCV(mk,chk);
        prtS("Master key check sum");
        prtS(chk);
        CHECK(strcmp(tmp+4,chk)==0,lblKO);
        prtS("correct");
        dspClear();        
        ret= dspLS(1,"Key loaded");
        prtS("key loaded");
        
        tmrPause(2);
        
        return 1;
lblKO:
        return -1;
}

void tctt0015(void){
    int ret;
    ret= cryStart('m');    
    ret= loadMK(); CHECK(ret>0,lblKO);    
    goto lblEnd;
lblKO:
    trcErr(ret);
    dspLS(0,"KO!");
lblEnd:
    tmrPause(3);
    cryStop('m');
}
