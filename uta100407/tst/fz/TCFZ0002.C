#include <string.h>
#include "log.h"
#include "tst.h"

#ifdef __TEST__
static code const char *srcFile= __FILE__;

#define CHK CHECK(ret>=0,lblKO)
//Integration test case tcfz0002.
//payment requirement

void tcfz0002(void){
        card Per;
        int ret;

        //Download parameter
        ret= dspClear(); CHECK(ret>=0,lblKO);
        ret= dspLS(0,"download param"); CHECK(ret>=0,lblKO);    
        tcnl0002();
        ret= prtS("    DOWNLOADED PARAM   "); CHECK(ret>=0,lblKO);   
        ret= prtS("-----------------------"); CHK;  

        tmrPause(3);

        //Download (Domestic & Range BINs)
        ret= dspClear(); CHECK(ret>=0,lblKO);
        ret= dspLS(0,"fill bin"); CHECK(ret>=0,lblKO);    
        tctt0002();
        ret= prtS("      FILLED BIN       "); CHECK(ret>=0,lblKO);   
        ret= prtS("-----------------------"); CHK;  

        tmrPause(3);

        //Parsing response.
        ret= dspClear(); CHECK(ret>=0,lblKO);
        ret= dspLS(0,"parse response"); CHECK(ret>=0,lblKO);    
        tcab0038();
        ret= prtS("    PARSED RESPONSE    "); CHECK(ret>=0,lblKO);   
        ret= prtS("-----------------------"); CHK;  

        tmrPause(3);

        //terminal permission to do transaction
        ret= dspClear(); CHECK(ret>=0,lblKO);
        ret= dspLS(0,"set  terminal "); CHECK(ret>=0,lblKO);    
        ret= dspLS(1,"permission "); CHECK(ret>=0,lblKO);    
                
        Per = 0x00FFFFFF; //all transaction are permitted

        fillTxnTypeTerminalPer(Per,TerminalPer);
        ret= prtS("set TERMINAL PERMISSION"); CHK;  
        ret= prtS("-----------------------"); CHK;  

        tmrPause(3);

        //password required
        ret= dspClear(); CHECK(ret>=0,lblKO);
        ret= dspLS(0,"set  password "); CHECK(ret>=0,lblKO);    
        
        Per=0x00000000; //no need asking password

        fillTxnTypeTerminalPassReq(Per,TerminalPer);
        ret= prtS("set OPERATOR PASSWORD"); CHK;  
        ret= prtS("-----------------------"); CHK;  
        
        tmrPause(3);

        //operator permission to do transactions
        ret= dspClear(); CHECK(ret>=0,lblKO);
        ret= dspLS(0,"set  operator "); CHECK(ret>=0,lblKO);    
        ret= dspLS(1,"permission "); CHECK(ret>=0,lblKO);    
        
        Per = 0x00FFFFFF;//all transaction are permitted

        fillTxnTypeTerminalPer(Per , OprPer);   
        ret= prtS("set OPERATOR PERMISSION"); CHECK(ret>=0,lblKO);   
        ret= prtS("-----------------------"); CHK;  

        tmrPause(3);
        
        ret= prtS("     ** FINISHED **    "); CHECK(ret>=0,lblKO);   
        
        hdlCustomer(); 

    goto lblEnd;

lblKO:
    trcErr(ret);
    dspLS(0,"KO!");
    tmrPause(3);
lblEnd:
    tmrStop(1);


}
#endif
