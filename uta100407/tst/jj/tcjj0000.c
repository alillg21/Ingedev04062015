#include <pinpad.h>
#include <Sec.h>

#define KEYS_CYPHER_BLOCK_SIZE					8
#define KEYS_MERCHANT_DEFAULT					"DEFAULT"
#define KEYS_SCHEME_DEBIT						"GENERIC DEBIT PIN"

static secHandle_t SessionID;

static secError_t keyGetHandle(secHandle_t* pSession, const char* EncryptionScheme, const char* AcquirerName, const char* MerchantName)
{
	secError_t 	eError;
	
	eError		= secSessionOpen(pSession);
	
	if(eError == RET_OK)
		eError 	= secSetTag(*pSession, SEC_SPONSOR_NAME,	strlen(AcquirerName), 		AcquirerName);

	if(eError == RET_OK)
		eError	= secSetTag(*pSession, SEC_ACQUIRER_NAME, 	strlen(EncryptionScheme), 	EncryptionScheme);

	if(eError == RET_OK)
		eError	= secSetTag(*pSession, SEC_MERCHANT_NUMBER,	strlen(MerchantName), 		MerchantName);

	if(eError != RET_OK)
		secSessionClose(*pSession);
	
	return eError;
}



static void testTK(void)
{
       uint8 testbuff[8];
       size_t buffsize=sizeof(testbuff);

	pp_display_clear(0);
	pp_display_text(1, 1, "cryStart...", PT_ALIGN_LEFT);
	pp_get_key(1);

	// Open the Session 
	secSessionOpen(&SessionID);

	// Get Correct Handle based on the Acquirer
	if(keyGetHandle(&SessionID, "GENERIC DEBIT PIN", "ISEA", KEYS_MERCHANT_DEFAULT) != RET_OK)
		return;

       pp_display_clear(2);
	pp_display_text(2, 1, "cryLoadKey...", PT_ALIGN_LEFT);
	pp_get_key(1);

	// Load the keys using the proper Tag
	// KCA - Cross Aquirer Key (Used to Encrypt KEK)
	// KEK - eKCA(KEK) 
	if(secGetTag(SessionID, SEC_KCA, &buffsize, &testbuff) != RET_OK)
	{
	  secSetTag(SessionID, SEC_KCA,	8, "\x11\x11\x11\x11\x11\x11\x11\x11");
	  secSetTag(SessionID, SEC_KEK,	8, "\xF4\x03\x79\xAB\x9E\x0E\xC5\x33"); //=DES(11 11 11 11 11 11 11 11,11 11 11 11 11 11 11 11);
	}

	pp_display_clear(3);
	pp_display_text(3, 1, "cryVerify...", PT_ALIGN_LEFT);
	pp_get_key(1);

	memset(&testbuff,0,sizeof(testbuff));
	secGetTag(SessionID, SEC_KCA, &buffsize, &testbuff); //testbuf=DES(00 00 00 00 00 00 00 00,11 11 11 11 11 11 11 11)

	// Check the Transport key we store on the TMS  1111....1111
	memset(&testbuff,0,sizeof(testbuff));
	secGetTag(SessionID, SEC_KEK, &buffsize, &testbuff); //testbuf=DES(00 00 00 00 00 00 00 00,11 11 11 11 11 11 11 11)

       // Compare with the expected certificate
       pp_display_clear(4);
	if(!memcmp(testbuff, "\x82\xE1\x36", 3))
	   pp_display_text(4, 1, "OK!", PT_ALIGN_LEFT);
	else
	   pp_display_text(4, 1, "KO!", PT_ALIGN_LEFT);
	pp_get_key(1);
}


static void testWK(void)
{
       uint8 testbuff[8];
       size_t buffsize=sizeof(testbuff)-1;

       // Start Encryption
	pp_display_clear(0);
	pp_display_text(1, 1, "cryStart...", PT_ALIGN_LEFT);
	pp_get_key(1);
	
       testTK();

       pp_display_clear(0);
	pp_display_text(2, 1, "cryLoadDKuDK...", PT_ALIGN_LEFT);
	pp_get_key(1);

	// eKEK(KPE)  
	// Load our Encrypted Working Key 
	secSetTag(SessionID, SEC_KPE,	 8, "\x42\x25\xEA\x8E\x9A\x0B\x5F\x63"); //DES(WK=6B218F24DE7DC66C,TK=1111111111111111)

       // Verify Key we loaded on the SSA
	pp_display_clear(3);
	pp_display_text(3, 1, "cryVerify...", PT_ALIGN_LEFT);
	pp_get_key(1);
	
	memset(&testbuff,0,sizeof(testbuff));
	secGetTag(SessionID, SEC_KPE, &buffsize, &testbuff); //testbuf=DES(00 00 00 00 00 00 00 00,WK=6B218F24DE7DC66C)

	// Check by comparing the certificate
       pp_display_clear(4);
	if(!memcmp(testbuff, "\xF7\x53\xC2", 3))
	   pp_display_text(4, 1, "OK!", PT_ALIGN_LEFT);
	else
	   pp_display_text(4, 1, "KO!", PT_ALIGN_LEFT);
	pp_get_key(1);
	
	return;
}

void tcjj0000(void){
       uint8 testbuff[8+1];
       uint8 const PAN[]="2347890123456741";
	size_t buffsize=sizeof(testbuff)-1;   


	testWK();

       pp_display_clear(0);
	pp_display_text(2, 1, "Key Exist...", PT_ALIGN_LEFT);
	pp_get_key(1);
	
	secSetTag(SessionID, SEC_PAN, sizeof(PAN)-1, PAN);
	
       /* test PIN Encryption (format 0) */
	memset(&testbuff,0,sizeof(testbuff));  
       pp_display_clear(0);
       pp_display_text(1, 0, "ENTER PIN:", 0);
       pp_pin_entry(0, 8, 1<<26, 100);
       secGetTag(SessionID,SEC_STANDARD_ENCRYPTED_PIN,&buffsize,testbuff);

	pp_display_clear(3);
	pp_display_text(3, 1, "cryVerify...", PT_ALIGN_LEFT);
	pp_get_key(1);
	
	
       pp_display_clear(4);
	if(!memcmp(testbuff, "\x06\xDA\x91\xDF\xF2\x76\xD3\xA8", 8))
	   pp_display_text(4, 1, "OK!", PT_ALIGN_LEFT);
	else
	   pp_display_text(4, 1, "KO!", PT_ALIGN_LEFT);
	pp_get_key(1);
	
	secSessionClose(SessionID);
	return;
}
