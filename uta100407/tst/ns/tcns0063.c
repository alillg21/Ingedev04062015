#include "sdk30.h"
#include "TlvTree.h"
#include "tst.h"
#include "sys.h"

//Interapplication communications
//
//This Testcase demonstrates a client application within the framework of interapplication 
//communications.
//This sample takes some parameters from user (amount) ant calls the appropriate service 
//(another application) which will treat entered value

// int iacServiceCall(char *tcSnd,char *tcRsp, word usNoAppli, word usService)
//  This function sends a message to the application by calling a service.
//  The message is build under TLV tree format then serialize before sending.
//  An answer is received from receiver. This answer is unserialize to get 
//  TLV tree format again. By using some browsing tree functions the message is
//  retrieved.    
// IN /  tcSnd[]   - request message to send to service
// OUT/  tcRsp[]   - service response 
// IN /  usNoAppli - identifier of the application(APPTYPE, application type) which provides the service                                                                 // USER2 application to call
// IN /  usService - identifier of the service   
//  This function returns a value:
//    >0 : IAC done
//    =0 : IAC missing Appli or no response
//    <0 : IAC failed

int iacServiceCall(StructPt *data, unsigned short usNoAppli, unsigned short usService) 
{
	TLV_TREE_NODE pxNode=0;                                                    
	byte buf[40000]="";// max buffer size for serialised  TLV tree                                                    
	int iSize;
	byte ucPriority;
    int iRet, iCode;
 
    memset(buf,0,sizeof(buf));
   
    // Build message to send under TLV tree format
	// ===========================================
	             
    pxNode = TlvTree_New(0x1110); // Create tree
    TlvTree_AddChild(pxNode, 0x1111, data, sizeof(StructPt));  
	
	 // Get serialized buffer size
	iSize = TlvTree_GetSerializationSize(pxNode, TLV_TREE_SERIALIZER_DEFAULT);          
	CHECK(iSize>0, lblKO);
	
	// Serialize the tree in the allocated buffer
	iRet = TlvTree_Serialize(pxNode, TLV_TREE_SERIALIZER_DEFAULT, buf, iSize);    
    CHECK(iRet>0, lblKO);
    
    // Verify of service exists service 
	iRet = ServiceGet(usNoAppli, usService, &ucPriority); 
	CHECK(iRet==0, lblNoService);
	
	// temporary disable printer and keyboard
	prtStop();
	kbdStop();    
	    
    // Call service and get response
	iRet = ServiceCall(usNoAppli, usService, iSize, buf, &iCode);               
	
	// enable printer and keyboard
	kbdStart(1);   
	prtStart();
	                    	  
	CHECK(iRet==0, lblKO);
    CHECK(iCode>=0, lblNoRsp);
    
    pprintf("USR_ServiceCall : ret = %d, cod = %d",iRet,iCode);
	
    // Parse message received from TLV tree format :
    
    // UnSerialize the tree from the allocated buffer
	iRet = TlvTree_Unserialize(&pxNode, TLV_TREE_SERIALIZER_DEFAULT, buf, iSize); 
    CHECK(iRet==TLV_TREE_OK, lblKO);
    
    // Get message from the tree
    data = TlvTree_GetData(TlvTree_GetFirstChild(pxNode)) ;                     

    iRet = 1;
    goto lblEnd;
    
 lblNoService:
    //no service regoistered 
    iRet=0;
    goto lblEnd;
 lblNoRsp:
	// No response from service      
    iRet=0;
    goto lblEnd;
 lblKO:
	//Processing Error 
    iRet=-1;
 lblEnd: 
    TlvTree_Release(pxNode);    
    return iRet;
}


void tcns0063(void) {
    int ret;
    S_TRANSIN  in;
    S_TRANSOUT out;
    S_MONEY curr;
    StructPt data;
    
        
    unsigned short appID = 0x77; // remote application id
    unsigned short srvID = 101; // remote service id
    
    dspStart();
    prtStart();    
    
    in.amount = 1234;
	    memcpy(curr.nom,"USD",sizeof(T_NOM_MONEY));
	    memcpy(curr.code,"840",sizeof(T_COD_MONEY));    
	    curr.posdec = 2;    
    in.currency = curr;
    in.payment = CARD_PAYMENT;
    in.transaction = DEBIT_TR;
    
    out.rc_payment = 0xFF;
    
    data.Param.SelectFunction.param_in = in;
    data.Param.SelectFunction.param_out = out;
    data.service = srvID;
    data.type_code = appID;
  
    
    prtS("tcns0063 start");
    prtS("pmt 10.00 USD");
    
    prtS("service call: ");
    
    ret = iacServiceCall(&data, appID, srvID) ;    
    CHECK(ret>=0,lblKO);  
    if(ret == 0) goto lblNoService;    
    prtS("service found ");
    
   if(data.Param.SelectFunction.param_out.rc_payment == PAY_OK)
	   prtS("Payment OK");
   else 
	   prtS("Payment failed");
    
	goto lblEnd;
  lblNoService:
  	prtS("IAC: no service");
  	goto lblEnd;
  lblKO:
    prtS("IAC: error");
    trcErr(ret);
  lblEnd:
    prtS("IAC: done");
    prtStop();
    dspStop();
}
