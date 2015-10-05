#include <string.h>
#include "pri.h"
#include "tst.h"


static code const char *srcFile= __FILE__;


static int waitForEnq(void){
    byte trm[]= {cENQ,0};
    return comRecvRsp(0,trm,10); //wait for ENQ
}

static int bufAppByte(tBuffer *buf,byte b){
        VERIFY(buf);
        return bufApp(buf,&b,1);
}



//Request Instruction Set
static int prepareReq01(tBuffer *buf){
        int ret;
        byte lrc;
        VERIFY(buf);

        ret= bufAppByte(buf,cSTX); CHECK(ret>0,lblKO);
        ret= bufAppByte(buf,cFS); CHECK(ret>0,lblKO);
        ret= bufAppStr(buf,"43"); CHECK(ret>0,lblKO); //D1, Size of Packet including STX and ETX
        ret= bufAppByte(buf,cFS); CHECK(ret>0,lblKO);
//      ret= bufAppStr(buf,"7"); CHECK(ret>0,lblKO); //D2, Packet Version (7|8|9)
        ret= bufAppByte(buf,cFS); CHECK(ret>0,lblKO);
        ret= bufAppStr(buf,"0"); CHECK(ret>0,lblKO); //D3, Sequence count
        ret= bufAppByte(buf,cFS); CHECK(ret>0,lblKO);
        ret= bufAppStr(buf,"0"); CHECK(ret>0,lblKO); //D4, Retransmit count
        ret= bufAppByte(buf,cFS); CHECK(ret>0,lblKO);
        ret= bufAppStr(buf,"50"); CHECK(ret>0,lblKO); //D5, Application Identity, 50= Schlumberger
        ret= bufAppByte(buf,cFS); CHECK(ret>0,lblKO);
        ret= bufAppStr(buf,"1"); CHECK(ret>0,lblKO); //D6, Session Requests, 1= Single Request Session
        ret= bufAppByte(buf,cFS); CHECK(ret>0,lblKO);
        ret= bufAppStr(buf,"4"); CHECK(ret>0,lblKO); //D7, Command Number, 7= Request Report From Host
        ret= bufAppByte(buf,cFS); CHECK(ret>0,lblKO);
        ret= bufAppStr(buf,"100"); CHECK(ret>0,lblKO); //D10, Terminal Software Version
        ret= bufAppByte(buf,cFS); CHECK(ret>0,lblKO);
        //ret= bufAppStr(buf,""); CHECK(ret>0,lblKO); //D20, Reserved
        ret= bufAppByte(buf,cFS); CHECK(ret>0,lblKO);
        ret= bufAppStr(buf,"1234567"); CHECK(ret>0,lblKO); //D21, Merchant Number
        //      ret= bufAppStr(buf,"33300012"); CHECK(ret>0,lblKO); //D21, Merchant Number
        //ret= bufAppStr(buf,"11111111"); CHECK(ret>0,lblKO); //D21, Merchant Number
        ret= bufAppByte(buf,cFS); CHECK(ret>0,lblKO);
        ret= bufAppStr(buf,"11110001"); CHECK(ret>0,lblKO); //D22, Terminal ID
        //ret= bufAppStr(buf,"18807598"); CHECK(ret>0,lblKO); //D22, Terminal ID        
        //ret= bufAppStr(buf,"1234568"); CHECK(ret>0,lblKO); //D22, Terminal ID 
        ret= bufAppByte(buf,cFS); CHECK(ret>0,lblKO);
        //ret= bufAppStr(buf,""); CHECK(ret>0,lblKO); //D23, Terminal Parameter Version Number
//      ret= bufAppStr(buf,"5"); CHECK(ret>0,lblKO);
        //ret= bufAppStr(buf,""); CHECK(ret>0,lblKO); //D24, Terminal Instruction Set Version Number
        ret= bufAppByte(buf,cFS); CHECK(ret>0,lblKO);
        //ret= bufAppStr(buf,""); CHECK(ret>0,lblKO); //D25, Reserved
        ret= bufAppByte(buf,cFS); CHECK(ret>0,lblKO);
        ret= bufAppByte(buf,cFS); CHECK(ret>0,lblKO);
        ret= bufAppByte(buf,cFS); CHECK(ret>0,lblKO);
//      ret= bufAppStr(buf,"011234"); CHECK(ret>0,lblKO); //P1, Action Type
        /*ret= bufAppByte(buf,cFS); CHECK(ret>0,lblKO);
        ret= bufAppStr(buf,"319"); CHECK(ret>0,lblKO); //P3 Package Id
        ret= bufAppByte(buf,cFS); CHECK(ret>0,lblKO);
        ret= bufAppStr(buf,"011234"); CHECK(ret>0,lblKO); //P7 User Name & password
        ret= bufAppByte(buf,cFS); CHECK(ret>0,lblKO);
        ret= bufAppStr(buf,"0"); CHECK(ret>0,lblKO);   //P9 Item cODE INITITED
        ret= bufAppByte(buf,cFS); CHECK(ret>0,lblKO);
        ret= bufAppStr(buf,"0"); CHECK(ret>0,lblKO);     //P10 Capture Type
        ret= bufAppByte(buf,cFS); CHECK(ret>0,lblKO);
        //ret= bufAppStr(buf,""); CHECK(ret>0,lblKO);   //P11 Captured Data
        ret= bufAppByte(buf,cFS); CHECK(ret>0,lblKO);
        ret= bufAppStr(buf,"0"); CHECK(ret>0,lblKO);    // P12 Use Encrption
        ret= bufAppByte(buf,cFS); CHECK(ret>0,lblKO);
        //ret= bufAppStr(buf,""); CHECK(ret>0,lblKO);   // P13 E-Key-PBL
        ret= bufAppByte(buf,cFS); CHECK(ret>0,lblKO);
//      ret= bufAppStr(buf,"438"); CHECK(ret>0,lblKO);   // P14 E-Key-PVT
        ret= bufAppByte(buf,cFS); CHECK(ret>0,lblKO);
        ret= bufAppStr(buf,"1"); CHECK(ret>0,lblKO);    //P60 Quantity 
        ret= bufAppByte(buf,cFS); CHECK(ret>0,lblKO);
        ret= bufAppStr(buf,"10000"); CHECK(ret>0,lblKO); //P61 Value in Cents
        ret= bufAppByte(buf,cFS); CHECK(ret>0,lblKO);
        //ret= bufAppStr(buf,""); CHECK(ret>0,lblKO);   //A1 Account Number 
        ret= bufAppByte(buf,cFS); CHECK(ret>0,lblKO);
        ret= bufAppStr(buf,"9820123456"); CHECK(ret>0,lblKO); // A3 Client Phone Number
        ret= bufAppByte(buf,cFS); CHECK(ret>0,lblKO);
        */ret= bufAppByte(buf,cETX); CHECK(ret>0,lblKO);
        lrc= stdLrc(bufPtr(buf),bufLen(buf)); //calculate LRC
    ret= bufApp(buf,&lrc,1);CHECK(ret>0,lblKO);//append LRC
    return bufLen(buf);
lblKO: return -1;
}

#define BUFLEN 100
void tcpa0001(void){
    int ret;
    int idx;
    byte b;
    char *ptr;
    char msg[dspW+1];
        tBuffer buf;
        byte dat[BUFLEN];

    memset(msg,0,dspW+1);
    bufInit(&buf,dat,BUFLEN);
        ret= dspStart(); CHECK(ret>=0,lblKO);
     
    ret= comStart(chnMdm); CHECK(ret>=0,lblKO);
        ret= dspLS(0,"comSet..."); CHECK(ret>=0,lblKO); 
    ptr= "8N12400"; //C.A.P.S.
    ret= dspLS(3,ptr); CHECK(ret>=0,lblKO); 
    ret= comSet(ptr); CHECK(ret>=0,lblKO);
    ret= dspLS(0,"comSet OK"); CHECK(ret>=0,lblKO); 
    
    ret= dspLS(1,"comDial..."); CHECK(ret>=0,lblKO); 
      ptr= "0-00911244117131";//C.A.P.S.
//      0091-124- 4117131
//        ptr= "0-4117131";//C.A.P.S.
//ptr = "5624" ;
        //ptr= "0-951151510761";//C.A.P.S.
        ret= dspLS(3,ptr); CHECK(ret>=0,lblKO);     
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
  
    ret= dspLS(1,"comDial OK"); CHECK(ret>=0,lblKO); 
    ret= dspLS(2,"wait for ENQ"); CHECK(ret>=0,lblKO); 
   
        ret= waitForEnq(); CHECK(ret>0,lblKO); 
    ret= dspLS(2,"ENQ OK"); CHECK(ret>=0,lblKO); 

        tmrPause(3);
        ret= prepareReq01(&buf);
        ret= comSendBuf(bufPtr(&buf),bufLen(&buf));
        CHECK(ret>0,lblKO);

//      for(idx= 0; idx<100; idx++){
    do
        {
        ret= comRecv(&b,10); //receive characters
        if(ret<0) break;
        if('A'<=b && b<='Z')
                {
          //  msg[idx%dspW]= (char)b; //display ascii characters
            dspLS(2,msg);
        }
        }while (b!=cETX);
   // }
        tmrPause(1);
        
// sending again message for testing 
        
        ret= comSendBuf(bufPtr(&buf),bufLen(&buf));
        CHECK(ret>0,lblKO1);

        tmrPause(1);    
         for(idx= 0; idx<200; idx++)
         {
        ret= comRecv(&b,500); //receive characters
        if(ret<0) break;
        if('A'<=b && b<='Z')
                {
            msg[idx%dspW]= (char)b; //display ascii characters
            dspLS(2,msg);
        }
    }
        
        tmrPause(1);    
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
        goto lblEnd;
lblKO1:
    trcErr(ret);
    dspLS(0,"PUNEET !!");
        goto lblEnd;
lblEnd:
    tmrPause(3);
    comStop();
    dspStop();
}


        
