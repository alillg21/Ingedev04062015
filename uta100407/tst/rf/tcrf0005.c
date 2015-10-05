#include <string.h>
#include "tst.h"

char msgstr[200];

static code const char *srcFile= __FILE__;

//Unitary test case tcrf0005.
//Downloading data - resolving 881: Full download

static int waitForStar(void){
    byte trm[]= {'*',0};
    return comRecvRsp(0,trm,60); //wait for *
}
static int waitForPAD(void){
    byte trm[]= {'\x05',0};
    return comRecvRsp(0,trm,60); //wait for ENQ
}
static int recvRsp(void){
	int ret;
    byte dRsp[512];
    tBuffer bRsp;

    bufInit(&bRsp,dRsp,512);
    ret= comRecvStxDataEtxLrc(&bRsp,60); //receive package
    CHECK(ret>=0,lblKO);
	strcpy(msgstr,dRsp);
	return bufLen(&bRsp);
lblKO:
	return -1;
}

void tcrf0005(void){
    int ret;
    char *ptr;
	char msg[dspW+1];
	char msglrc;
	char errcode[3];
	char lastfid[2];

    ret= dspStart(); CHECK(ret>=0,lblKO);
    
    memset(msg,0,dspW+1);
    
    ret= dspLS(0,"comSet..."); CHECK(ret>=0,lblKO); 
    ptr= "07E11200"; //GIRO
    ret= dspLS(3,ptr); CHECK(ret>=0,lblKO); 

    TRCDT("comSet...");
    ret= comStart(0); CHECK(ret>=0,lblKO);
    ret= comSet(ptr); CHECK(ret>=0,lblKO);
    TRCDT("comSet done");
    
	ret= dspLS(0,"comSet OK"); CHECK(ret>=0,lblKO); 
    
    ret= dspLS(1,"comDial..."); CHECK(ret>=0,lblKO); 
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
    
//	Sending "Initial download" request to the Host

	strcpy(msgstr,"\x02");
	strcat(msgstr,"9\x2E");
	strcat(msgstr,"0012345000              050323161310AO90000000\x1C");
	strcat(msgstr,"V001000000000000\x03");
	strcat(msgstr,"j");
	ptr=msgstr;
	
	ret= comSendReq((byte *)ptr,(word)strlen(ptr)); CHECK(ret>0,lblKO);
	recvRsp(); //the recieved data stored in msgstr
	fmtSbs(errcode,msgstr,46,3); // Error code
	
//	Checking if Continuation required

	while (strncmp(errcode,"881",3)==0) { // If 881
		// processing continuation download
		fmtSbs(lastfid,msgstr,54,1); // The last downloaded FID
		strcpy(msgstr,"\x02");
		strcat(msgstr,"9\x2E");
		strcat(msgstr,"0012345000              050323161310AO90000000\x1C");
		strcat(msgstr,"V002");
		strcat(msgstr,lastfid);
		strcat(msgstr,"00000000000\x03"); // plus a "0" after lastfid, because " " doesn't work
	
		msglrc= stdLrc(&msgstr[1], strlen(msgstr)-1); // calculating Lrc
		strcat(msgstr, &msglrc);

		ret= comSendReq((byte *)ptr,(word)strlen(ptr)); CHECK(ret>0,lblKO); 

		recvRsp();
		fmtSbs(errcode,msgstr,46,3); // Error code
	}

// Checking if there's more mail exists on the server 

	if (strncmp(errcode,"880",3)==0) { // If 881
		fmtSbs(lastfid,msgstr,54,1); // The last downloaded FID
		strcpy(msgstr,"\x02");
		strcat(msgstr,"9\x2E");
		strcat(msgstr,"0012345000              050323161310AO90000000\x1C");
		strcat(msgstr,"V002");
		strcat(msgstr,lastfid);
		strcat(msgstr,"00000000000\x03"); // plus a "0" after lastfid, because " " doesn't work
	
		msglrc= stdLrc(&msgstr[1], strlen(msgstr)-1); // calculating Lrc
		strcat(msgstr, &msglrc);

		ret= comSendReq((byte *)ptr,(word)strlen(ptr)); CHECK(ret>0,lblKO); 

		recvRsp();
		fmtSbs(errcode,msgstr,46,3); // Error code
	}

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
