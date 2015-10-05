#include <string.h>
//#include <stdio.h>
#include "log.h"
#include "tst.h"

static code const char *srcFile = __FILE__;

//Unitary test case tcnl0017.
//Functions testing for ethernet:
//connect and send a request 

void tcnl0017(void) {

    int ret = 0;

    ret = dspLS(1, "onlParamDnlTCP");   //CHECK(ret>=0,lblKO);    
    ret = onlParamDlTcpIp(BEG); // CHECK(ret>=0,lblKO);    

    prtS("SAVING JSJSJSJS");
    //save in db
    ret = saveDB(BEG);
    if(ret == 1)
        prtS("SO WHY");
    // CHECK(ret>=0,lblKO);   
    //print dnl
    prtS("BEG FINISHED");

    ret = mapPutByte(regDnlPerformed, 1);   //CHECK(ret>=0,lblKO);   
    prtS("FIRST FINISHED");

    /*ret= onlParamDlTcpIp(BINPARAM);// CHECK(ret>=0,lblKO);             

       dspClear(); //RTCHK;
       ret= dspLS(0,"Save BINPARAM...");       

       //save in db    
       ret= saveDB(BINPARAM);//CHECK(ret>=0,lblKO);  
       ret= mapPutByte(regDnlPerformed ,1); // CHECK(ret>=0,lblKO);   

       /*    ret= dspLS(1,"Logo Download ..."); //RTCHK;    
       ret= onlParamDlTcpIp(BITMAP1);//CHECK(ret>=0,lblKO);          

       dspClear(); //RTCHK;
       ret= dspLS(0,"Save LOGO...");   

       //save in db    
       ret= saveDB(BITMAP1);//CHECK(ret>=0,lblKO);     

       dspClear(); //RTCHK;
       //print dnl
       ret= rptDnl(BITMAP1);//CHECK(ret>=0,lblKO);  
       ret= mapPutByte(regDnlPerformed ,1); // CHECK(ret>=0,lblKO);   
       ret= 1;
     */
    hdlPowerOn();

//      admDownloadDefaultTCPIP();

    //admDownloadTCPIP();      
    // logDailyTCPIP();        
    //logLogoTCPIP();  
    tmrPause(3);

    goto lblEnd;

  lblEnd:
    dspLS(0, "CORECT");
}
