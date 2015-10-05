#include <string.h>
#include "log.h"
#include "tst.h"

#ifdef __TEST__
static code const char *srcFile= __FILE__;

//Integration test case tctt0004.
//Getting Permission details.

void tctt0004(void){
        card txnPer;
        byte txnType;
        byte buf[8];

//------------------------
double x=1234567890123456; //16 digits
//char str[20];
//sprintf(str,"%.0f",x);
//sprintf(str,"%.2f",x);

x=-123456789012345;
//sprintf(str,"%.0f",x);
//------------------------
        txnPer = 0x00001F9F;
        memcpy(buf,"\x00\x00\x1F\x9F",8);

        // in bitTest function, bits are counted from left to right starting from 1 

        txnType = bitTest(buf,1); //RFU (Reserved for Future Use)
        txnType = bitTest(buf,2); //RFU
        txnType = bitTest(buf,3); //RFU
        txnType = bitTest(buf,4); //RFU
        txnType = bitTest(buf,5); //RFU
        txnType = bitTest(buf,6); //RFU
        txnType = bitTest(buf,7); //RFU
        txnType = bitTest(buf,8); //RFU
        
        txnType = bitTest(buf,9); //RFU
        txnType = bitTest(buf,10); //RFU
        txnType = bitTest(buf,11); //RFU
        txnType = bitTest(buf,12); //RFU
        txnType = bitTest(buf,13); //RFU
        txnType = bitTest(buf,14); //FullBatch Transfer
        txnType = bitTest(buf,15); //Loyalty
        txnType = bitTest(buf,16); //Offline Refund
        
        txnType = bitTest(buf,17); //Pre-Auth Completion
        txnType = bitTest(buf,18); //Installment
        txnType = bitTest(buf,19); //CashBack
        txnType = bitTest(buf,20); //Tip
        txnType = bitTest(buf,21); //Report
        txnType = bitTest(buf,22); //Batch
        txnType = bitTest(buf,23); //Pre-Receipt
        txnType = bitTest(buf,24); //Duplicate

        txnType = bitTest(buf,25); //Offline
        txnType = bitTest(buf,26); //Pre-Auth
        txnType = bitTest(buf,27); //Mail Order
        txnType = bitTest(buf,28); //Manual Entry
        txnType = bitTest(buf,29); //Void
        txnType = bitTest(buf,30); //Return
        txnType = bitTest(buf,31); //Cash
        txnType = bitTest(buf,32); //Purchase

}
#endif