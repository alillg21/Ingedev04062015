#include "sdk30.h"
#include "TlvTree.h"
	

void tcns0064(unsigned char*, unsigned char*);

//  In order to dispatch IAC incoming requests you should register the appropriate service
// in function give_interface():
//
//	MyServices[i].appli_id  = no;
//	MyServices[i].serv_id   = REMOTE_SERVICE;               // Service => Answer service to Training application
//	MyServices[i].sap       = (T_SERVICE_FUNCTION)iacServiceDispatch;
//	MyServices[i].priority  = 20;
//	i++;
//
// where REMOTE_SERVICE is service identifier requested by client applications
//
//Once OS captures an appropriate service call from the client applications
//the function iacServiceDispatch() is being called
//see  https://svn.ingenico.com/UTAPROJECTS/fr-comtech/LLTest 
	
int iacServiceDispatch(unsigned int size, void *msg){

	int ret = 0;
	FILE *pxPrn=NULL;
    TLV_TREE_NODE piNode=NULL;
    NO_SEGMENT No;
    StructPt *data;
    int iRet;
	
	pxPrn = fopen("PRINTER", "w-");

    // Create tree
    No = ApplicationGetCurrent(); 
    
    piNode = TlvTree_New(0x1110);     
    iRet = TlvTree_Unserialize(&piNode, TLV_TREE_SERIALIZER_DEFAULT, msg, size);
    if(iRet!=TLV_TREE_OK) goto lblKO;
   
    data = TlvTree_GetData(TlvTree_GetFirstChild(piNode));      

   /// transaction processing
    iRet = remote_service_process(No,&data->Param.SelectFunction.param_in,&data->Param.SelectFunction.param_out);
    if(iRet!=FCT_OK) goto lblKO;
   
    //TlvTree_AddChild(piNode, 0x1111, data, sizeof(StructPt));
    // Add message to the tree
//	iRet = TlvTree_SetData(TlvTree_GetFirstChild(piNode), data, sizeof(StructPt));
//	if(iRet!=TLV_TREE_OK) goto lblKO;
//	
	//pprintf("pmt: %d\n",data->Param.SelectFunction.param_out.rc_payment);

	 Serialize the tree in the allocated buffer
	iRet = TlvTree_Serialize(piNode, TLV_TREE_SERIALIZER_DEFAULT, msg, size); 
	if(iRet<=0) goto  lblKO;
	
	ttestall(PRINTER, 2*100); 
	
	fclose(pxPrn);

	ret = 1;
	goto lblEnd;

	// Errors treatment 
lblKO:	
	ret=-1;  
lblEnd: 
    // Release ressources 
	TlvTree_Release(piNode);
	return ret;
}

void tcns0064(unsigned char* in, unsigned char* out) {
	FILE *pxPrn=NULL;
   
    pxPrn = fopen("PRINTER", "w-");
    
	strcpy(out,"tcns64 response");
	
	pprintf(" tcns64 got call \n\n"
		           "caller sais:\n%s\n", (char*)in);        // Print msg received 
	
	ttestall(PRINTER, 2*100); 
	
	fclose(pxPrn);
  return;
}
