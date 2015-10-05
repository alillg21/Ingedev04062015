#include <string.h>
#include "sys.h"
#include "tst.h"

#define TSA_OK 1

/* Telium Key Injection Application (KIA) is used by NAR to inject all existing key files used by the US.
 * Telium System Application (TSA) is used to view KCV,Serial,Secret Area,etc.  TSA also comes with
 * a TsaLib.lib which can be link to your application.  A function called TSA_GetKeyID  populates T_SEC_DATAKEY_ID data structure.
 * 
 * Note: TSA_GetKeyID will only compile inside sgm.c due to sdk30.h requirement.
 *       tchs0003 T_SEC_DATAKEY_ID is hard coded to exercise cryDukptGetPinTSA(...) in system component.
 * 
 * Schemes used: SDukptLDes.sgn, SDukptLTDes.sgn, TlvKeyVerif.sgn, SDK's Schemes.
 * TSA header file : TSA_def.h, TSA_api.h
 * TDES key file FK300423 is injected thru KIA (NAR Application)
 */
int tchs0003(void) {	
	//T_SEC_DATAKEY_ID    stDukptAreaId;
	int SecretArea;
	char AlgoType;
	word Number;
	unsigned int BankId;
	char *msg1 = "ENTER PIN:";
	char *msg2 = " ";
	byte acc[8];
	memcpy(acc,"\x00\x00\x40\x12\x34\x56\x78\x90",8);
    byte blk[8];                // Pinblock calculated by cryptomodule
    byte SerialNumberKey[10];   // SNKey return at pin = KET SET ID + TRMS ID + TRANSACTION COUNTER
	int ret = 0;
	unsigned char LocPK = 0;   // Key location 0-9
	trcS("GetPin_DUKPT Begin\n");
	dspStop();
	//ret = TSA_GetKeyID(TSA_KEY_DUKPT, LocPK,&stDukptAreaId);
	//CHECK(ret >= 0, lblKOINIT);
	ret = 1;
	if(ret==TSA_OK) {
		 ret = cryStart('m');
		CHECK(ret >= 0, lblKO);
		trcFN("TSA_GetKeyID: ret=%d\n", ret);
		// The following data is populated by TSA_GetKeyID, if TsaLib.lib is linked to your project.
		SecretArea = 0x00042131;
		AlgoType = 13;   // See SDK_TELIUM_6.5.3  defines TLV_TYPE_TDESDUKPTLIGHT   = 13
		Number = LocPK*80+32;
		BankId = 0x80000000; 
		ret = cryDukptGetPinTSA(SecretArea, AlgoType, Number, BankId,
				msg1,msg2,acc,SerialNumberKey, blk);
		CHECK(ret >= 0, lblKO);
	}
	dspStart();
	trcS("GetPin_DUKPT End\n");
	goto lblEnd;
	/*lblKOINIT:
	    trcFN("TSA_GetKeyID lblKOINIT=%d\n", ret);
	    ret = -1;
	    goto lblEnd;*/
	lblKO:
	   trcFN("cryDukptGetPinTSA lblKO=%d\n", ret);
	   ret = -1;
	   goto lblEnd;
	lblEnd:
	cryStop('m');               // Stop in case if it was started; the return code is not checked intentionnally 
	return ret;
}
