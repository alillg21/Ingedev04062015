#include "SDK30.H"
#include "log.h"
#include <string.h>
#include "sys.h"
#include "pri.h"
#include "_emvdctag_.h"
#include "bri.h"
#include "prepaid.h"
#include "lineEncryption.h"

#include "logon.h" //@@AS0.0.26
#include "keyinject.h"


#define ISO_BUFFER_LEN 2048 //@agmr

#ifdef TEST_BRI
void testScrollDisplay8();
#endif

extern void tcan0020();

#ifdef LINE_ENCRYPTION
void hex_dump(char *title,void *src,int len) { //@@AS0.0.10
	hex_dump_char(title,src,len);
}
#endif //LINE_ENCRYPTION

int enterHuruf(const char *title, byte len,char *buf)
{
	int ret;
	char dest[256];

	MEM_ZERO(dest);

	dspLS(2, title);
	ret = enterTxt3(3, dest, len, 0xFF);
	CHECK(ret == kbdVAL || ret > 0, lblKO);

	memcpy(buf, dest, strlen(dest));

	lblKO:
		return -1;
}

int enterUang(const char *title, byte len, char *buf)
{
	int ret;
	byte exp;
	char dest[256];

	MEM_ZERO(dest);

	dspLS(2, title);
	MAPGETBYTE(appExp, exp, lblKO);
	ret = enterAmt(3, (char *)dest, exp);
	CHECK(ret == kbdVAL || ret > 0, lblKO);

	memcpy(buf, dest, strlen(dest));

	return ret;
	lblKO:
		return -1;
}

int enterAngka(const char *title, byte len,char *buf)
{
	int ret;
	char dest[256];

	MEM_ZERO(dest);

	dspLS(2, title);
	ret = enterStr(3, dest, len+1);
	CHECK(ret == kbdVAL || ret > 0, lblKO);

	memcpy(buf, dest, strlen(dest));

	return 1;
	lblKO:
		return -1;
}

int pmtPrepare(byte trtType)
{
	int ret;
	char traName[dspW + 1];

	mapMove(rqsBeg, (word) trtType - 1);
	MAPGET(rqsDisp, traName, lblKO);

	MAPPUTSTR(traTransType, traName, lblKO);
	MAPPUTSTR(traTypeName, traName, lblKO);
	MAPPUTBYTE(traTxnType, trtType, lblKO);
	MAPPUTBYTE(traTxnId, trtType, lblKO);

	return ret;
	lblKO:
		return -1;
}

int  pmtPinManual(void) { //from duwi
	char Pan[lenPan + 1];       // Primary account number
	char pwd[6 + 1];
	int ret;
	int len;
	char data[2];
	char blok1[16+1];
	char blok2[16+1];
	char dataXor[8+1];
	byte blok1Byte[8+1];
	byte blok2Byte[8+1];
	char tmk[16+1];
	char tpk[16+1];
	char dataKey[8+1];
	char tmkByte[8 + 1];
	char tpkByte[8 + 1];
	char dataPin[8+1];

	trcS("pmtPinManual Beg\n");
	MEM_ZERO(Pan);
	MEM_ZERO(pwd);
	MEM_ZERO(blok1);
	MEM_ZERO(blok2);
	MEM_ZERO(dataXor);
	MEM_ZERO(tmk);
	MEM_ZERO(tpk);
	MEM_ZERO(tmkByte);
	MEM_ZERO(tpkByte);

	MAPGET(traPan, Pan, lblKO);

	dspClear();
	dspLS(2,"INPUT PIN ?");
	ret = enterPwd(3, pwd, 7);
    	CHECK(ret >= 0, lblKO);
    	CHECK(ret == kbdVAL, lblEnd);
	len = strlen(pwd);
	num2dec(data, len, 2);
	fmtPad(data, -2, 0x30);
	memcpy(blok1, data, 2);
	memcpy(blok1 + 2, pwd, len);
	fmtPad(blok1, 16, 0x46);

	memcpy(blok2, "0000", 4);
	memcpy(blok2 + 4, Pan + 3, 12);
       hex2bin(blok1Byte, blok1, 8);
	hex2bin(blok2Byte, blok2, 8);
	xor(blok1Byte, blok2Byte, dataXor, 8);

       memcpy(dataKey,"\x11\x11\x11\x11\x11\x11\x11\x11", 8);

	stdDES(dataPin, dataXor, dataKey);		//encrypt
	ret = mapPut(traPinBlk, dataPin, lenPinBlk);
       CHECK(ret >= 0, lblKO);

	return ret;
	lblKO:
	lblEnd:
	return -1;
}

int debug_set()
{
	char buf[2];
	int ret;
	memset(buf,0,sizeof(buf));
	dspClear();
	dspLS(1, "DEBUG REQUEST");
	dspLS(2, "1 - ON, 0 - OFF");
	ret = enterStr(3, buf, 2);
	if(buf[0] == 0x31)
	{	debugReqFlag = 1;
		dspClear();
		dspLS(1, "    DEBUG ON    ");
	}
	else
	{	debugReqFlag = 0;
		dspClear();
		dspLS(1, "    DEBUG OFF   ");
	}
	tmrPause(1);
	memset(buf,0,sizeof(buf));
	dspClear();
	dspLS(1, "DEBUG RESPONSE");
	dspLS(2, "1 - ON, 0 - OFF");
	ret = enterStr(3, buf, 2);
	if(buf[0] == 0x31)
	{	debugRspFlag = 1;
		dspClear();
		dspLS(1, "    DEBUG ON    ");
	}
	else
	{	debugRspFlag = 0;
		dspClear();
		dspLS(1, "    DEBUG OFF   ");
	}
	tmrPause(1);
	return 1;
}

char acceptable_key(int accept_key, int timeout)
	{
		char key, key1, key2;
		int tOut;

		kbdStart(1);

		tmrStart(0, timeout * 100); //60 seconds timeout for user reaction
		kbdStart(1);

		switch(accept_key)
		{
			case kbdF1_kbdF4:
				key1 = kbdF1;
				key2 = kbdF4;
				break;

			case kbdVAL_kbdANN:
				key1 = kbdANN;
				key2 = kbdVAL;
				break;
		}

		tOut = tmrGet(0);

		while(tOut != 0)
		{
			tOut = tmrGet(0);
			key = kbdKey(); 	//retrieve the key pressed if any
			if(key == key1 || key == key2)
				break;		//quit the loop if the key is pressed
		}

		kbdStop();
		tmrStop(0);

		if (tOut == 0)
		{
			usrInfo(infConnTimeout);
			//Beep();
			return key1; // cancel
		}

		return key;
	}

int display_text(int line, int y, char *text, int alignment) {
	int ret, col;
	int wdt = 21;

	font_size(6, 8);
	switch(alignment)
	{
		case dsp_ALIGN_LEFT://left
		gotoxy(line, y); ret = printf(text); CHECK(ret>=0, lblKO);
		break;

		case dsp_ALIGN_CENTER://center
		col = (wdt - strlen(text)) / 2;
		gotoxy(line, col + y); ret = printf(text); CHECK(ret>=0, lblKO);
		break;

		case dsp_ALIGN_RIGHT://right
		col = wdt - strlen(text);
		gotoxy(line, col); ret = printf(text); CHECK(ret>=0, lblKO);
		break;
	}

	return ret;
	lblKO:
		return -1;
}


	/*
	**	@author:	EQ
	**	@date:		2009-09-29
	**	@version:	1.0
	**	@desc:		Get key pressed with timeout
	**
	**	@param		time out in second
	**
	*/
char udf_get_key_press(int timeout)
	{
		char key;
		int tOut;

		kbdStart(1);

		tmrStart(0, timeout * 100); //60 seconds timeout for user reaction
		kbdStart(1);

		tOut = tmrGet(0);

		while(tOut != 0)
		{
			tOut = tmrGet(0);
			key = kbdKey(); 	//retrieve the key pressed if any
			if(key) break;		//quit the loop if the key is pressed
		}

		kbdStop();
		tmrStop(0);

		if (tOut == 0)
		{
			usrInfo(infConnTimeout);
			Beep();
			return '\0';
		}

		return key;
	}

void PrintInit()
{
    int ret;
    byte counter = 0;
    char AcqName[lenName + 1];
    char Aid[lenAID + 1];

	ret = rptReceipt(rloInitHeader);
    	CHECK(ret >= 0, lblKO);
    	for (counter = 0; counter < dimAcq; counter++) {
        mapMove(acqBeg, counter);
        MAPGET(acqName, AcqName, lblKO);
        if(AcqName[0] == 0)
            break;
        ret = rptReceipt(rloInitBody);
        CHECK(ret >= 0, lblKO);
    }
    ret = rptReceipt(rloInitEmvHeader);
    CHECK(ret >= 0, lblKO);
    for (counter = 0; counter < dimAID; counter++) {
        mapMove(aidBeg, counter);
        MAPGET(emvAid, Aid, lblKO);
        if(Aid[0] == 0)
            break;
        ret = rptReceipt(rloInitEmvBody);
        CHECK(ret >= 0, lblKO);
    }
	ttestall(0,300);
	lblKO:
		;
}

void isIct220()
{
	char buf[2];
	int ret;
	memset(buf,0,sizeof(buf));
	dspClear();
	dspLS(1, "ICT 220 SET");
	dspLS(2, "1 - ON, 0 - OFF");
	ret = enterStr(3, buf, 2);
	if(buf[0] == 0x31)
	{
		MAPPUTBYTE(appICT220, 1, lblEnd)
		dspClear();
		dspLS(1, "   ICT 220 ON   ");
	}
	else
	{
		MAPPUTBYTE(appICT220, 0, lblEnd)
		dspClear();
		dspLS(1, "   ICT 220 OFF  ");
	}
	tmrPause(1);
	lblEnd:
		;

}

void PrintBinAndIssuerTable() //@@AS0.0.32 - debug only
{
    int ret;
    byte binId,IssId;
    byte BinLO[lenBinPrefix + 1],BinHI[lenBinPrefix + 1],prtbuf[40];
    char localBinName[lenName + 1];
    byte IssuerID;

	prtS("---BIN TABLE---");
    for (binId = 0; binId < dimBin; binId++)
	{  // Analyse BIN multi records
		ret = mapMove(binBeg, (word) binId);
		CHECK(ret >= 0, lblDBA);    // Move to the next record
		MAPGET(binName, localBinName, lblDBA);
		if (strlen(localBinName)>0) {
			prtS(localBinName);
			MAPGET(binLo, BinLO, lblDBA);
			MAPGET(binHi, BinHI, lblDBA);
			sprintf(prtbuf,"%02X%02X%02X%02X%02X-%02X%02X%02X%02X%02X",
					BinLO[1],BinLO[2],BinLO[3],BinLO[4],BinLO[5],
					BinHI[1],BinHI[2],BinHI[3],BinHI[4],BinHI[5]);
			prtS(prtbuf);
		}
	}

    prtS("---ISSUER TABLE---");
    for (IssId = 0; IssId < dimIss; IssId++)
   	{  // Analyse BIN multi records
   		ret = mapMove(issBeg, (word) IssId);
   		CHECK(ret >= 0, lblDBA);    // Move to the next record
   		MAPGETBYTE(issID, IssuerID, lblDBA);
   		sprintf(prtbuf,"IssuerID=%d",IssuerID);
   		prtS(prtbuf);
   		MAPGETBYTE(issPinOpt, IssuerID, lblDBA);
   		sprintf(prtbuf,"issPinOpt=%d",IssuerID);
   		prtS(prtbuf);
   		MAPGET(issRef,prtbuf,lblDBA);
   		prtS(prtbuf);
   	}
    goto lblEnd;
    lblDBA:
    	usrInfo(infDataAccessError);
    lblEnd:
    	;
}

void AcquirerInfo()
{
	int counter = 0;
	int ret;
	char AcqName[lenName + 1];
	char prim_ip[32+1];
	char Nii[lenNII + 1];
	char TID[lenTid + 1];
	char MID[lenMid + 1];
	char key;

	while(counter < dimAcq)
	{
	       mapMove(acqBeg, counter);
	       MAPGET(acqName, AcqName, lblKO);
	        if(AcqName[0] == 0)
	           break;
	       MAPGET(acqPriTxnIPSrv, prim_ip, lblKO);
		MAPGET(acqNII, Nii, lblKO);
		MAPGET(acqTID, TID, lblKO);
		MAPGET(acqMID, MID, lblKO);
	       dspClear();
		display_text(0, 0, 	AcqName, 		dsp_ALIGN_CENTER);
		display_text(1, 0, 	"IP:", 			dsp_ALIGN_LEFT);
		display_text(2, 0, 	prim_ip, 			dsp_ALIGN_LEFT);
		display_text(3, 0, 	"NII:", 			dsp_ALIGN_LEFT);
		display_text(3, 4, 	Nii, 				dsp_ALIGN_LEFT);
		display_text(4, 0, 	"TID:", 			dsp_ALIGN_LEFT);
		display_text(4, 4, 	TID,				dsp_ALIGN_LEFT);
		display_text(5, 0, 	"MID:", 			dsp_ALIGN_LEFT);
		display_text(5, 4, 	MID, 			dsp_ALIGN_LEFT);
		display_text(7, 0, 	"Next", 			dsp_ALIGN_RIGHT);
		key = acceptable_key(kbdF1_kbdF4, 60);
		if(key == kbdF1)
			counter--;
		else
			counter++;
    }

	lblKO:
		;
}

void CetakMasterKey()
{
	int counter = 0;
	int ret;
	char AcqName[lenName + 1];
	char MKey[lenMKey + 1];
	//char key;

	while(counter < dimAcq)
	{
	       mapMove(acqBeg, counter);
	       MAPGET(acqName, AcqName, lblKO);
	        if(AcqName[0] == 0)
	           break;
	      // MAPGET(acqMasterKey, MKey, lblKO);
		hex_dump_char("MKey", MKey, lenMKey);
    }

	lblKO:
		;
}

void hexDumpTMS(void)
{
	char TACDft[6+1];
	char TACDen[6+1];
	char TACOnl[6+1];

	char TACDfth[10+1];
	char TACDenh[10+1];
	char TACOnlh[10+1];
	int counter = 0;
	char key;
	int ret;

	while(counter < dimAID)
	{
		MEM_ZERO(TACDft);
		MEM_ZERO(TACDen);
		MEM_ZERO(TACOnl);

       	mapMove(aidBeg, counter);
		MAPGET(emvTACDft, TACDft, lblKO);
		MAPGET(emvTACDen, TACDen, lblKO);
		MAPGET(emvTACOnl, TACOnl, lblKO);

		bin2hex(TACDfth, TACDft+1, 5);
		bin2hex(TACDenh, TACDen+1, 5);
		bin2hex(TACOnlh, TACOnl+1, 5);

		dspClear();
		display_text(1, 0, 	"TAC Default:", 	dsp_ALIGN_LEFT);
		display_text(2, 0, 	TACDfth, 			dsp_ALIGN_LEFT);
		display_text(3, 0, 	"TAC Denial:",		dsp_ALIGN_LEFT);
		display_text(4, 0, 	TACDenh, 			dsp_ALIGN_LEFT);
		display_text(5, 0, 	"TAC Online:",  	dsp_ALIGN_LEFT);
		display_text(6, 0, 	TACOnlh,			dsp_ALIGN_LEFT);
		display_text(7, 0, 	"Next", 			dsp_ALIGN_RIGHT);
		key = acceptable_key(kbdF1_kbdF4, 60);
		if(key == kbdF1)
			counter--;
		else
			counter++;
    }

	lblKO:
		;

	}


int CheckFallBack(int ret)
{
unsigned char FallBack;

switch(ret) //Bao add
		{
		case TAG_PROCESS_COMPLETED:
		case TAG_PROCESSING_ERROR: //17	//@@SIMAS-NOT_FALLBACK
			break;
	    case TAG_CARD_ERROR:
	    case TAG_CDA_FAILED:
	    case TAG_NOT_ACCEPTED:
	    case TAG_ICC_DATA_FORMAT_ERROR:
	    case TAG_MISSING_MANDATORY_ICC_DATA:
	    case TAG_ICC_REDUNDANT_DATA:
	    case TAG_MISSING_MANDATORY_TERM_DATA:
	    case TAG_INTERNAL_ERROR:
	    case TAG_SERVICE_NOT_ALLOWED:
	    case TAG_NOT_ENOUGH_MEMORY:
	    case TAG_WRONG_DEL_INPUT_PARAMETER:
			mapGetByte(traFallback, FallBack);
			if(emvGetFallBack() || (FallBack == 1))
			{
				return 1;
			}
		default:
			return 0;
		}
	return 0;
}

#if 0
static int testDial(byte ctl) {
    const char *ptr;

    ptr = 0;
    switch (ctl) {
      case 0:
          ptr = "192.168.0.1|5030";
          break;
      case 1:
          ptr = "10.10.59.27|2000";
          break;
      case 2:
          ptr = "82.247.161.69|6789";
          break;
      default:
          break;
    }
    VERIFY(ptr);
    return comDial(ptr);
}
#endif

void pmtEchoTest(void) {
	int ret;
	char traName[dspW + 1];


	trcS("pmtEchoTest Beg\n");

//    ret = selectAcquirer();
//    if(ret <= 0)
//        goto lblKO;

    mapMove(acqBeg,0);

	mapMove(rqsBeg, (word) (trtEchoTest- 1));
	MAPGET(rqsDisp, traName, lblKO);
	trcFS("traName: %s\n", traName);

    MAPPUTBYTE(regLocType,'T',lblKO);
	MAPPUTSTR(traTransType, traName, lblKO);
	MAPPUTSTR(traTypeName, traName, lblKO);
	MAPPUTBYTE(traTxnType, trtEchoTest, lblKO);
	MAPPUTBYTE(traTxnId, rqtEchoTest, lblKO);

	ret = onlSession();
	CHECK(ret > 0, lblKO);

	ret = valRspCod();
	CHECK(ret > 0, lblKO);

	ret = 1;
	goto lblEnd;

lblKO:
    return;
lblEnd:
    trcS("pmtEchoTest End\n");

}


void PrintIsoMsg()
{
	int ret;
	char req[ISO_BUFFER_LEN];
	word lenreq;
	word lenrsp;
	char rsp[ISO_BUFFER_LEN];
	char buf[2];

	ret = valAskPwd(appPassTel);
	CHECK((ret > 0) && (ret != kbdANN), lblEnd);

		memset(buf,0,sizeof(buf));
		dspClear();
		dspLS(1, "0 -> RESPONSE");
		dspLS(2, "1 -> REQUEST");
		ret = enterStr(3, buf, 2);
		CHECK(ret != kbdANN, lblEnd);
		if(buf[0] == 0x31)
		{
			MAPGET(appRequest, req, lblKO);
			MAPGETWORD(appReqLen, lenreq, lblKO);
			hex_dump_char("Request", req, lenreq);
		}
		else
		{	MAPGET(appRespon, rsp, lblKO);
			MAPGETWORD(appRspLen, lenrsp, lblKO);
			hex_dump_char("Response", rsp, lenrsp);
		}
		tmrPause(1);

	goto lblEnd;
	lblKO:
		usrInfo(infDataAccessError);
	lblEnd:
		trcS("tes");
}


int debug_emv_set()
{
	char buf[2];
	int ret;
	memset(buf,0,sizeof(buf));
	dspClear();
	dspLS(1, "DEBUG EMV");
	dspLS(2, "1 - ON, 0 - OFF");
	ret = enterStr(3, buf, 2);
	if(buf[0] == 0x31)
	{	debugEmvFlag = 1;
		dspClear();
		dspLS(1, "    DEBUG ON    ");
	}
	else
	{	debugEmvFlag = 0;
		dspClear();
		dspLS(1, "    DEBUG OFF   ");
	}
	tmrPause(1);

	return 0;
}

void ICCDebug()
{
	rptReceipt(rloEmvDebug);
	ttestall(0, 300);
}



void functionMenu()
{
	char 	mnu[MNUMAX][dspW + 1]; 
	char 	*ptr[MNUMAX];         
	int 		sta=0;  
	int 		ret;

	strcpy(mnu[0],   "Debug Menu");
	strcpy(mnu[1],   "Test Koneksi");
	strcpy(mnu[2],   "Cetak Acquirer");
 	strcpy(mnu[3],   "Info Aquireer");
 	strcpy(mnu[4],   "Debug EMV");
	strcpy(mnu[5],   "Download Key");
	strcpy(mnu[6],   "Logon");
	strcpy(mnu[7],   "Debug Iso Msg");
	strcpy(mnu[8],   "Debug Prepaid");
	strcpy(mnu[9],   "Load Parameter");
	strcpy(mnu[10], "SPV Menu");
	strcpy(mnu[11], "Merchant Menu");
	strcpy(mnu[12], "Disable Menu");
	strcpy(mnu[13], "Set TID");
	strcpy(mnu[14], "Cetak Versi");

	memset(ptr,0,sizeof(ptr));
	ptr[0]   = mnu[0];
	ptr[1]   = mnu[1];
	ptr[2]   = mnu[2];
	ptr[3]   = mnu[3];
	ptr[4]   = mnu[4];
	ptr[5]   = mnu[5];
	ptr[6]   = mnu[6];
	ptr[7]   = mnu[7];
	ptr[8]   = mnu[8];
	ptr[9]   = mnu[9];
	ptr[10] = mnu[10];
	ptr[11] = mnu[11];
	ptr[12] = mnu[12];
	ptr[13] = mnu[13];

	ret = mnuSelect((Pchar *) ptr, sta, 60);    

	CHECK(ret >= 0, lblEnd);

	if(!ret)
	{                  
		goto lblEnd;
	}

	ret %= MNUMAX;            

	switch(ret)
	{
		case 1 :
			pmtEchoTest();
			break;

		case 2 :
			PrintInit();
			break;

		case 3 :
			AcquirerInfo();
			break;

		case 4 :
			debug_emv_set();
			break;
#ifdef LINE_ENCRYPTION
		case 5 :
			tleMenu();
			break;
#endif
		case 6 :
			debitLogon(0,1);
			break;

		case 7 :
			setDumpIsoMessage();
			break;

		case 8 :
			setDebugData();
			break;

		case 9 :
			FnTMS();
			break;

		case 10 :
			Fn99();
			break;
//#ifdef TEST_BRI
//#ifdef PREPAID
		case 11 :
			 Fn100();
			break;
//#endif
//#endif
		case 12 :
			FnDisableMenu();
			break;

		case 13 :
			FnSettingTID();
			break;

		case 14 :
			cetakVersion();
			break;

		
	}

	lblEnd:
		return;
}


void settingMenu()
{
	char 			mnu[MNUMAX][dspW + 1]; 
	char 			*ptr[MNUMAX];         
	int 				sta=0;  
	int 				ret;


	strcpy(mnu[0],"Setting Menu");
	strcpy(mnu[1],"Load Parameter");
	strcpy(mnu[2],"Download Key");
	strcpy(mnu[3],"Gprs Setup");
	strcpy(mnu[4],"Set Koneksi");
	strcpy(mnu[5],"Set Tanggal");
	strcpy(mnu[6],"Cetak Acquirer");
	strcpy(mnu[7],"Cetak Versi");

	memset(ptr,0,sizeof(ptr));
	ptr[0]   = mnu[0];
	ptr[1]   = mnu[1];
	ptr[2]   = mnu[2];
	ptr[3]   = mnu[3];
	ptr[4]   = mnu[4];
	ptr[5]   = mnu[5];
	ptr[6]   = mnu[6];
	ptr[7]   = mnu[7];

	ret = mnuSelect((Pchar *) ptr, sta, 60);    

	CHECK(ret >= 0, lblEnd);

	if(!ret)
	{                  
		goto lblEnd;
	}

	ret %= MNUMAX;            

	switch(ret)
	{
		case 1 :
			FnTMS();
			break;
#ifdef LINE_ENCRYPTION
		case 2 :
			tleMenu();
			break;
#endif
		case 3 :
			FnCommunication();
			break;

		case 4 :
			FnConnSetting();
			break;

		case 5 :
			FnTerminal();
			break;

		case 6 :
			PrintInit();
			break;

		case 7:
			cetakVersion();
			break;
}

	lblEnd:
		return;
}

void call_function(int nbr_func)
{
	int 	ret;
	char tcPwd1[lenPwd + 1];
	
	switch(nbr_func)
	{
		case 0:
			pmtEchoTest();
			break;
		case 1:
			PrintInit();
			break;
//		case 2:
//			hexDumpTMS();
//			break;
//		case 3:
//			PrintIsoMsg();
//			break;
//		case 4:
//			debug_set();
//			break;
//		case 5:
//			CetakMasterKey();
//			break;
//		case 6:
//			debug_emv_set();
//			break;
//		case 7:
//			ICCDebug();
//			break;
		case 2:
			AcquirerInfo();
			break;
#if 0
		case 10:
			{
				byte pin[20],pinbcd[20],pan[20],ePin[20];
				byte mk[16],wk[16];
				int retval,pinlen;

				memcpy(mk,"1234567887654321",16);
				memcpy(wk,"1122334455667788",16);
				memcpy(pan,"\x45\x41\x79\x00\x26\x10\x70\x07",8);

				if (valAskPin(pin) == kbdVAL) {
					pinlen=strlen(pin);
					if ((pinlen % 2) != 0)
						strcat(pin,"F");
					hex2bin(pinbcd,pin,strlen(pin)/2);
					retval=GetEncryptedPinBlock(pinbcd,pinlen,mk,wk,16,pan,8,ePin);
				}
			}

			break;
#endif
		//case 11:
#ifdef TEST_BRI
		  //  testScrollDisplay8();
#endif
		    //break;

		case 37: //@@AS0.0.20
			debug_emv_set();
			break;

#ifdef LINE_ENCRYPTION
		case 80:
		    tleMenu();
//			tleProcess(0);
			break;
#endif //LINE_ENCRYPTION

		case 81:
			debitLogon(0,1);
			break;

#ifdef DUMP_ISO_MESSAGE
	    case 93:
			setDumpIsoMessage();
			break;
#endif

#ifdef DEBUG_PREPAID_DATA
		case 94:
		    setDebugData();
		    break;
#endif

#ifdef BRIZI_DISCOUNT
		case 88:
			setDiscount();
			break;
#endif

		case 95:
		FnTerminal();
		break;
		case 96:
		FnTMS();
		break;
		case 97:
		FnCommunication();
		break;
		case 98:
		FnConnSetting();
		break;
		case 99:
		Fn99();
		break;


		case 101:
			req4Key(); // inject mkwk
			break;
		case 291:
			PrintBinAndIssuerTable(); //@@AS0.0.32
			break;


#ifdef TEST_BRI
          case 30:
            testReprintLastMiniAtm();
            break;
          case 40:
            testReprintMiniAtm();
            break;
          case 50:
            testPrintDetail();
            break;
//          case 60:
//            viewCardServiceLog();
//          	break;
//          case 61:
//        	viewMiniAtmLog();
//          	break;
//          case 62:
//        	viewTunaiLog();
//          	break;
          case 70:
        	testPrintTotal();
          	break;
          case 71:
        	  testPrintTotalHariIni();
          	break;
          case 72:
        	testPrintTotalHariSemua();
          	break;
          case 90:
            testBacaLogSemua();
            break;
#ifdef PREPAID
		  case 110:
		    testPrepaidInit();
		    break;
		  case 111:
		    testInfoSaldo();
		    break;
		  case 112:
		    testPrepaidSale();
		    break;

#endif
#endif //TEST_BRI

		  case 100:
		  	Fn100();
		  	break;

		case 233:
			FnDisableMenu();
			break;

		case 33:
			FnSettingTID();
			break;

		case 44:
			cetakVersion();
			break;

		case 22:
			memset(tcPwd1, 0, sizeof(tcPwd1));
			
			ret = usrInfo(infPwdInput);  
			ret = enterPwd(2, tcPwd1, lenPwd + 3);
			if(strcmp(tcPwd1, "000000") == 0)
				functionMenu(); 
			else
				usrInfo(infWrongPin);    
			
			break;

		case 11:
			memset(tcPwd1, 0, sizeof(tcPwd1));
			
			ret = usrInfo(infPwdInput);  
			ret = enterPwd(2, tcPwd1, lenPwd + 3);
			if(strcmp(tcPwd1, "000000") == 0)
				settingMenu();
			else
				usrInfo(infWrongPin);     

			
			
			break;

//		  case 10:
//		    viewLog();
//		    break;
	}
}

int displayResponse(void){
     char RspCod[lenRspCod + 1];
     byte txnType;
     int ret;
	 char buf[20];

	dspClear();

	 MAPGET(traRspCod, RspCod, lblKO);
	 CHECK(ret >= 0, lblKO);

	MAPGETBYTE(traTxnType,txnType,lblKO);
	switch(txnType)
	{
	    case trtEchoTest:
	        if(strcmp(RspCod,"00") == 0)
	        {
	            usrInfo(infTestKoneksiBerhasil);
	        }
	        break;
		case trtTransferAntarBankInquiry:
			if(strcmp(RspCod, "68") == 0)
            {
            	usrInfo(infRsp68Transfer);
                return 0;
            }break;
		case trtTransferAntarBank:
			if(strcmp(RspCod, "68") == 0)
            {
            	usrInfo(infRsp68Transfer);
                return 9999;
            }break;
		case trtReissuePIN:
	        if(strcmp(RspCod, "60") == 0)
	        {
	 	        usrInfo(infRsp60RekeningSalah);
	 	        return 0;
	 	    }
		case trtGantiPIN:
		case trtGantiPasswordSPV:
			if(strcmp(RspCod, "16") == 0)
            {
            	usrInfo(infRsp16CardServ);
                return 0;
            }else if(strcmp(RspCod, "IP") == 0)
            {
            	usrInfo(infRspIP);
                return 0;
            }else if(strcmp(RspCod, "09") == 0)
            {
            	usrInfo(infRsp09);
                return 0;
            }
            break;
	    case trtSetorPasti:
			if(strcmp(RspCod, "68") == 0)
            {
            	usrInfo(infRsp68Transfer);
                return 0;
            }else if(strcmp(RspCod, "06") == 0)
            {
            	usrInfo(infRsp06TV);
                return 0;
            }else if(strcmp(RspCod, "13") == 0)
            {
            	usrInfo(infInvalidAmount);
                return 0;
            }else if(strcmp(RspCod, "53") == 0)
            {
            	usrInfo(infRsp60RekeningSalah);
                return 0;
            }else if(strcmp(RspCod, "76") == 0)
            {
            	usrInfo(infRsp60RekeningSalah);
                return 0;
            }
		break;
	    case trtAktivasiKartuOtorisasi:
	    case trtRenewalKartuOtorisasi:
	    case trtReissuePINOtorisasi:
            if(strcmp(RspCod, "00") == 0)
            {
            // usrInfo(infOtorisasiBerhasil);
                return 9999;
            }else if(strcmp(RspCod, "IP") == 0)
            {
            	usrInfo(infRspIP);
                return 0;
            }else if(strcmp(RspCod, "09") == 0)
            {
            	usrInfo(infRsp09);
                return 0;
            }
            break;
	    case trtAktivasiKartuInquiry:
	    case trtRenewalKartuInquiry:
    	    if(strcmp(RspCod, "00") == 0)
                return 9999;
        case trtRenewalKartu:
        case trtAktivasiKartu:
            if(strcmp(RspCod, "NC") == 0)
            {
                usrInfo(infRenewalBerhasilPINGagal);
                return 1;
            }else if(strcmp(RspCod, "IP") == 0)
            {
            	usrInfo(infRspIP);
                return 0;
            }else if(strcmp(RspCod, "09") == 0)
            {
            	usrInfo(infRsp09);
                return 0;
            }
	        else if(strcmp(RspCod, "60") == 0)
	        {
	 	        usrInfo(infRsp60RekeningSalah);
                return 0;
	        }
            break;
	case trtTCash:
		break;
		/*Changed By Ali*/
		case trtRegistrasiSmsBanking:
		/*End Of Changed By Ali*/
        case trtRegistrasiInternetBanking:
        	if(strcmp(RspCod, "64") == 0)
            {
            	usrInfo(infRsp64IBank);
                return 0;
            }else if(strcmp(RspCod, "32") == 0)
            {
            	usrInfo(infRspRegSms);
                return 0;
            }
            break;
        case trtPembayaranCicilanInquiry:
        	if(strcmp(RspCod, "14") == 0)
        	{
        		usrInfo(infRsp14Cicilan);
        		return 0;
        	}
        	break;
    	case trtPembayaranPascaBayarInquiry:
		case trtPembayaranPascaBayar:
		case trtPembayaranPulsa:
        	if(strcmp(RspCod, "14") == 0)
        	{
        		usrInfo(infRsp14Pulsa);
        		return 0;
        	}
			else if(strcmp(RspCod, "03") == 0)
        	{
        		usrInfo(infRsp10Hp);
        		return 0;
        	}
			else if(strcmp(RspCod, "11") == 0)
        	{
        		usrInfo(infRsp11Hp);
        		return 0;
        	}
			else if(strcmp(RspCod, "12") == 0)
        	{
        		usrInfo(infRsp11Hp);
        		return 0;
        	}
			else if(strcmp(RspCod, "20") == 0)
        	{
        		usrInfo(infRsp20Hp);
        		return 0;
        	}
			else if(strcmp(RspCod, "21") == 0)
        	{
        		usrInfo(infRsp11Hp);
        		return 0;
        	}
        	break;
		
        case trtTarikTunai:
       	case trtInfoSaldo:
		case trtInfoSaldoBankLain:
			if(strcmp(RspCod, "55") ==0)
        	{
        		usrInfo(infRsp50);
        		return 0;
        	}
        	break;
		case trtMiniStatement:
        	if(strcmp(RspCod, "53") ==0)
	        	{
	        		usrInfo(infRsp53Tunai);
	        		return 0;
	        	}
			if(strcmp(RspCod, "13") == 0)
	            {
	                usrInfo(infRsp65);
	        		return 0;
	            }
        	break;

		case trtMutRek:
        	if(strcmp(RspCod, "53") ==0)
	        	{
	        		usrInfo(infRsp53Tunai);
	        		return 0;
	        	}
			if(strcmp(RspCod, "13") == 0)
	            {
	                usrInfo(infRsp65);
	        		return 0;
	            }
        	break;
		case trtPrevilege:
        	if(strcmp(RspCod, "53") ==0)
	        	{
	        		usrInfo(infRsp53Tunai);
	        		return 0;
	        	}
			if(strcmp(RspCod, "13") == 0)
	            {
	                usrInfo(infRsp65);
	        		return 0;
	            }
        	break;
		/*Changed By Ali*/
		case trtPembayaranTVBerlanggananInquiry:
		case trtPembayaranTVBerlangganan:
			break;

		case trtPembayaranTiketPesawatInquiry:
		case trtPembayaranTiketPesawat:
			if(strcmp(RspCod, "06") == 0)
	            {
	                usrInfo(infRsp06TV);
	        		return 0;
	            }
			if(strcmp(RspCod, "13") == 0)
	            {
	                usrInfo(infRsp13TV);
	        		return 0;
	            }
			if(strcmp(RspCod, "14") == 0)
	            {
	                usrInfo(infRsp14Psw);
	        		return 0;
	            }
			if(strcmp(RspCod, "80") == 0)
	            {
	                usrInfo(infRsp80Psw);
	        		return 0;
	            }
			if(strcmp(RspCod, "88") == 0)
	            {
	                usrInfo(infRspPswSb);
	        		return 0;
	            }
			 if(strcmp(RspCod, "26") == 0)
	            {
	                usrInfo(infRspPswSb);
	        		return 0;
	            }
			if(strcmp(RspCod, "89") == 0)
	            {
	                usrInfo(infRsp89TV);
	        		return 0;
	            }
			if(strcmp(RspCod, "68") == 0)
	            {
	                usrInfo(infRsp68TV);
	        		return 0;
	            }
			if(strcmp(RspCod, "96") == 0)
	            {
	                usrInfo(infRsp96TV);
	        		return 0;
	            }
			break;

		case trtPembayaranBrivaInquiry:
		case trtPembayaranBriva:
			if(strcmp(RspCod, "14") == 0)
	            {
	                usrInfo(infRspBrivaNf);
	        		return 0;
	            }
			if(strcmp(RspCod, "92") == 0)
	            {
	                usrInfo(infRspBrivaEx);
	        		return 0;
	            }
			if(strcmp(RspCod, "88") == 0)
	            {
	                usrInfo(infRspBrivaSb);
	        		return 0;
	            }
	        if((strcmp(RspCod, "05") == 0)||(strcmp(RspCod, "93")==0))
	            {
	                usrInfo(infRspBrivaCe);
	        		return 0;
	            }
			
			break;
		

		case trtPembayaranPdamInquiry:
			if(strcmp(RspCod, "14") == 0)
	            {
	                usrInfo(infRspPdam);
	        		return 0;
	            }
			if(strcmp(RspCod, "15") == 0)
	            {
	                usrInfo(infRspPdamSba);
	        		return 0;
	            }
			if(strcmp(RspCod, "88") == 0)
	            {
	                usrInfo(infRspPdamSb);
	        		return 0;
	            }
	        if((strcmp(RspCod, "Q1") == 0)||(strcmp(RspCod, "93")==0))
	            {
	                usrInfo(infRspPdamGagal);
	        		return 0;
	            }
			break;
			
		case trtPembayaranPdam:
		case trtPembayaranTelkomInquiry:
		case trtPembayaranTelkom:
			break;
		#ifdef SSB
		case trtPembayaranSSBInquiry:
		case trtPembayaranSSB:
			break;
		#endif
		case trtPembayaranDPLKR:
			break;
			
		case trtTrxPhoneBankingTransferInquiry:
		case trtTrxPhoneBankingTransfer:
			if(strcmp(RspCod, "RP") == 0)
	            {
	                usrInfo(infRspPBTRP);
	        		return 0;
	            }
			if(strcmp(RspCod, "NF") == 0)
	            {
	                usrInfo(infRspPBTNF);
	        		return 0;
	            }
			if(strcmp(RspCod, "64") == 0)
	            {
	                usrInfo(infRspPBT64);
	        		return 0;
	            }
	            if(strcmp(RspCod, "HP") == 0)
	            {
	                usrInfo(infRspPBTHP);
	        		return 0;
	            }
		break;
		case trtTrxPhoneBankingPulsa:
			if(strcmp(RspCod, "RP") == 0)
	            {
	                usrInfo(infRspPBTRP);
	        		return 0;
	            }
			if(strcmp(RspCod, "NF") == 0)
	            {
	                usrInfo(infRspPBTNF);
	        		return 0;
	            }
			if(strcmp(RspCod, "64") == 0)
	            {
	                usrInfo(infRsp53Tunai2);
	        		return 0;
	            }
	            if(strcmp(RspCod, "HP") == 0)
	            {
	                usrInfo(infRspPBTHP);
	        		return 0;
	            }
		break;
		case trtPembayaranKKBRIInquiry:
		case trtPembayaranKKBRI:
		case trtPembayaranKKANZ:
		case trtPembayaranKKCITIKK:
		case trtPembayaranKKCITIKTA:
		case trtPembayaranKKHSBCKK:
		case trtPembayaranKKHSBCKTA:
		case trtPembayaranKKRBSKK:
		case trtPembayaranKKRBSKTA:
		case trtPembayaranKKStanCharKK:
		case trtPembayaranKKStanCharKTA:
			if(strcmp(RspCod, "01") == 0)
	            	{
	                	usrInfo(infRspKKANZ01);
	        		return 0;
	            	}
			break;
		case trtPembayaranPLNPraInquiry: 
			if(strcmp(RspCod, "01") == 0)
	            {
	                usrInfo(infRspPLNPra1);
	        		return 0;
	            }
			else if(strcmp(RspCod, "03") == 0)
	            {
	                usrInfo(infRspPLNPra2);
	        		return 0;
	            }
	            else if(strcmp(RspCod, "04") == 0)
	            {
	                usrInfo(infRspPLNPra3);
	        		return 1;
	            }
			else if(strcmp(RspCod, "Q4") == 0)
			{
				usrInfo(infRspTimeOut);
				return 0;
			}
			break;
  		case trtPembayaranPLNPra:
  		case trtPembayaranPLNToken:
			if(strcmp(RspCod, "01") == 0)
	            {
	                usrInfo(infRspPLNPra1);
	        		return 0;
	            }
			else if(strcmp(RspCod, "02") == 0)
	            {
	            	MAPGET(traTotAmt, buf, lblKO);
					mapPut(traAmt,buf,lenInquiryAmt);
	                usrInfo(infRspCetTok);
	        		return 1;
	            }
	            if(strcmp(RspCod, "03") == 0)
	            {
	                usrInfo(infRspPLNPra2);
	        		return 0;
	            }
	            else if(strcmp(RspCod, "04") == 0)
	            {
	                usrInfo(infRspPLNPra3);
	        		return 1;
	            }
			else if(strcmp(RspCod, "Q4") == 0)
			{
				usrInfo(infRspTimeOut);
				return 0;
			}
		break;
		case trtAbsensi:
			if(strcmp(RspCod, "FE") == 0){
				usrInfo(infRspAbsenFE);
        			return 0;
        		}
        		else if(strcmp(RspCod, "BA") == 0){
				usrInfo(infRspAbsenBA);
        			return 0;
        		}
        		else if(strcmp(RspCod, "AE") == 0){
				usrInfo(infRspAbsenAE);
        			return 0;
        		}
		break;
		case trtPembayaranSPPInquiry:
		case trtPembayaranSPP:
				     if(strcmp(RspCod, "14") == 0)
            {
                usrInfo(infRsp14Cicilan);
        		return 0;
            }
					 if(strcmp(RspCod, "16") == 0)
            {
                usrInfo(infRsp17Univ);
        		return 0;
            }
					  if(strcmp(RspCod, "17") == 0)
            {
                usrInfo(infInvalidAmount);
        		return 0;
            }
					  if(strcmp(RspCod, "88") == 0)
            {
                usrInfo(infRspSppSb);
        		return 0;
            }
					  if(strcmp(RspCod, "89") == 0)
            {
                usrInfo(infSpTelat);
        		return 0;
            }
		break;
		/*End of Changed By Ali*/

#ifdef T_BANK
	case trtTCashOut:
		if(strcmp(RspCod, "00") == 0)
		{
			MAPPUTWORD(traMnuItm, mnuCashIn, lblKO);
			pmtBRI();
		}
		break;
#endif
        case trtPembayaranPLNPasca:
        case trtPembayaranPLNPascaInquiry:
	     if(strcmp(RspCod, "01") == 0)
            {
                usrInfo(infRsp04Pln);
        		return 0;
            }
	     if(strcmp(RspCod, "02") == 0)
            {
            	char buf[30];
                char str[30];
                card tmp;

                memset(buf,0,sizeof(buf));
                MAPGET(traRspBit13,buf,lblKO);
                memset(str,0,sizeof(str));
                bin2hex(str, buf, 1);
                tmp = (str[0]-0x30)*10 + (str[1]-0x30);
                memset(buf,0,sizeof(buf));
                convertBulan(1,buf,(byte)tmp);
                memset(str,0x20,16);
                memcpy(str+((16-strlen(buf))/2),buf,strlen(buf));
                dspLS(1,str);
                usrInfo(infRsp08Pln);
        	  return 0;
            }
            if(strcmp(RspCod, "06") == 0)
            {
                usrInfo(infRsp06Pln);
        		return 0;
            }
            if(strcmp(RspCod, "13") == 0)
            {
                usrInfo(infRsp13Pln);
        		return 0;
            }
	     if(strcmp(RspCod, "51") == 0)
            {
                usrInfo(infRsp51Pln);
        		return 0;
            }
            if(strcmp(RspCod, "68") == 0)
            {
                usrInfo(infRsp68PascaByr);
        		return 0;
            }
            if((strcmp(RspCod, "88") == 0) || (strcmp(RspCod, "54") == 0))
            {
                usrInfo(infRsp88);
        		return 0;
            }
            if(strcmp(RspCod, "89") == 0)
            {
                usrInfo(infRsp89PascaByr);
        		return 0;
            }
		default:
            break;
	}
#ifdef PREPAID
    if(isPrepaidTransaction(1) == 1)
    {
        if(strcmp(RspCod, "05") == 0)
            {usrInfo(infRspPrepaid05);return 0;}
        if(strcmp(RspCod, "NF") == 0)
            {usrInfo(infRspPrepaidNF);return 0;}
        if(strcmp(RspCod, "UI") == 0)
            {usrInfo(infRspPrepaidUI);return 0;}
        if(strcmp(RspCod, "62") == 0)
            {usrInfo(infRspPrepaid62);return 0;}
        if(strcmp(RspCod, "OV") == 0)
            {usrInfo(infRspPrepaidOV);return 0;}
        if(strcmp(RspCod, "ZB") == 0)
            {usrInfo(infRspPrepaidZB);return 0;}
        if(txnType == trtPrepaidRedeemInquiry)
        {
            if(memcmp(RspCod,"\x00\x00",2)==0)
                return 0;
        }
    }
#endif

	if (txnType==trtDebitLogon) //@@AS0.0.26 : no need to diplay response for logon
		return 0;

	if(strcmp(RspCod, "00") == 0)
	{
// @@OA - mulai
		if(isBRIInquiryTransaction() || txnType==trtTLEKeyDownload)
		    return 0;
// @@OA - selesai
		usrInfo(infApprove);
		return 1;
	}
	else if(strcmp(RspCod, "01") == 0)
	 	usrInfo(inf01Declined);
	else if(strcmp(RspCod, "02") == 0)
	 	usrInfo(inf02Eod);
	else if(strcmp(RspCod, "03") == 0)
	 	usrInfo(inf03InvMc);
	else if((strcmp(RspCod, "04") == 0) || (strcmp(RspCod, "09") == 0)|| (strcmp(RspCod, "40") == 0) || (strcmp(RspCod, "41") == 0) || (strcmp(RspCod, "43") == 0))
	 	usrInfo(inf04Pic);
	else if(strcmp(RspCod, "05") == 0)
	 	usrInfo(infRsp05);
	else if(strcmp(RspCod, "07") == 0)
	 	usrInfo(inf04Pic);
	else if(strcmp(RspCod, "08") == 0)
	 	usrInfo(inf08Sign);
	else if(strcmp(RspCod, "10") == 0)
		usrInfo(infRsp10);
	else if(strcmp(RspCod, "11") == 0)
		usrInfo(infRsp11Kta);
	else if(strcmp(RspCod, "12") == 0)
		usrInfo(inf12InvTra);
	else if(strcmp(RspCod, "13") == 0)
		usrInfo(inf13InvAmt);
	else if(strcmp(RspCod, "14") == 0)
		usrInfo(inf14CardNo);
	else if((strcmp(RspCod, "15") == 0) || (strcmp(RspCod, "62") == 0))
		usrInfo(infRsp15Cn);
	else if(strcmp(RspCod, "16") == 0)
		usrInfo(infRsp16Nf);
	else if(strcmp(RspCod, "19") == 0)
		usrInfo(inf19ReEntTra);
	else if(strcmp(RspCod, "21") == 0)
		usrInfo(inf21NoTra);
	else if(strcmp(RspCod, "23") == 0)
		usrInfo(infRsp23Btl);
	else if(strcmp(RspCod, "25") == 0)
	 	usrInfo(inf25InActCard);
	else if(strcmp(RspCod, "28") == 0)
		usrInfo(infRsp28Da);
	else if(strcmp(RspCod, "30") == 0)
		usrInfo(inf30FormatErr);
	else if(strcmp(RspCod, "31") == 0)
		usrInfo(inf31BankIdNF);
	else if(strcmp(RspCod, "34") == 0)
	 	usrInfo(infRsp34);
	else if(strcmp(RspCod, "38") == 0)
	 	usrInfo(infRsp38);
	else if(strcmp(RspCod, "43") == 0)
	 	usrInfo(inf43StolenCard);
	else if(strcmp(RspCod, "44") == 0)
	 	usrInfo(inf44PINChangeRe);
	else if(strcmp(RspCod, "45") == 0)
	 	usrInfo(inf45CardActUse);
	else if(strcmp(RspCod, "47") == 0)
	 	usrInfo(infRsp47Ibd);
	else if(strcmp(RspCod, "50") == 0)
	 	usrInfo(infRsp50);
	else if(strcmp(RspCod, "51") == 0)
	    usrInfo(inf51InsFund);
	else if(strcmp(RspCod, "52") == 0)
	    usrInfo(inf52NoCheckCo);
	else if(strcmp(RspCod, "53") == 0)
	    usrInfo(inf53NoSavingCo);
	else if(strcmp(RspCod, "54") == 0)
	 	usrInfo(inf54ExpCard);
	else if(strcmp(RspCod, "55") == 0)
	 	usrInfo(inf55InvPIN);
	else if(strcmp(RspCod, "56") == 0)
	 	usrInfo(inf56InvCard);
	else if(strcmp(RspCod, "57") == 0)
	 	usrInfo(infRsp57Rts);
	else if(strcmp(RspCod, "58") == 0)
	 	usrInfo(inf58InvTra);
	else if(strcmp(RspCod, "59") == 0)
	 	usrInfo(infRsp59);
	else if(strcmp(RspCod, "61") == 0)
	 	usrInfo(inf61OverLim);
	else if((strcmp(RspCod, "63") == 0)  || (strcmp(RspCod, "82") == 0))
	 	usrInfo(inf63SecVio);
	else if(strcmp(RspCod, "64") == 0)
        	usrInfo(infRspPBT64);
	else if(strcmp(RspCod, "68") == 0)
        	usrInfo(infRsp68Transfer);
	else if(strcmp(RspCod, "69") == 0)
        	usrInfo(infRsp69Fp);
	else if(strcmp(RspCod, "75") == 0)
        	usrInfo(inf75PinExc);
	else if(strcmp(RspCod, "76") == 0)
        	usrInfo(inf76InvDesc);
	else if(strcmp(RspCod, "77") == 0)
        	usrInfo(inf77RecErr);
	else if(strcmp(RspCod, "78") == 0)
        	usrInfo(inf78InvTrace);
	else if(strcmp(RspCod, "79") == 0)
        	usrInfo(inf79BatchOpen);
	else if(strcmp(RspCod, "80") == 0)
        	usrInfo(inf80InvBatchNo);
	else if(strcmp(RspCod, "85") == 0)
        	usrInfo(inf85BatchNoF);
    	else if(strcmp(RspCod, "88") == 0)
	 	usrInfo(inf88CallIssuer);
	else if((strcmp(RspCod, "89") == 0)|| (strcmp(RspCod, "96") == 0))
    		usrInfo(inf89InvTID);
	else if(strcmp(RspCod, "91") == 0)
    		usrInfo(inf91IssError);
	else if(strcmp(RspCod, "93") == 0)
    		usrInfo(inf93Declined);
	else if(strcmp(RspCod, "94") == 0)
    		usrInfo(inf94Duplicated);
	else if(strcmp(RspCod, "95") == 0)
    		usrInfo(inf95TotMis);
	else if(strcmp(RspCod, "96") == 0)
    		usrInfo(inf96SysMal);
	else if(strcmp(RspCod, "97") == 0)
    		usrInfo(infRspKrip);
	else if(strcmp(RspCod, "98") == 0)
    		usrInfo(infRsp98Limit);
	else if(strcmp(RspCod, "NF") == 0)
	    usrInfo(infRspPBTNF);
    	else if((strcmp(RspCod, "Q4") == 0) || (strcmp(RspCod, "q4") == 0))
		usrInfo(infRspQ4);
	else if((strcmp(RspCod, "Q1") == 0) || (strcmp(RspCod, "q1") == 0) || (strcmp(RspCod, "91") == 0) || (strcmp(RspCod, "94") == 0)|| (strcmp(RspCod, "96") == 0)|| (strcmp(RspCod, "97") == 0)|| (strcmp(RspCod, "99") == 0))
		usrInfo(infRspQ1);
	else
		usrInfo(infOnlineResult);

	return 0;

	lblKO:
		return -1;
}

void FmtExpDat(char *dst, char *src)//For format expdat 20100820
{
	char ExpDatFmt[lenExpDat + 1];

	memcpy(ExpDatFmt, src, 2);//For receipt
	ExpDatFmt[2] = '/';
	memcpy(ExpDatFmt + 3, src +2, 2);
	memcpy(dst, ExpDatFmt, 5);
}

#ifdef TEST_BRI
void testScrollDisplay8()
{
    int ret;
//    char str[128];
    char buf[128];//,buf1[128];
    char judul[30];
    char baris1[30];char baris2[30];
//    byte key;
    char msg[10][21];
    int i=0;

    memset(msg,0,sizeof(msg));
    strcpy(judul,"AKTIVASI KARTU");
    //nomor kartu
    memset(buf,0,sizeof(buf));
    strcpy(msg[i++],"Nomor Kartu:");
    strcpy(msg[i++],"1234567890123456");

    //nomor rekening
    memset(buf,0,sizeof(buf));
    mapGet(traNomorRekeningAsalBRI,buf,sizeof(buf));
    trimRight(buf,0x20);
    strcpy(msg[i++],"Nomor Rekening:");
    strcpy(msg[i++],"22223333444455566");

    //Nama nasabah
    memset(buf,0,sizeof(buf));
    mapGet(traNamaPenerima,buf,30);
    trimRight(buf,0x20);
    strcpy(buf,"NAMA NAMA NAMA NAMA  NAMA12345");
    ret = displayLongText(buf, strlen(buf), baris1, baris2, 20);
    strcpy(msg[i++],"Nama:");
    strcpy(msg[i++],baris1);
    if(ret == 2)
        strcpy(msg[i++],baris2);

    //Tanggal lahi
    memset(buf,0,sizeof(buf));
    mapGet(traTanggalLahir,buf,lenTanggalLahir);
    strcpy(msg[i++],"Tgl Lahir:");
    strcpy(msg[i++],"01012000");

    ret = scrollDisplay8(msg,i,6,judul,"BATAL      LANJUT");
}
#endif

