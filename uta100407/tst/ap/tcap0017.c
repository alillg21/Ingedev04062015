#include <string.h>

#include "sys.h"
#include "tst.h"
#ifdef __TEST__

//Integration test case tcap0017.
//test cry3DESDukptGetPinUpdateKey  for Dukpt get pin and update key 
//Application sp2P5.

static int InitDUKPT(void)
{
    int ret;	
	byte locMK= 0x12;
	byte keySN[10];
	// keySN="\xFF\xFF\x12\x34\x56\x78\x21\x00\x00\x01"; //KeySerialNumber 10 bytes long (KET SET ID,TRMS ID,TRANSACTION COUNTER)
	// keySN="\xFF\xFF\x12\x34\x56\x78\x21\x00\x00\x00"; //KeySerialNumber 10 bytes long (KET SET ID,TRMS ID,TRANSACTION COUNTER)	
	memcpy(keySN,"\xFF\xFF\x98\x76\x54\x32\x10\xE0\x00\x02",10);//txn counter must start from 00 02 
	ret= cry3DESDukptInit(locMK,keySN); CHECK(ret>=0,lblKo);

	return 1;    
	goto lblEnd;

lblKo: 
	prtS("Error in InitDUKPT");         
    goto lblEnd;
lblEnd:    
    return ret;
}

void tcap0017(void){
 int ret;
 byte acc[8]="";
 byte blk[8+1]=""; //pinblock calculated by cryptomodule
 char msg1[3*dspW+1]=" "; 
 char msg2[dspW+1]="PIN:";
 byte loc= 0x06; //Mac key location
 
 byte keySN[10]="";
 char tmp[prtW+1];
 byte crt[8]="";
 
   
    tctt0020();
    
    memcpy(acc, "\x00\x00\x40\x12\x34\x56\x78\x90", 8); 	
    memset(blk,0,8);
    
    ret = prtStart();
    CHECK(ret >= 0, lblKO);
    
    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "cryStart...");
    CHECK(ret >= 0, lblKO);

    ret = cryStart('m');        //open cryptomodule
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "DUKPT INIT...");
    CHECK(ret >= 0, lblKO);   
        
	ret= InitDUKPT(); CHECK(ret>=0,lblEnd);
	
	strcat(msg1,"\n");
	strcat(msg1,msg2);
	strcat(msg1,"\n");	
	strcat(msg1,"\n");	
		
	dspStop();
	
	// Ask Pin and  update keys
	crySetPpd("0124C33010600");   
    ret= cry3DESDukptGetPinUpdateKey(msg1, acc, 120, keySN, blk,loc);
	CHECK(ret>=0,lblKO);
		
	// Pinblock 
	ret= prtS("Iteration 1");	
    ret= memcmp(blk,"\x10\xA0\x1C\x8D\x02\xc6\x91\x07",8);
    if(ret==0){
    	ret= prtS("PinBlock:");
    	ret= bin2hex(tmp,blk,8);  CHECK(ret >= 0, lblKO);
    	ret= prtS(tmp);     	
    }
    else
    {
    	prtS("incorect Pin Block");
    }
    
	//print macKey CheckSum	
	ret= cryVerify(loc,crt);

	ret= memcmp(crt,"\x34\x17\xBC\xDA",4);
	if(ret==0){
		ret= prtS("MAC key check value:");
		ret= bin2hex(tmp,crt,4);CHECK(ret >= 0, lblKO);
		prtS(tmp);		
	}
	else
	{
		prtS("incorect MAC key");
	}

/********************************************************/
	//don't Ask Pin but update keys
	prtS("\n\n");
	ret= prtS("Iteration 2");
	crySetPpd("0124C33010601");   
    ret= cry3DESDukptGetPinUpdateKey(msg1, acc, 120, keySN, blk,loc);
	CHECK(ret>=0,lblKO);
		
	//no  Pinblock 
	
	//print macKey CheckSum	
	ret= cryVerify(loc,crt);
	ret= memcmp(crt,"\x87\x77\x49\x35",4);
	if(ret==0){
		ret= prtS("MAC key check value:");
		ret= bin2hex(tmp,crt,4);CHECK(ret >= 0, lblKO);
		prtS(tmp);
		prtS("corect MAC key");
	}
	else
	{
		prtS("incorect MAC key");
	}
	
	 ret = dspStart();
    CHECK(ret >= 0, lblKO);

   
		
	ret=1;
    goto lblEnd;

    lblKO: 
	prtS("Error in tcap0017");         
    goto lblEnd;
lblEnd:
	cryStop('m');               //close cryptomodule ressource
	prtStop();
	dspStop(); 
	if (ret!=1) prtS("Error!");
}

#endif
