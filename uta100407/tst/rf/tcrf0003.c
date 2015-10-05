#include <string.h>
#include "tst.h"

static code const char *srcFile= __FILE__;

//Unitary test case tcrf0003.
//Downloading data

static int waitForStar(void){
    byte trm[]= {'*',0};
    return comRecvRsp(0,trm,60); //wait for *
}
static int waitForPAD(void){
    byte trm[]= {'\x05',0};
    return comRecvRsp(0,trm,60); //wait for ENQ
}
static int waitForInitDResp(void){
	/*
	x029x2E0012345000...........
    ...050411160436AO9000088
    1x1CV0021.0000000000x1CW0x1D0.
    50000000000509999999991-
    4212700...........1-4212
    700...........421-2200..
    ..........0,999999....11
    0110MM......x1D1.560000000
    0058999999999...........
    ........................
    .....421-2200...........
    .0,999999....110110.....
    ...x03x09
	*/ // 12x24 + 5 = 293 bytes
    byte trm[]= {'\x03',0};
    return comRecvRsp(0,trm,60); //wait for ETX & LRC
}
static int recvRsp(void){
	int ret;
    byte dRsp[512];
    tBuffer bRsp;

    bufInit(&bRsp,dRsp,512);
    ret= comRecvStxDataEtxLrc(&bRsp,60); //receive package
    CHECK(ret>=0,lblKO);
    return bufLen(&bRsp);    
lblKO:
	return -1;
}

void tcrf0003(void){
    int ret;
    int idx;
    byte b;
    char *ptr;
	char msgstr[200];
	char msg[dspW+1];
	char msglrc;

    ret= dspStart();
    CHECK(ret>=0,lblKO);
    
    memset(msg,0,dspW+1);
    
    ret= dspLS(0,"comSet..."); CHECK(ret>=0,lblKO); 
//    ptr= "07E11200"; //ATOS
//      ptr= "07E12400"; //SMT
//      ptr= "07E11200"; //SG
    ptr= "07E11200"; //GIRO
    ret= dspLS(3,ptr); CHECK(ret>=0,lblKO); 

    TRCDT("comSet...");
    ret= comStart(0); CHECK(ret>=0,lblKO);
//    ret= comSet("07E1");  //open internal modem using 7,E,1 and default baud rate
    ret= comSet(ptr);
    CHECK(ret>=0,lblKO);
    TRCDT("comSet done");
    
    ret= dspLS(0,"comSet OK"); CHECK(ret>=0,lblKO); 
    
    ret= dspLS(1,"comDial..."); CHECK(ret>=0,lblKO); 
    //ptr= "0-0836065555"; //ATOS
    //ptr= "0-00-21671951036"; //SMT
    //ptr= "0-00-97165739374"; //SG
    ptr= "0-4212700"; //GIRO
    ret= dspLS(3,ptr); CHECK(ret>=0,lblKO); 
    
    TRCDT("comDial...");
    ret= comDial(ptr);
    if(ret<0){
        switch(-ret){
            case comAnn: ret= dspLS(2,"comDial Aborted"); break;
            case comBusy: ret= dspLS(2,"comDial BUSY"); break;
            case comNoDialTone: ret= dspLS(2,"NO TONE"); break;
            case comNoCarrier: ret= dspLS(2,"NO CARRIER"); break;                   
            default: ret= dspLS(2,"UNKNOWN PB"); break;
        }
        goto lblEnd;
    }
    CHECK(ret>=0,lblKO);
    TRCDT("comDial done");
    
    ret= dspLS(1,"comDial OK");
    CHECK(ret>=0,lblKO); 

    ret= waitForStar();
    CHECK(ret>0,lblKO); 

// Sending request to the PAD

	ptr= "N123456789123456789-12121212306\x0D\x0A";
	ret= comSendReq((byte *)ptr,(word)strlen(ptr));
	CHECK(ret>0,lblKO); 

	ret= waitForPAD();
    CHECK(ret>0,lblKO); 
    
/*	for(idx= 0; idx<64; idx++){
		ret= comRecv(&b,20); //receive characters
		if(ret<0) break;
		if('A'<=b && b<='Z'){
			msg[idx%dspW]= (char)b; //display ascii characters
			dspLS(2,msg);
		}
	}
*/
 
// download only a specific DID field // for testing only
/*
	strcpy(msgstr,"\x02");
	strcat(msgstr,"9\x2E");
	strcat(msgstr,"0012345000              050323161310AO90000000\x1C");
	strcat(msgstr,"V005000000000000\x03");
	strcat(msgstr,"j");
	ptr=msgstr;

	ret= comSendReq((byte *)ptr,(word)strlen(ptr));
	CHECK(ret>0,lblKO); 

	for(idx= 0; idx<64; idx++){
		ret= comRecv(&b,20); //receive characters
		if(ret<0) break;
		if('A'<=b && b<='Z'){
			msg[idx%dspW]= (char)b; //display ascii characters
			dspLS(2,msg);
		}
	}
*/
//	Sending "Initial download" request to the Host

	strcpy(msgstr,"\x02");
	strcat(msgstr,"9\x2E");
	strcat(msgstr,"0012345000              050323161310AO90000000\x1C");
	strcat(msgstr,"V001000000000000\x03");
	strcat(msgstr,"j");
	ptr=msgstr;
	// 54th char -> the last DID
	
	ret= comSendReq((byte *)ptr,(word)strlen(ptr));
	CHECK(ret>0,lblKO); 

	//ret= waitForInitDResp();
	//CHECK(ret>0,lblKO); 

	recvRsp();
	
	/*for(idx= 0; idx<64; idx++){
		ret= comRecv(&b,20); //receive characters
		if(ret<0) break;
		if('A'<=b && b<='Z'){
			msg[idx%dspW]= (char)b; //display ascii characters
			dspLS(2,msg);
		}
	}*/

// 	Sending "Continuation download" request to the Host

	strcpy(msgstr,"\x02");
	strcat(msgstr,"9\x2E");
	strcat(msgstr,"0012345000              050323161310AO90000000\x1C");
	strcat(msgstr,"V002100000000000\x03");
	
	msglrc= stdLrc(&msgstr[1], strlen(msgstr)-1); // calculating Lrc
	strcat(msgstr, &msglrc);
	ptr=msgstr;

	ret= comSendReq((byte *)ptr,(word)strlen(ptr));
	CHECK(ret>0,lblKO); 

	recvRsp();

// 	Sending "Continuation download" request to the Host

	strcpy(msgstr,"\x02");
	strcat(msgstr,"9\x2E");
	strcat(msgstr,"0012345000              050323161310AO90000000\x1C");
	strcat(msgstr,"V002300000000000\x03");
	
	msglrc= stdLrc(&msgstr[1], strlen(msgstr)-1); // calculating Lrc
	strcat(msgstr, &msglrc);
	ptr=msgstr;

	ret= comSendReq((byte *)ptr,(word)strlen(ptr));
	CHECK(ret>0,lblKO); 

	recvRsp();

// End of Downloading Data

    ret= dspLS(3,"Receiving done");
    CHECK(ret>=0,lblKO); 
        
    ret= comHangStart();
    CHECK(ret>=0,lblKO); 

    ret= comHangWait();
    CHECK(ret>=0,lblKO); 

    ret= comStop(); //close the port
    CHECK(ret>=0,lblKO); 

    tmrPause(3);
    goto lblEnd;

lblKO:
    trcErr(ret);
    dspLS(0,"KO!");
    tmrPause(3);
lblEnd:
    tmrPause(2);
    comStop();
    dspStop();
}
