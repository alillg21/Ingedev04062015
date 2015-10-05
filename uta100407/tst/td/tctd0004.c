
// Functional test case tctd0004.
//
// author:      Tamas Dani
// date:        11.07.2005
// description: connection to Erste PAD ()

#include <stdio.h>
#include <string.h>
#include "log.h"

#ifdef __TEST__

#define CHK CHECK( ret >= 0, lblKO )

static code const char *srcFile = __FILE__;

static int waitForStar(void){
    int ret;
    int idx;
    byte b;
    char msg[dspW+1];

    for(idx= 0; idx<64; idx++){
        ret= comRecv(&b,20); //receive characters
        if(ret<0) break;
        if('A'<=b && b<='Z'){
            msg[idx%dspW]= (char)b; //display ascii characters
            dspLS(2,msg);
        }
        else if(b=='*'){
            return 1;
        }
    }
    return 0;
}

static int waitForENQ(void){
    int ret;
    int idx;
    byte b;
    char msg[dspW+1];

    for(idx= 0; idx<64; idx++){
        ret= comRecv(&b,20); //receive characters
        if(ret<0) break;
        if('A'<=b && b<='Z'){
            msg[idx%dspW]= (char)b; //display ascii characters
            dspLS(2,msg);
        }
        else if(b==5){
            return 1;
        }
    }
    return 0;
}

void tctd0004(void){
    int ret;
    char *ptr;
    char msg[dspW+1];

    memset(msg,0,dspW+1);
    ret= dspStart(); CHECK(ret>=0,lblKO);
    ret= dspLS(0,"comSet..."); CHECK(ret>=0,lblKO); 
//  ptr= "7E11200"; //GIRO
    ptr= "7O11200"; //ERSTE
    ret= dspLS(3,ptr); CHECK(ret>=0,lblKO); 

    ret= comStart(chnMdm); CHECK(ret>=0,lblKO);
    ret= comSet(ptr);
    CHECK(ret>=0,lblKO);
    
    ret= dspLS(0,"comSet OK"); CHECK(ret>=0,lblKO); 
    
    ret= dspLS(1,"comDial..."); CHECK(ret>=0,lblKO); 
    ptr= "0-06-12670756 "; //ERSTE
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
    
    ret= dspLS(1,"comDial OK"); CHECK(ret>=0,lblKO); 
    ret= waitForStar(); CHECK(ret>0,lblKO); 

    ptr= "N1728031740POSTAB-15012126703\x0D";
    ret= comSendReq((byte *)ptr,(word)strlen(ptr));
    CHECK(ret>0,lblKO);

    ret= waitForENQ();
    CHECK(ret>0,lblKO);

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

#endif
