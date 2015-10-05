#include <string.h>
#include "pri.h"
#include "tst.h"

#ifdef __OWF__

// Verify  valid response Mac and updating the key register if the Reponse
// Mac matches the calculated Mac for   APACS 40 Pinpad message
//Verify ResponseMac Message coming to the pinpad
//x02S80083444FFFFFFFFFFFB37AB341x78x78x78x78180000766000113000002759x1C00000013320x1CAUTHx20COD
//Ex3D002759x1Cx1Cx1Cx1Cx03x44

static int getKCV(byte *key,char *ChkSum){
        int ret;
        
        byte dst[8+1]="";
        byte src[8+1]="";
        char buf[8+1]="";
        byte seddst[8+1]="";
        
        memset(ChkSum,0,5);             
        memcpy(src,"\x00\x00\x00\x00\x00\x00\x00\x00",8);               
        stdDES(dst,src,key);
        stdSED(seddst,dst,key+8);
        stdDES(dst,seddst,key);
        ret= bin2hex((char *)buf,dst,8); CHECK(ret>=0,lblKO);
        memcpy(ChkSum,buf,4);
        
        ret=1;
        goto lblEnd;

lblKO:
    trcErr(ret);
    dspLS(0,"KO!");
lblEnd:   
        trcS("getKCV: End\n");
        return ret;
}

void tctt0019(void){
    int ret;
    byte crt[4]="";
    char tmp[prtW+1];
	byte bChksum[5];

	byte PinKeyLoc;
	byte MacKeyLoc;
	byte WKLoc;
	char ChkSum[5];
	//byte MacKey[(32/2)+1] ="";
	byte WorkingKey[(32/2)+1] ="";
		    

	byte ReqMacResidue[8+1]="";

	byte ResMacResidue[8+1]="";
	byte ResMac[8+1]="";
	byte CResMacResidue[8+1]="";

	byte TransactionType;
    byte AuthParameterType;
    byte TxnAmountBlk[16+1]="";
	byte AuthParam[(16/2)+1]="";
	byte TxnAmount[16+1]="";
	byte ConstructRspPacket[200+1]="";



	tBuffer res;
	byte dRes[256];


	memset(tmp,0,prtW+1);
    memset(crt,0,4);


	tctt0018();

	WKLoc=0x36;
	PinKeyLoc=0x40;
	MacKeyLoc=0x42;

	//Verify Reponse MAC
 
	memcpy(ResMac,"B37AB341",8); 
	memcpy(ReqMacResidue,"\x0C\x48\xDE\x50",4);

	bufInit(&res,dRes,256);

	ret= bufApp(&res,(byte *)"180000766000113000002759",0);CHECK(ret>=0,lblKO);
	ret= bufApp(&res,(byte *)"\x1C",1); CHECK(ret>=0,lblKO);
	ret= bufApp(&res,(byte *)"00000013320",0); CHECK(ret>=0,lblKO);
	ret= bufApp(&res,(byte *)"\x1C",1); CHECK(ret>=0,lblKO);
	ret= bufApp(&res,(byte *)"AUTH",0); CHECK(ret>=0,lblKO);
	ret= bufApp(&res,(byte *)"\x20",1); CHECK(ret>=0,lblKO);
	ret= bufApp(&res,(byte *)"CODE",0); CHECK(ret>=0,lblKO); 
	ret= bufApp(&res,(byte *)"\x3D",1); CHECK(ret>=0,lblKO);
	ret= bufApp(&res,(byte *)"002759",0);CHECK(ret>=0,lblKO); 
	ret= bufApp(&res,(byte *)"\x1C",1); CHECK(ret>=0,lblKO);
	ret= bufApp(&res,(byte *)"\x1C",1); CHECK(ret>=0,lblKO);
	ret= bufApp(&res,(byte *)"\x1C",1); CHECK(ret>=0,lblKO);
	ret= bufApp(&res,(byte *)"\x1C",1); CHECK(ret>=0,lblKO);

	memcpy(TxnAmountBlk,"83444FFFFFFFFFFF",16);CHECK(ret>=0,lblKO);
	memcpy(AuthParam,"\x83\x44\x4F\xFF\xFF\xFF\xFF\xFF",8);CHECK(ret>=0,lblKO);

	    
	memset (ConstructRspPacket, '\0', 200);
	memcpy (ConstructRspPacket+strlen((char*)ConstructRspPacket), res.ptr,strlen((char*)res.ptr));
	memcpy (ConstructRspPacket+strlen((char*)ConstructRspPacket), ResMac,strlen((char*)ResMac));


    
	AuthParameterType=3;//DE_MAC_KEY
    TransactionType=TxnAmountBlk[0];
	fmtTok((char*)TxnAmount,(char *)&TxnAmountBlk[2],"F");
    

    ret= cryStart('m');
    ret= a40CheckMac(TransactionType,AuthParameterType,ConstructRspPacket,AuthParam,TxnAmount,ReqMacResidue,ResMacResidue,ResMac,MacKeyLoc,PinKeyLoc);CHECK(ret>=0,lblKO); 
   
	memcpy(CResMacResidue,"\xC2\x45\x21\x12",4);
	CHECK(memcmp(ResMacResidue,CResMacResidue,4)==0,lblKO);  


	if (ret == 5) //request and responce MAC are different DE_FAIL 5
	{
		dspLS(1,"MAC not Matching");  
		goto lblKO;
	}
	else
	{
	
		ret= a40UpdateKeyRegister(ReqMacResidue,ResMacResidue,WKLoc);CHECK(ret>=0,lblKO);
	}	
	
    ret= cryVerify(WKLoc,crt); CHECK(ret>=0,lblKO);
    
	memcpy(WorkingKey,"\xC8\x8C\x54\xFB\x57\x2A\x8C\xF2",8);
	memcpy(WorkingKey+8,"\xC8\x8C\x54\xFB\x57\x2A\x8C\xF2",8);
	getKCV(WorkingKey,ChkSum);

	ret= hex2bin(bChksum,ChkSum,2); CHECK(ret==2,lblKO); 
   	CHECK(memcmp(crt,bChksum,2)==0,lblKO);  
	ret= dspLS(2,"KeyReg Updated");


    
    goto lblEnd;
lblKO:
    trcErr(ret);
    dspLS(0,"KO!");
lblEnd:
    tmrPause(3);
    cryStop('m');
    prtStop();
    dspStop();
}
#else
void tctt0019(void) {}
#endif
