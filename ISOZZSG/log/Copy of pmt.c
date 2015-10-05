//Subversion reference
//$HeadURL: https://svn.ingenico.com/UTAPROJECTS/sea-partner/5W/trunk/ISO5WPH/log/pmt.c $
//$Id: pmt.c 1987 2009-05-06 05:56:47Z ajloreto $

/** \file
 * Financial (payment) operations transaction flow is implemented here 
 *
 * All of them follow the return code convention:
 *    - >0 : operation OK, continue
 *    -  0 : user cancel or timeout: smooth exit
 *    - <0 : irrecoverable error, stop processing anyway
 */

#include "log.h"
#include <string.h>
#include <stdlib.h>
#include "stdio.h"
#include <bri.h> //@agmr BRI
#include "prepaid.h"

static byte UseChip = 0; // Bao add : 1 indicate must be use chip card first
extern byte txnAsli; //@agmr BRI2

/** Detect if the transaction is supposed to be forced on line.
 * 
 * \header log\\log.h
 * \source log\\pmt.c
 */
int pmtForceOnline(void)
{
	trcS("pmtForceOnline Beg\n");
	return 0;                   //to be implemented
}

/** Perform the Manual Card input
 *  Processes Manual Entry of Card Number
 * 
 * \header log\\log.h 
 * \source log\\pmt.c
 */
int pmtManInput(void)
{
	char Pan[lenPan + 1];
	char Trk2[lenTrk2 + 1];
	char ExpDat[lenExpDat + 1];
	char Buf[128 * 3];          // Buffer to accept the input
	char tmp[dspW * 2 + 1];
	int ret;

	trcS("pmtManInput Beg\n");
	memset(Pan, 0, lenPan + 1);
	memset(Trk2, 0, lenTrk2 + 1);
	memset(ExpDat, 0, lenExpDat + 1);
	memset(Buf, 0, sizeof(Buf));

	ret = usrInfo(infPanInput);
	CHECK(ret >= 0, lblKO);
	ret = enterStr(2, Buf, lenPan + 1);
	CHECK(ret >= 0, lblKO);
	if(ret != kbdVAL)
	{
		goto lblEnd;
	}                           // Cancel, timeout => exit
	strcpy(Pan, Buf);

	memset(Buf, 0, sizeof(Buf));    // ** Expiration date input **
	ret = usrInfo(infExpDatInput);
	CHECK(ret >= 0, lblKO);     // Msg ExpDatInput on screen
	ret = enterStr(2, Buf, lenExpDat + 1);
	CHECK(ret >= 0, lblKO);     // Exp data entry
	if(ret != kbdVAL)
	{
		goto lblEnd;
	}                           // Cancel, timeout => exit
	ExpDat[0] = Buf[2];         // Format expiration date
	ExpDat[1] = Buf[3];         //YY
	ExpDat[2] = Buf[0];
	ExpDat[3] = Buf[1];         //MM

	MAPPUTSTR(traPan, Pan, lblDBA);
	MAPPUTSTR(traExpDat, ExpDat, lblDBA);
	strcpy(tmp, Pan);
	strcat(tmp, "=");
	strcat(tmp, ExpDat);
	ret = mapPut(traTrk21, tmp, dspW);
	CHECK(ret >= 0, lblDBA);
	MAPPUTSTR(traTrk22, tmp + dspW, lblDBA);

	ret = kbdVAL;
	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	ret = 0;
	goto lblEnd;
lblDBA:
	usrInfo(infDataAccessError);
	ret = 0;
	goto lblEnd;
lblEnd:
	trcFN("pmtManInput: ret=%d\n", ret);
	return ret;
}

/** Perform the card input
 *  Waits for card input then processes depending on the entry mode.
 * 
 * \header log\\log.h 
 * \source log\\pmt.c
 */
int pmtCardInput(void)
{
	char EntMod;
	char CrdInpCtl[lenCrdInpCtl + 1];   // Control settings for card input
	char Pan[lenPan + 1];
	char Trk2[lenTrk2 + 1];
	char Trk1[128 + 1];
	char CardHolderName[40 + 1];
	char ExpDat[lenExpDat + 1];
	char Buf[128 * 3];          // Buffer to accept the input
	char *pTrk2;
	char FirstKey;
	char tmp[dspW * 2 + 1];
	int ret;
	byte Fallback;
	char Srvcod[3+1];
	char BlockedPan[lenPan + 1];
	int Fallbackret = 0;
	word inf;
	byte txnType; //@agmr - BRI
	

	trcS("pmtCardInput Beg\n");
	memset(Pan, 0, lenPan + 1);
	memset(Trk2, 0, lenTrk2 + 1);
	memset(ExpDat, 0, lenExpDat + 1);
	memset(Buf, 0, sizeof(Buf));
	memset(Trk1, 0, sizeof(Trk1));
	memset(CardHolderName, ' ', sizeof(CardHolderName));
	memset(BlockedPan, '*', sizeof BlockedPan);

	MAPGETBYTE(traTxnType,txnType,lblKO);
	
//++@agmr - prepaid
#ifdef PREPAID
    if(isPrepaidTrans(txnType) == 1)
        goto lblEnd;
#endif //PREPAID
//--@agmr - prepaid	
	
	MAPGETBYTE(traFallback,Fallback,lblKO);
	if(Fallback == 1)
		inf = infCardInputFB;
	else
	{
//+++ @agmr - BRI 
	    if(txnType == trtAktivasiKartuOtorisasi ||
	       txnType == trtRenewalKartuOtorisasi ||
	       txnType == trtReissuePINOtorisasi
	       )
	        inf = infCardPengawasInput;
	    else if (
	        txnType == trtAktivasiKartuInquiry ||
//	        txnType == trtRenewalKartuInquiry ||
	        txnType == trtReissuePIN)
	        inf = infCardNasabahInput;
	    else if(txnType == trtRenewalKartuInquiry)
	        inf = infCardNasabahLamaInput;
	    else if(txnType == trtRenewalKartu)
	        inf = infCardNasabahBaruInput;
	    else  
//--- @agmr - BRI	          
		    inf = infCardInput;
	}

	ret = usrInfo(inf);
	CHECK(ret >= 0, lblKO);
	MAPGET(rqsEntMod, CrdInpCtl, lblKO);
	trcFS("CrdInpCtl: %s\n", CrdInpCtl);

	//Bao add for swipe card only when fallback
	if((emvGetFallBack() != 0)||(Fallback == 1)) 
		{
		memset(CrdInpCtl,0,sizeof(CrdInpCtl));
		Fallbackret = 1;
		CrdInpCtl[0] = 'm';
		CrdInpCtl[1] = 'k';
		emvSetFallBack(0);
		MAPPUTBYTE(traFallback, 0,lblKO);
		}
	//End

	//ret = enterCard((byte *) Buf, CrdInpCtl);   // Process card input and return entry mode
	ret = getCard((byte *) Buf, CrdInpCtl);					
	CHECK(ret >= 0, lblKO);
	CHECK(ret > 0, lblExit);

	EntMod = (char) ret;
	switch (EntMod)
	{
        case 'm':                //Mag card swiped from user dialog
            if(*Buf != 0)
			{
				memcpy(Trk1, Buf, 128);
				ret = fmtTok(0, Trk1, "^");
				CHECK(ret <= sizeof(Trk1), lblInvalidTrk);
				memset(Trk1, 0, sizeof(Trk1));
				memcpy(Trk1, &Buf[ret + 1], 128 - ret);
				ret = fmtTok(CardHolderName, Trk1, "^");  // Retrieve cardholder name from track1
			}

			MAPPUTSTR(traTrk1, CardHolderName, lblKO);

			pTrk2 = &Buf[128];
			MAPPUTSTR(traTrk2, pTrk2, lblKO);
			ret = fmtTok(0, pTrk2, "=");
			CHECK(ret <= lenPan, lblInvalidTrk);  // Search separator '='
			ret = fmtTok(Pan, pTrk2, "=");    // Retrieve Pan from track 2
			VERIFY(ret <= lenPan);

            pTrk2 += ret;         // Continue to analyse track 2 
            CHECK(*pTrk2 == '=', lblInvalidTrk);  // Should be separator
            pTrk2++;
            ret = fmtSbs(ExpDat, pTrk2, 0, lenExpDat);    // Retrieve expiration date from track 2
            VERIFY(ret == 4);

		  //Bao add for check chip card
		  pTrk2 += ret; //For get Service code traSrvCod
		  ret = fmtSbs(Srvcod, pTrk2, 0, 3); // Retrieve 
		  VERIFY(ret == 3);
		  MAPPUTSTR(traSrvCod, Srvcod, lblDBA);
		  if((Srvcod[0] == '2')||(Srvcod[0] == '6'))
		  {
		      if (emvFullProcessCheck() == 1)
	      	  {
				  if(Fallback == 0)
					UseChip = 1;
				  else
				  	UseChip = 0;
	      	  } 
		  }
		  else
	  	 	Fallbackret = 0; 
		  //end
		  
          break;
      case 'c':                //Smart card inserted from user dialog
//++@agmr - tolak transaksi mini atm, card service dan tunai untuk chip
            if(isBRITransaction() == 1 && txnType != trtInstallment)
                goto lblChipBarred;
//--@agmr - tolak transaksi mini atm, card service dan tunai untuk chip
          break;
      default:                 //Card manually entered from user dialog
          VERIFY(ret == 'k');
          FirstKey = Buf[0];    // Analyze the key 
          if(FirstKey == kbdANN) {  // Cancel => exit
              ret = 0;
              goto lblEnd;
          }
          if('0' > FirstKey || FirstKey > '9') {    // No numeric key => exit
              ret = 0;
              goto lblEnd;
          }

			memset(Buf, 0, sizeof(Buf));
			Buf[0] = FirstKey;

			ret = pmtManInput();
			CHECK(ret >= 0, lblExit);

			break;
	}

	//Saving for manual Input is in the function pmtManInput()
	if(EntMod != 'k')
	{
		MAPPUTSTR(traPan, Pan, lblDBA);
        memcpy(BlockedPan + 12, Pan + 12, lenPan-12); //@agmr
        BlockedPan[sizeof(BlockedPan)-1]=0;	                      //@agmr		   
      	MAPPUTSTR(traBlockPan, BlockedPan,lblDBA);   
		MAPPUTSTR(traExpDat, ExpDat, lblDBA);
		strcpy(tmp, Pan);
		strcat(tmp, "=");
		strcat(tmp, ExpDat);
		ret = mapPut(traTrk21, tmp, dspW);
		CHECK(ret >= 0, lblDBA);
		MAPPUTSTR(traTrk22, tmp + dspW, lblDBA);
	}

	if(Fallbackret == 1)
		 EntMod = 'f';

	ret = EntMod;

//prtS("pmtCardInput");
//++ @agmr - BRI	
	if(txnType == trtRenewalKartuInquiry)
	{
	    char buf[20];    
	    memset(buf,0,sizeof(buf));
	    mapGet(traPan,buf,sizeof(buf));
	    mapPut(traNomorKartuLama,buf,lenNomorKartuLama);
	}
	if(txnType == trtAktivasiKartuOtorisasi ||
	   txnType == trtRenewalKartuOtorisasi ||
	   txnType == trtReissuePINOtorisasi 
	   )
	{
	    char buf[20];    
	    memset(buf,0,sizeof(buf));
	    mapGet(traPan,buf,sizeof(buf));
	    mapPut(traNomorKartuPetugas,buf,lenNomorKartuPetugas);	    
	}   
//++ @agmr - BRI	
	goto lblEnd;
lblChipBarred:
    usrInfo(infChipEntryBarred);
    ret = -100;
    goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	ret = 0;
	goto lblEnd;
lblExit:                     //smooth exit
	ret = 0;
	goto lblEnd;
lblInvalidTrk:
	usrInfo(infInvalidTrack);
	ret = 0;
	goto lblEnd;
lblDBA:
	usrInfo(infDataAccessError);
	ret = 0;
	goto lblEnd;
	//  lblStop:
	//    if(!ret)
	//        goto lblEnd;
	//    usrInfo(infProcessingError);
	//    ret = 0;
	//    goto lblEnd;
lblEnd:
	trcFN("pmtCardInput: ret=%d\n", ret);
	return ret;
}

int pmtOldPinInput(void)
{
	char Pan[lenPan + 1];       // Primary account number
	byte Acc[8];                // Account extracted from pan
	byte Blk[lenPinBlk];        // Pinblock calculated by cryptomodule
	char Msg1[dspW + 1];        // "           12.00"
	char Msg2[dspW + 1];        // "PIN:"
	char Msg[(sizeof(Msg1) + sizeof(Msg2) + dspW) + 1];
	byte LocPK;                 // Pin Key location
	int ret;

	trcS("pmtOldPinInput Beg\n");

	memset(Blk, 0, sizeof(Blk));
	ret = cryStart('m');
	CHECK(ret >= 0, lblKO);

	MAPGETBYTE(appPinKeyLoc, LocPK, lblKO);

	// Extract Account Number from Pan for ANSIX 9.8 pin calculation
	MAPGET(traPan, Pan, lblKO);
	ret = stdGetAcc(Acc, Pan);  // Format Pan number (12 pan digits without Luhn preceded by 0000)

	// Pin entry and Pinblock calculation

//	MAPGET(msgPin, Msg2, lblKO);
//	dspStop();                  // Close channel to give cryptomodule access to HMI
//	strcpy(Msg, " ");
//	strcat(Msg, "\n");
//	strcat(Msg, Msg2);
//	strcat(Msg, "\n");          // Display Pin entry on PinPad
//	strcat(Msg, "\n");     

    dspClear();
    dspLS(1,"   UBAH PIN");
    strcpy(Msg,"\nOld PIN ?\n\n");
	ret = cryGetPin(Msg, Acc, LocPK, Blk);
	CHECK(ret >= 0, lblKO);     // Enter Pin
	if(ret == 0)
	{
		goto lblEnd;
	}                           // Cancel, timeout => exit
	ret = dspStart();
	CHECK(ret >= 0, lblKO);     // Now we can open HMI again
	ret = mapPut(traPinBlk, Blk, lenPinBlk);
	CHECK(ret >= 0, lblKO);     // Save Pinblock into DBA (record "tra" transaction stuff)

	ret = 1;
	goto lblEnd;

lblKO:
	usrInfo(infProcessingError);
	ret = 0;
	goto lblEnd;
lblEnd:
	cryStop('m');               // Stop in case if it was started; the return code is not checked intentionnally
	trcFN("pmtOldPinInput: ret=%d\n", ret);
	return ret;
}

int pmtNewPinInput(void)
{
	char Pan[lenPan + 1];       // Primary account number
	byte Acc[8];                // Account extracted from pan
	byte Blk[lenPinBlk];        // Pinblock calculated by cryptomodule
	char Msg1[dspW + 1];        // "           12.00"
	char Msg2[dspW + 1];        // "PIN:"
	char Msg[(sizeof(Msg1) + sizeof(Msg2) + dspW) + 1];
	byte LocPK;                 // Pin Key location
	int ret;
	char pin1[20],pin2[20];
	int i;

	trcS("pmtNewPinInput Beg\n");

    for(i = 0; i < 2; i++)
    {
    	memset(Blk, 0, sizeof(Blk));
    	ret = cryStart('m');
    	CHECK(ret >= 0, lblKO);
    
    	MAPGETBYTE(appPinKeyLoc, LocPK, lblKO);
    
    	// Extract Account Number from Pan for ANSIX 9.8 pin calculation
    	MAPGET(traPan, Pan, lblKO);
    	ret = stdGetAcc(Acc, Pan);  // Format Pan number (12 pan digits without Luhn preceded by 0000)
    
    	// Pin entry and Pinblock calculation
    
    //	MAPGET(msgPin, Msg2, lblKO);
    //	dspStop();                  // Close channel to give cryptomodule access to HMI
    //	strcpy(Msg, " ");
    //	strcat(Msg, "\n");
    //	strcat(Msg, Msg2);
    //	strcat(Msg, "\n");          // Display Pin entry on PinPad
    //	strcat(Msg, "\n");     
        dspClear();
        dspLS(1,"   UBAH PIN");
           
        if(i == 0)
            strcpy(Msg,"\nENTER NEW PIN :");    
        else
            strcpy(Msg,"\nRE-ENTER NEW PIN:");
    	ret = cryGetPin(Msg, Acc, LocPK, Blk);
    	CHECK(ret >= 0, lblKO);     // Enter Pin
    	if(ret == 0)
    	{
    		goto lblEnd;
    	}                           // Cancel, timeout => exit
    	ret = dspStart();
    	CHECK(ret >= 0, lblKO);     // Now we can open HMI again
    	ret = mapPut(traNewPinBlk, Blk, lenPinBlk);
    	CHECK(ret >= 0, lblKO);     // Save Pinblock into DBA (record "tra" transaction stuff)
    	if(i==0)
    	    memcpy(pin1,Blk,lenPinBlk);
    	else
    	    memcpy(pin2,Blk,lenPinBlk);
    	
    	if(i == 1)
    	{
    	    if(memcmp(pin1,pin2,lenPinBlk) == 0)
    	        break;
    	    else
    	    {
                usrInfo(infPINNotMatch);
                i=-1;
    	    }
    	}
    }

	ret = 1;
	goto lblEnd;

lblKO:
	usrInfo(infProcessingError);
	ret = 0;
	goto lblEnd;
lblEnd:
	cryStop('m');               // Stop in case if it was started; the return code is not checked intentionnally
	trcFN("pmtNewPinInput: ret=%d\n", ret);
	return ret;
}

/** Perform the pin input and calculate pinblock.
 * 
 * \header log\\log.h 
 * \source log\\pmt.c
 */
int pmtPinInput(void)
{
	char Pan[lenPan + 1];       // Primary account number
	byte Acc[8];                // Account extracted from pan
	byte Blk[lenPinBlk];        // Pinblock calculated by cryptomodule
	char Msg1[dspW + 1];        // "           12.00"
	char Msg2[dspW + 1];        // "PIN:"
	char Msg[(sizeof(Msg1) + sizeof(Msg2) + dspW) + 1];
	byte LocPK;                 // Pin Key location
	int ret;

/* PIN HARDCODE - mulai */
//	memcpy(Blk, "\x78\x07\x13\x63\x7B\x32\x32\x6E", 8);
//	ret = mapPut(traPinBlk, Blk, lenPinBlk);
//	return 1;
// PIN HARDCODE - selesai*/
	trcS("pmtPinInput Beg\n");

	memset(Blk, 0, sizeof(Blk));
	ret = cryStart('m');
	CHECK(ret >= 0, lblKO);

	MAPGETBYTE(appPinKeyLoc, LocPK, lblKO);

	// Extract Account Number from Pan for ANSIX 9.8 pin calculation
	MAPGET(traPan, Pan, lblKO);
	ret = stdGetAcc(Acc, Pan);  // Format Pan number (12 pan digits without Luhn preceded by 0000)

	// Pin entry and Pinblock calculation

	MAPGET(msgPin, Msg2, lblKO);
	dspStop();                  // Close channel to give cryptomodule access to HMI
	strcpy(Msg, " ");
	strcat(Msg, "\n");
	strcat(Msg, Msg2);
	strcat(Msg, "\n");          // Display Pin entry on PinPad
	strcat(Msg, "\n");     
	ret = cryGetPin(Msg, Acc, LocPK, Blk);
	CHECK(ret >= 0, lblKO);     // Enter Pin
	if(ret == 0)
	{
		goto lblEnd;
	}                           // Cancel, timeout => exit
	ret = dspStart();
	CHECK(ret >= 0, lblKO);     // Now we can open HMI again
	ret = mapPut(traPinBlk, Blk, lenPinBlk);
	CHECK(ret >= 0, lblKO);     // Save Pinblock into DBA (record "tra" transaction stuff)

	ret = 1;
	goto lblEnd;

lblKO:
	usrInfo(infProcessingError);
	ret = 0;
	goto lblEnd;
lblEnd:
	cryStop('m');               // Stop in case if it was started; the return code is not checked intentionnally
	trcFN("pmtPinInput: ret=%d\n", ret);
	return ret;
}

/** Allows user to enter amount, validates it then returns the validation result. 
 *    - Data element : traAmt
 * 
 * \header log\\log.h
 * \source log\\pmt.c
 */
int pmtAmountInput(void) {
    char Amt[lenAmt + 1];
    byte Exp;
    int ret;

    trcS("pmtAmountInput Beg\n");
    
// ++ @agmr - BRI- inquriy transaction
    {
    	byte tmp;
    	
        MAPGETBYTE(traTxnType,tmp,lblKO);
        switch(tmp)
        {
//            case trtInquiryTrans:
            case trtInfoSaldo:
            case trtInfoSaldoBankLain:
			case trtMiniStatement:
            case trtTransferSesamaBRIInquiry:
            case trtTransferSesamaBRI:
            case trtTransferAntarBankInquiry:
            case trtTransferAntarBank:
            case trtPembayaranPLNPascaInquiry:
            case trtPembayaranPLNPasca:
            case trtPembayaranCicilanInquiry:
            case trtPembayaranCicilan:
            case trtPembayaranPulsa:
            case trtPembayaranZakat:
            case trtSetorSimpananInquiry:
            case trtSetorSimpanan:
            case trtSetorPinjamanInquiry:
            case trtSetorPinjaman:
            case trtTarikTunai:
            case trtVoidTarikTunai:
            case trtRegistrasiInternetBanking:
            case trtRegistrasiPhoneBanking:
            case trtAktivasiKartuOtorisasi:
            case trtAktivasiKartuInquiry:
            case trtAktivasiKartu:
            case trtRenewalKartuOtorisasi:
            case trtRenewalKartuInquiry:
            case trtRenewalKartu:
            case trtReissuePINOtorisasi:
            case trtReissuePIN:
            case trtGantiPasswordSPV:
            case trtInstallment:
            case trtInfoKodeBank:
            case trtPembayaranPascaBayarInquiry:
            case trtPembayaranPascaBayar:
                return 1;
            default:
                break;              
        }
    }
// -- @agmr - BRI - inquriy transaction        

    usrInfo(infAmtInput);
    MAPGETBYTE(appExp, Exp, lblKO);
    memset(Amt, 0, sizeof(Amt));
    ret = enterAmt(3, Amt, Exp);
    CHECK(ret >= 0, lblKO);
//++@agmr 
    if(ret == kbdANN || ret == kbdF1 || ret == 0)
    {
        ret = -100;
        goto lblEnd;
    }
//--@agmr        
    if(ret != kbdVAL) {         // Cancel, timeout => exit
        ret = 0;
        goto lblEnd;
    }
    MAPPUTSTR(traAmt, Amt, lblKO);

    ret = valAmount();
    CHECK(ret > 0, lblKO);      // Check amount limit
    ret = 1;
    goto lblEnd;
  lblKO:
//++@agmr
    if(ret == -100)
    	goto lblEnd;
//--@agmr
    usrInfo(infProcessingError);
    ret = 0;
    goto lblEnd;
  lblEnd:
    trcFN("pmtAmountInput: ret=%d\n", ret);
    return ret;
}

/** Perform the tip amount input
 * \header log\\log.h
 * \source log\\pmt.c
 */
int pmtTipInput(void) {
    char TipAmt[lenAmt + 1];
    int ret;
    byte idx;
    byte Exp;

    trcS("pmtTipInput Beg\n");
    MAPGETBYTE(traTxnType, idx, lblKO);
    switch (idx) {
      case trtSale:
      case trtOffline:
          break;
      default:
          goto lblEnd;
    }

    ret = valIsTipAllowed();
    CHECK(ret >= 0, lblEnd);
    if(ret == 0) {
        ret = 1;
        goto lblEnd;
    }
    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    usrInfo(infTipInput);
    MAPGETBYTE(appExp, Exp, lblKO);
    memset(TipAmt, 0, sizeof(TipAmt));
    ret = enterAmt(3, TipAmt, Exp); // Tip Amount entry
    CHECK(ret >= 0, lblKO);
    if(ret != kbdVAL) {         // Cancel, timeout => exit
        ret = 0;
        goto lblEnd;
    }
    MAPPUTSTR(traTipAmt, TipAmt, lblKO);
    if(atoi(TipAmt) > 0) {
    	MAPPUTBYTE(traTipFlag, 1, lblKO);
    }
    ret = 1;
    goto lblEnd;
  lblKO:
    usrInfo(infProcessingError);
    ret = 0;
    goto lblEnd;
  lblEnd:
    trcFN("pmtTipInput: ret=%d\n", ret);
    return ret;
}

static int pmtComputeTotAmt(void)
{
	int ret;
	char amtS[lenAmt + 1];
	char amtTip[lenAmt + 1];
	char amtTot[lenTotAmt + 1];

// ++ @agmr - BRI- inquriy transaction
    {
    	byte tmp;
    	
        MAPGETBYTE(traTxnType,tmp,lblKO);
        switch(tmp)
        {
//            case trtInquiryTrans:
            case trtInfoSaldo:
			case trtMiniStatement:
                return 1;
            default:
                break;              
        }
    }
// -- @agmr - BRI - inquriy transaction    

	MAPGET(traAmt, amtS, lblKO);
	MAPGET(traTipAmt, amtTip, lblKO);

	ret = sizeof(amtTot);
	VERIFY(ret >= 16 + 1);      //the length should be >= 16+1, see syntax of addStr
	memset(amtTot, 0, sizeof(amtTot));
	addStr(amtTot, amtS, amtTip);
	trcFS("amtS=%s\n", amtS);
	trcFS("amtTip=%s\n", amtTip);
	trcFS("amtTot=%s\n", amtTot);
	CHECK(strlen(amtTot) <= lenAmt, lblKO);
	MAPPUTSTR(traTotAmt, amtTot, lblKO);
	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	ret = 0;
	goto lblEnd;
lblEnd:
	trcFN("pmtComputeTotAmt: ret=%d\n", ret);
	return ret;
}

/** Process other transaction types with similar txn flow like void & tip adjust.
 * \source log\\pmt.c 
 */
static int pmtTxnOthers(byte txnType)
{
	int ret;
	word key;
	byte flag;
	word menuItem = 0;
	byte IssId,AcqId; //@agmr

	trcS("pmtTxnOthers Beg\n");

    txnAsli = 0; //@agmr
	switch (txnType)
	{
		case trtVoid:
			key = appVoidPwd;
			menuItem = mnuVoid;
			break;
			
	    case trtVoidTarikTunai:
	        key = appVoidPwd;
	        menuItem = mnuVoidTarikTunai;
			break;
		default:
			key = appVoidPwd;
			break;
	}

	ret = scrDlg(infEnterTrace, traFindRoc);
	CHECK(ret >= 0, lblKO);
	CHECK(ret == kbdVAL, lblEnd);

    if(txnType == trtVoidTarikTunai)
        ret = briLogLoadByRoc(TUNAI_LOG);
    else
	    ret = logLoad();
	CHECK(ret > 0, lblEnd);

    //++@agmr - kembaliin mnuItem
    MAPPUTWORD(traMnuItm,menuItem,lblKO);
    //--@agmr - kembaliin mnuItem
    
//++ @agmr     
    MAPPUTBYTE(regLocType, 'T', lblKO); 
    
	MAPGETBYTE(traIssIdx, IssId, lblKO);
	MAPGETBYTE(traAcqIdx, AcqId, lblKO);
	mapMove(issBeg, (word) IssId);
	mapMove(acqBeg, (word) AcqId);
//-- @agmr      

	if(txnType == trtVoid || txnType == trtVoidTarikTunai)
	{
		MAPGETBYTE(traVoidFlag, flag, lblKO);
		CHECK(flag != 1, lblAlreadyVoided);
	}

	ret = pmtComputeTotAmt();
	CHECK(ret > 0, lblKO);

	switch (txnType)
	{
		case trtVoid:
			key = infVoidTotal;
			break;
		default:
			key = infVoidTotal;
			break;
	}

//	ret = usrInfo(key);         //total amount confirmation
{
    char buf[30],buf1[30];
    
    memset(buf,0,sizeof(buf));
    mapGet(traTotAmt,buf,sizeof(buf));
    fmtAmt(buf1,buf,0,",.");
    fmtPad(buf1,-16,' ');
    MAPPUTSTR(msgBuffer,buf1,lblKO);
	ret = usrInfo(infVoidTotal2);
	CHECK(ret > 0, lblKO);
	CHECK(ret == kbdVAL, lblKO);
}

// ++ @agmr - BRI
    ret = pmtBRIKonfirmasiTanpaInquiry(txnType);
    if(ret <= 0)
        goto lblKO;
// ++ @agmr - BRI

	ret = onlSession();         // Start on-line authorization request
	CHECK(ret > 0, lblKO);
	
    ret = valRspCod();
    CHECK(ret > 0, lblDeclined);
	
	MAPPUTWORD(traMnuItm, menuItem, lblKO);
	ret = logSave();
	CHECK(ret > 0, lblKO);

	ret = 1;
	goto lblEnd;

lblDeclined:
    ret = -3; //@agmr - BRI
    goto lblEnd;
    
lblAlreadyVoided:
	usrInfo(infAlreadyVoided);
	ret = 0;
	goto lblEnd;
lblKO:
	ret = 0;
	trcS("pmtTxnOthers aborted\n");
lblEnd:
	trcFN("pmtTxnOthers: ret=%d\n", ret);
	return ret;
}

/** Ask user to input approval code.
 * \source log\\pmt.c 
 */
static int pmtAskAppCod(void)
{
	int ret;

	trcS("pmtAskAppCod Beg\n");

	ret = scrDlg(infEnterAppCod, traAutCod);
	CHECK(ret >= 0, lblKO);
	if(ret == kbdANN)
	{
		ret = 0;
		goto lblEnd;
	}

	ret = 1;
	goto lblEnd;

lblKO:
	trcS("pmtAskAppCod aborted\n");
lblEnd:
	trcFN("pmtAskAppCod: ret=%d\n", ret);
	return ret;
}

static void pmtPrepareOfflineIfAvailable(void)
{
	card idx = 0;
	char buf[lenMnu + 1], Rrn[lenRrn + 1];
	word mnuItem, trxNum;
	byte TxnTypebeforeOffline;
	word TxnMnuItmbeforeOffline, count;
	int ret;

	trcS("pmtPrepareOfflineIfAvailable Beg\n");
	//check if trickle feed is enabled
	ret = valIsTrickleFeedEnabled();//if trickle feed is not enabled do not process this function
	CHECK(ret > 0, lblEnd);

	//check first for offline transactions
	MAPGETWORD(regOfflineCnt, count, lblKO);
	if(count > 0)
	{
		//save last transaction details before sending offline txns
		MAPGETBYTE(traTxnType, TxnTypebeforeOffline, lblKO);
		MAPPUTBYTE(traTxnTypeBeforeOffline, TxnTypebeforeOffline, lblKO);
		MAPGETWORD(traMnuItm, TxnMnuItmbeforeOffline, lblKO);
		MAPPUTBYTE(traMnuItmBeforeOffline, TxnMnuItmbeforeOffline, lblKO);

		MAPPUTBYTE(traTxnType, trtOffline, lblKO);
		MAPPUTWORD(traMnuItm, mnuOffline, lblKO);   //change first to offline for proper connection to host
		MAPGET(mnuOffline, buf, lblKO);
		MAPPUTSTR(traCtx, buf + 2, lblKO);

		//initialize log to point on first offline txn
		MAPPUTBYTE(regLocType, 'L', lblKO);
		MAPGETWORD(regTrxNo, trxNum, lblKO);
		while(idx < trxNum)
		{
			mapMove(logBeg, (word) idx);
			MAPGETWORD(logMnuItm, mnuItem, lblKO);
			if(mnuItem == mnuOffline)
			{
				MAPGET(logRrn, Rrn, lblKO);
				if(strlen(Rrn) == 0)
				{
					trcS("offline txn present\n");
					break;
				}
			}
			idx++;
		}
		if((idx <= trxNum) && (mnuItem == mnuOffline))
		{
			MAPPUTBYTE(traTxnType, trtOffline, lblKO);
			idx++;
			MAPPUTWORD(traIdx, idx, lblKO);
		}
	}
lblKO:
lblEnd:
	trcS("pmtPrepareOfflineIfAvailable End\n");
}

/** Perform the card validation, amount, pin, etc input and on-line authorization.
 * \return
 *    -  >0 : Magnetic stripe processing done.
 *    -  =0 : User cancel or timeout
 *    -  <0 : Transaction failure
 * \source log\\pmt.c 
 */
static int pmtMagStripe(void)
{
	int ret;
	word key;
	byte PanDisplay;
	char Amt[lenAmt + 1];
	byte txnType;

	trcS("pmtMagStripe Beg\n");

	//Bao add for EMV
	if(UseChip == 1){
		dspClear();
		usrInfo(infEMVChipDetect);
		Beep();
		ret = 0;
		goto lblEnd;
	}
	//==========

    ret = valCard();            // Track2 analysis 
    CHECK(ret > 0, lblKO);

//++@agmr
    if(checkPendingSettlement()!=0)
        goto lblExit;
//--@agmr

	MAPGETBYTE(appTermOpt1, PanDisplay, lblKO);
	if(PanDisplay & 0x02)
	{
		while(1)// @@OA
		{		
		ret = usrInfo(infConfirmPan);
		CHECK(ret >= 0, lblKO);
		if(ret == kbdVAL) 	// @@OA
			break;			// @@OA
		if(ret == kbdANN)	// @@OA
			goto lblExit;
		Beep();
		//CHECK(ret == kbdVAL, lblExit);	// @@OA
		}
	}

	MAPGETBYTE(traTxnType, txnType, lblKO);

    if(txnType == trtGantiPIN ||
       txnType == trtAktivasiKartuInquiry ||
       txnType == trtRenewalKartu)
    {
		ret = valIsPinRequired();
		CHECK(ret >= 0, lblKO);
        if(ret)
        {
            if(txnType == trtGantiPIN)
            {
		        ret = pmtOldPinInput();    // Old Pin entry
		        CHECK(ret > 0, lblKO);
		    }
		
		    ret = pmtNewPinInput();    // New Pin entry
		    CHECK(ret > 0, lblKO);		
		}
		else if(ret == 0)
	    {
	        usrInfo(infCardPinNotRequired);
	        goto lblExit;
	    }
    }
  	
	else
	{
		MAPGET(traAmt, Amt, lblKO);
		if(Amt[0] == 0) {
			ret = pmtAmountInput();     // Enter transaction amount
			CHECK(ret > 0, lblKO);

			ret = pmtTipInput();        // Enter Tip amount
			CHECK(ret > 0, lblKO);

			ret = valAmount();          //validate amount
			CHECK(ret > 0, lblKO);

        // ++ @agmr - BRI- inquriy transaction
            MAPGETBYTE(traTxnType,txnType,lblKO);            
            switch(txnType)
            {
//                    case trtPayment:
                    case trtInfoSaldo:
                    case trtInfoSaldoBankLain:
					case trtMiniStatement:
                    case trtTransferSesamaBRIInquiry:
                    case trtTransferSesamaBRI:
                    case trtTransferAntarBankInquiry:
                    case trtTransferAntarBank:
                    case trtPembayaranPLNPascaInquiry:
                    case trtPembayaranPLNPasca:
                    case trtPembayaranCicilanInquiry:
                    case trtPembayaranCicilan:
                    case trtPembayaranPulsa:
                    case trtPembayaranZakat:
                    case trtSetorSimpananInquiry:
                    case trtSetorSimpanan:
                    case trtSetorPinjamanInquiry:
                    case trtSetorPinjaman:
                    case trtTarikTunai:
                    case trtVoidTarikTunai:
                    case trtRegistrasiInternetBanking:
                    case trtRegistrasiPhoneBanking:
                    case trtAktivasiKartuOtorisasi:
                    case trtAktivasiKartuInquiry:
                    case trtAktivasiKartu:
                    case trtRenewalKartuOtorisasi:    
                    case trtRenewalKartuInquiry:
                    case trtRenewalKartu:
                    case trtReissuePINOtorisasi:
                    case trtReissuePIN:
                    case trtGantiPasswordSPV:
                    case trtInstallment:
                    case trtInfoKodeBank:
                   	case trtPembayaranPascaBayarInquiry:
                   	case trtPembayaranPascaBayar:
                    goto lblJump;
                default:
                    break;              
            }
        // -- @agmr - BRI - inquriy transaction    

			ret = pmtComputeTotAmt();
			CHECK(ret > 0, lblKO);
    
//			ret = usrInfo(infTotal);    //total amount confirmation
//			CHECK(ret > 0, lblKO);
//			CHECK(ret == kbdVAL, lblExit);
{
            char buf[30],buf1[30];
            
            memset(buf,0,sizeof(buf));
            mapGet(traTotAmt,buf,sizeof(buf));
            fmtAmt(buf1,buf,0,",.");
            fmtPad(buf1,-16,' ');
            MAPPUTSTR(msgBuffer,buf1,lblKO);
        	ret = usrInfo(infTotal2);
        	CHECK(ret > 0, lblKO);
        	CHECK(ret == kbdVAL, lblExit);
}

		}
		
lblJump: ////  @agmr - BRI - inquiry transaction
        if(txnType != trtReissuePIN) //@agmr - BRI
        {    
		    ret = valIsPinRequired();
		    CHECK(ret >= 0, lblKO);
		    if(ret > 0)
		    {
			    ret = pmtPinInput();    // Pin entry
			    CHECK(ret > 0, lblKO);
		    }
		}
	}
	
//++ @agmr - BRI
        ret = pmtBRICustomInput();     // Enter Custom Input
        CHECK(ret > 0, lblKO);
//-- @agmr - BRI
	
// ++ @agmr - BRI
    ret = pmtBRIKonfirmasiTanpaInquiry(txnType);
    if(ret <= 0)
        goto lblEnd;
// ++ @agmr - BRI

	MAPGETWORD(traMnuItm, key, lblKO);
	switch (key)
	{
		case mnuOffline:
			ret = pmtAskAppCod();
			break;
		default:
			pmtPrepareOfflineIfAvailable();
			ret = onlSession();   // Start on-line authorization request
			break;
	}
	CHECK(ret > 0, lblKO);
	
	ret = 1;
	goto lblEnd;

lblExit:                     //smooth exit
	ret = 0;
	goto lblEnd;
lblKO:
	goto lblEnd;
lblEnd:
	trcFN("pmtMagStripe: ret=%d\n", ret);
	return ret;
}

/** Convert some data elements to EMV binary format and perform EMV transaction processing
 * \header log\\log.h
 * \source log\\pmt.c 
 */
#ifdef __EMV__
static int pmtSmartCard(void)
{
	int ret;
	byte TrnTyp[1 + lenTrnTyp];
	char DatTim[lenDatTim + 1];
	byte TrnDat[1 + lenTrnDat];
	byte TrnTim[1 + lenTrnTim];
	card InvNum;
	byte TrnSeqCnt[1 + lenTrnSeqCnt];
	char buf[2 * lenTrnSeqCnt + 1];

	trcS("pmtSmartCard Beg\n");
	
	usrInfo(infProcessing); //@agmr

	//Convert data elements to EMV binary format
	TrnTyp[0] = lenTrnTyp;
	VERIFY(TrnTyp[0] == 1);
	//MAPGETBYTE(rqsEmvTrnTyp, TrnTyp[1],lblKO);
	TrnTyp[1] = 0;              //AB: stub
	ret = mapPut(emvTrnTyp, TrnTyp, 1 + lenTrnTyp);
	CHECK(ret >= 0, lblDBA);

	MAPGET(traDatTim, DatTim, lblDBA);
	TrnDat[0] = lenTrnDat;
	VERIFY(TrnDat[0] == 3);
	ret = hex2bin(&TrnDat[1], &DatTim[2], lenTrnDat);
	VERIFY(ret == 3);
	ret = mapPut(emvTrnDat, TrnDat, 1 + lenTrnDat);
	CHECK(ret >= 0, lblDBA);

	TrnTim[0] = lenTrnTim;
	VERIFY(TrnTim[0] == 3);
	ret = hex2bin(&TrnTim[1], &DatTim[2 + 6], lenTrnTim);   // "hhmmss" -> "\xhh\xmm\xss"
	VERIFY(ret == 3);
	ret = mapPut(emvTrnTim, TrnTim, 1 + lenTrnTim);
	CHECK(ret >= 0, lblDBA);

	MAPGETCARD(regInvNum, InvNum, lblDBA);
	TrnSeqCnt[0] = lenTrnSeqCnt;
	VERIFY(TrnSeqCnt[0] == 4);
	num2dec(buf, InvNum, 2 * lenTrnSeqCnt); // ulong -> "NNNNNNNN"
	hex2bin(&TrnSeqCnt[1], buf, lenTrnSeqCnt);  // "NNNNNNNN" -> "\xNN\xNN\xNN\xNN"
	ret = mapPut(emvTrnSeqCnt, TrnSeqCnt, 1 + lenTrnSeqCnt);
	CHECK(ret >= 0, lblDBA);

    ret = emvProcessing();
    CHECK(ret >= 0, lblKO);

    //ret = 1; //Bao comment this
    goto lblEnd;
  lblDBA:
    usrInfo(infDataAccessError);
    ret = -1; //Bao comment this
    goto lblEnd;
  lblKO:
    goto lblEnd;
  lblEnd:
    trcFN("pmtSmartCard ret=%d\n", ret);
    return ret;
}
#endif

static int pmtLogSave(void)
{
	int ret;
	char RspCod[lenRspCod + 1]; //@agmr - BRI

	trcS("pmtLogSave Beg\n");
	
	//+++ @agmr - BRI
	if(isBRITransaction() == 1)
	{
	    if(isBRISaveToBatch() == 1)
	        goto saveBatch;
	    else
	    {
	        ret = 1;
	        goto lblEnd;
	    }
	}
	
	MAPGET(traRspCod, RspCod, lblKO);
	if(strcmp(RspCod, "00") != 0)
	{
	    ret = 1;
		goto lblEnd;
	}
	
	//--- @agmr - BRI
saveBatch:	
	ret = logSave();            //save data on current transaction into log
	CHECK(ret >= 0, lblKO);
	goto lblEnd;
lblKO:
	goto lblEnd;
lblEnd:
	trcFN("pmtLogSave ret=%d\n", ret);
	return ret;
}

static int fmtAmtReceipt(word key, char *amt)
{
    int ret;
    char amtFmt[20+1];
    byte exp;	

    MEM_ZERO(amtFmt);
    MAPGETBYTE(appExp, exp, lblKO);
    fmtAmt(amtFmt, amt, exp, ",.");
    ret = mapPut(key, amtFmt, strlen(amtFmt));
	CHECK(ret > 0, lblKO);

    return ret;	
    lblKO:
	return -1;	
		
}

static int pmtReceipt(word key)
{
	int ret;
	char EntMod;
	word mnuItem;
	char traName[dspW + 1];
	byte idx;
	char amtS[lenAmt + 1];
	char curr_sign[lenCurrSign + 1], fmtCurr[5+1];
	char buffExpDat[lenExpDat + 1], ExpDat[lenExpDat + 2];
	byte pinVer;
//++ @agmr - BRI  
    byte singleReceipt=0;
    char buf[64],buf1[64];
    word txnType; //BRI2
    byte isEMV = 0;
    byte isNegative = 0;
    byte isChip = 0;
    byte isVoid = 0;
    char name[30];
    char strExpDate[30];
	char BlockedPan[lenPan+1];
	char Pan[lenPan + 1];  
	char amount[20];  
	char str[128];
	card amt;
//-- @agmr - BRI    	

	MAPGETBYTE(traEntMod, EntMod, lblKO);
	switch (EntMod)
	{
		case 'c':
		case 'C':
			ret = 1;
			isChip = 1; //@agmr
			break;
		default:
			ret = valIsPrintRequired();
			CHECK(ret >= 0, lblEnd);
			break;
	}

	if(ret == 0)
	{
		ret = 1;
		goto lblEnd;
	}
    
    if(isBRITransaction() == 1 && key != trtInstallment) //@agmr - BRI
        ret = usrInfo(infPrinting);
    else
	    ret = usrInfo(infMerCopyPrinting);
	CHECK(ret >= 0, lblKO);
	
// ++ @agmr - BRI 
    if(isBRISaveToBatch() == 0)   
        ret = strFormatDatTim(traDatTim);
    else    
// -- @agmr - BRI        
	    ret = strFormatTraDatTim();
	CHECK(ret >= 0, lblKO);

	//get original transaction name
	//++ @agmr
	if(key == trtVoid)
	{
	    MAPGETBYTE(traOriginalTxnType, idx, lblKO);
	    isVoid == 1;
	}
	else
	//-- @agmr
	{
	    MAPGETBYTE(traTxnType, idx, lblKO);
	}
	    	    
	mapMove(rqsBeg, idx - 1);
	MAPGET(rqsDisp, traName, lblKO);
	MAPPUTSTR(traTransType, traName, lblKO);

	MAPGET(traAmt, buf, lblKO);
	ret = fmtAmtReceipt(traFmtAmt, buf);
	CHECK(ret > 0, lblKO);
	MAPGET(traTipAmt, buf1, lblKO);
	ret = fmtAmtReceipt(traFmtTip, buf1);
	CHECK(ret > 0, lblKO);
	addStr(amtS, buf, buf1);
	
	ret = fmtAmtReceipt(traTotAmt, amtS);
	CHECK(ret > 0, lblKO);
    MAPGET(appCurrSign, curr_sign, lblKO);
    MEM_0x20(fmtCurr);
    memcpy(fmtCurr, curr_sign, 3);
    MAPPUTSTR(appCurrSignSpaces, fmtCurr, lblKO);

	memset(ExpDat,0,sizeof(ExpDat));
	MAPGET(traExpDat, buffExpDat, lblKO);
	FmtExpDat(ExpDat, buffExpDat);
	MAPPUTSTR(traExpDatFmt, ExpDat, lblKO);

    txnType = key;
    if(emvFullProcessCheck() == 1)
    {    
        isEMV = 1;
    }
    
    ret = briPrintData(txnType, &key, &singleReceipt);
    if(ret < 0)
        goto lblKO;
    
    txnType = idx;
    if(ret == 0) //bukan transaksi BRI
    {  
        memset(name,0,sizeof(name));
        if(isVoid)
        {
            strcpy(name, "VOID ");
            if(txnType != trtRefund)
                isNegative = 1;
        }
        else
        {
            if(txnType == trtRefund)
                isNegative = 0;
        }
        if(isChip == 1)
        {
            MAPPUTSTR(traChipSwipe, " (Chip)", lblKO);
        }
                    
        memset(strExpDate,0,sizeof(strExpDate));
        sprintf(strExpDate,"EXPY DATE :  %s",ExpDat);
        MAPPUTSTR(rptBuf1,"",lblKO);
        MAPPUTSTR(rptBuf2,"",lblKO);
        MAPPUTSTR(rptBuf3,"",lblKO);
        MAPPUTSTR(rptBuf4,"",lblKO);
        MAPPUTSTR(rptBuf5,"",lblKO);
        MAPPUTSTR(rptBuf6,"",lblKO);  
        MAPPUTSTR(rptBuf7,"",lblKO);
        MAPPUTSTR(rptBuf8,"",lblKO);                
        
    	mapGet(traPan,Pan,sizeof(Pan));
    	memset(BlockedPan, '*', sizeof(BlockedPan));
        memcpy(BlockedPan + 12, Pan + 12, lenPan-12); //@agmr
        BlockedPan[sizeof(BlockedPan)-1]=0;	                      //@agmr
        MAPPUTSTR(traBlockPan, BlockedPan,lblKO);        

            
    	switch (txnType)
    	{
    		case trtSale:
    		    strcat(name,"SALE");
                memset(buf,0,sizeof(buf));
                sprintf(buf,"%48s",strExpDate);
                memcpy(buf,name,strlen(name));
                
                MAPPUTSTR(rptBuf1,buf,lblKO);     		    
    		    if(isEMV)
    		    {
    		        key = rloTrxEMV;
    		    }
    		    else
    		    {
    			    key = rloTrx;
    			}
    			break;
    		case trtPreAut:
    			key = rloPreAuth;
    			break;
//++ @agmr - BRI5    			
    		case trtAut:
    		    strcat(name,"AUTH");
                memset(buf,0,sizeof(buf));
                sprintf(buf,"%48s",strExpDate);
                memcpy(buf,name,strlen(name));
                
                MAPPUTSTR(rptBuf1,buf,lblKO);     		    
    		    if(isEMV)
    		    {
    		        key = rloTrxEMV;
    		    }
    		    else
    		    {
    			    key = rloTrx;
    			}
    			break;
//-- @agmr - BRI5      			  			
    		case trtRefund:
    		    strcat(name,"REFUND");
                memset(buf,0,sizeof(buf));
                sprintf(buf,"%48s",strExpDate);
                memcpy(buf,name,strlen(name));
                
                MAPPUTSTR(rptBuf1,buf,lblKO);     		    
    		    if(isEMV)
    		    {
    		        key = rloTrxEMV;
    		    }
    		    else
    		    {
    			    key = rloTrx;
    			}
    			break;
    			
    		case trtInstallment:
    		    strcat(name,"CCL BRING");
                memset(buf,0,sizeof(buf));
                sprintf(buf,"%48s",strExpDate);
                memcpy(buf,name,strlen(name));
                MAPPUTSTR(rptBuf1,buf,lblKO);    
    
    			memset(amount,0,sizeof(amount));
    			mapGet(traAmt,amount,sizeof(amount));
    			memset(buf,0,sizeof(buf));
    			fmtAmt(buf, amount, 0, ",.");
    			if(isNegative == 1)
    			    sprintf(str,"AMOUNT           -Rp. %26s",buf);
    			else
    			    sprintf(str,"AMOUNT            Rp. %26s",buf);
    //            strcpy(str,"12345678901234567890123456789012345678901234567890");
    			MAPPUTSTR(rptBuf4,str,lblKO);  
    			
    			if(isNegative == 1)
    			    sprintf(str,"  POKOK          -Rp. %26s",buf);
    			else
    			    sprintf(str,"  POKOK           Rp. %26s",buf);
    			MAPPUTSTR(rptBuf6,str,lblKO);
    			         
                memset(buf,0,sizeof(buf));
                mapGet(traInstallmentPlan,buf,lenInstallmentPlan);
                strcpy(str,"INFOMASI CICILAN BRING");
                MAPPUTSTR(rptBuf5,str,lblKO);			         
    			  
                memset(buf,0,sizeof(buf));
                mapGet(traInstallmentTerm,buf,lenInstallmentTerm);
                strcat(buf," BULAN");
                sprintf(str,"  JANGKA WAKTU %33s",buf);
                MAPPUTSTR(rptBuf7,str,lblKO); 		    
    
    			memset(amount,0,sizeof(amount));
    			mapGet(traInstallmentCicilan,amount,lenInstallmentCicilan);
    			amount[strlen(amount)-2] = 0;
    			memset(buf,0,sizeof(buf));
    			fmtAmt(buf, amount, 0, ",.");
    			sprintf(str,"  CICILAN BULANAN Rp. %26s",buf);			
                MAPPUTSTR(rptBuf8,str,lblKO); 
                if(isEMV)				    
    			    key = rloInstallmentEMV;
    			else
    			    key = rloInstallment;
    			break;	                

    		case trtCash:
    			key = rloTrx;
    			break;
    		case trtVerify:
    			key = rloCrdVrfy;
    			break;
    		case trtOffline:
    			MAPPUTSTR(traTransType, "OFFLSALE", lblKO);  
    			key = rloOfflineSale;
    			
    			break;
    			
    		default:
    			break;
    	}
    	
        memset(buf,0,sizeof(buf));
        memset(buf1,0,sizeof(buf1));
        if(isNegative ==1)
            strcpy(buf1,"-Rp. ");
        else
            strcpy(buf1,"Rp. ");
        mapGet(traAmt,buf,sizeof(buf));
        fmtAmt(buf1+strlen(buf1),buf,0,",.");
        memset(buf,0,sizeof(buf));
        sprintf(buf,"AMOUNT %40s",buf1);
        MAPPUTSTR(rptBuf2,buf,lblKO);
        
        memset(buf,0,sizeof(buf));
        memset(buf1,0,sizeof(buf1));
        if(isNegative ==1)
            strcpy(buf1,"-Rp. ");
        else
            strcpy(buf1,"Rp. ");
        mapGet(traTotAmt,buf,sizeof(buf));
		strcat(buf1,buf);
        memset(buf,0,sizeof(buf));
        sprintf(buf,"TOTAL %18s",buf1);
        MAPPUTSTR(rptBuf3,buf,lblKO); 
        
        if(txnType == trtInstallment && mnuItem != mnuVoid)
            MAPPUTSTR(rptBuf3,"",lblKO);     	
    }

	MAPGETWORD(traMnuItm, mnuItem, lblKO);
//	if(mnuItem == mnuVoid)
//	{
//	    if(isEMV)
//	        key = rloTrxEMV;
//	    else
//		    key = rloTrx;
////		key = rloVoid;
//	}

	trcS("pmtReceipt Beg\n");

	ret = rptReceipt(key);
	CHECK(ret >= 0, lblKO);

//	if(emvFullProcessCheck() == 1)
//	{
//        if(mnuItem != mnuInstallment)
//            rptReceipt(rloSpace);
//	    
//		ret = rptReceipt(rloEmvTags);
//		CHECK(ret >= 0, lblKO);
//	}

// ++ @agmr - BRI
    if(singleReceipt)
    {    
        rptReceipt(rloReceiptFeed);
        goto lblEnd;     
    }
    if(isBRITransaction() == 1 && txnType != trtInstallment)
    {
        ret = rptReceipt(rloReceiptTypeBankBRITrans);
    }
    else    
    {     
// -- @agmr - BRI    
    	MAPGETBYTE(traPinVer, pinVer, lblKO);
    
    	if(pinVer == 1)
    		ret = rptReceipt(rloPinVerTypeMerchant);
    	else
    		ret = rptReceipt(rloReceiptTypeMerchant);
    }

    if( ((txnType == trtTarikTunai) || (txnType == trtVoidTarikTunai)) && (singleReceipt == 0))
    {
        goto lblCustomerCopy;
    }    
	ret = usrInfo(infBankCopyConfirm);
	if(ret != kbdANN)
	{
		ret = usrInfo(infBankCopyPrinting);
		CHECK(ret >= 0, lblKO);
		ret = rptReceipt(key);

		if((EntMod == 'C') || (EntMod == 'c'))
		{
	        if(mnuItem != mnuInstallment)
	            rptReceipt(rloSpace);
		}

		CHECK(ret >= 0, lblKO);
		
// ++ @agmr - BRI
        if(singleReceipt)
        {    
            rptReceipt(rloReceiptFeed);
            goto lblEnd;     
        }
        if(isBRITransaction() == 1&& txnType != trtInstallment)
            ret = rptReceipt(rloReceiptTypeBankBRITrans);
        else    
        {     
// -- @agmr - BRI    		
		
    	if(pinVer == 1)
    		ret = rptReceipt(rloPinVerTypeBank);
    		else	
    		ret = rptReceipt(rloReceiptTypeBank);
    	}
	}
	
lblCustomerCopy: //@agmr
	ret = usrInfo(infCustCopyConfirm);
	if(ret != kbdANN)
	{
		ret = usrInfo(infCustCopyPrinting);
		CHECK(ret >= 0, lblKO);

		MAPPUTBYTE(traCustFlag, 1, lblKO);
		
		ret = rptReceipt(key);

		if((EntMod == 'C') || (EntMod == 'c'))
		{
	        if(mnuItem != mnuInstallment)
	            rptReceipt(rloSpace);
		}

		CHECK(ret >= 0, lblKO);
		
// ++ @agmr - BRI
        if(isBRITransaction() == 1 && txnType != trtInstallment)
            ret = rptReceipt(rloReceiptTypeCustomerBRITrans);
        else
        {
// -- @agmr - BRI       		
		
        	if(pinVer == 1)
        		ret = rptReceipt(rloPinVerTypeCustomer);
        		else
        		ret = rptReceipt(rloReceiptTypeCustomer);
        }
	}

	MAPPUTBYTE(traCustFlag, 0, lblKO);

	if(debugEmvFlag == 1){
		if(EntMod == 'C' ||EntMod == 'c')
			ret = rptReceipt(rloEmvDebug);
			//ttestall(0, 300);
		tmrSleep(3);
	}
	
	goto lblEnd;

lblKO:
	ret = -1;
	goto lblEnd;
lblEnd:
	trcFN("pmtReceipt ret=%d\n", ret);
	return ret;
}

//int pmtReceipt2(word key)
//{
//	int ret;
//
//	ret = pmtReceipt(key);
//	return ret;
//}

/** Increments Invoice num data element : regInvNum
 * \header log\\log.h
 * \source log\\pmt.c
 */
int pmtIncrementInv(void)
{
	int ret;
	card invNum;

	MAPGETCARD(regInvNum, invNum, lblKO);
	if(invNum >= 999999)
		invNum = 0;
	invNum++;
	MAPPUTCARD(regInvNum, invNum, lblKO);
	goto lblEnd;
lblKO:
	ret = -1;
	goto lblEnd;
lblEnd:
	trcFN("pmtIncrementInv ret=%d\n", ret);
	return ret;
}

/** Detect if the transaction is supposed to be forced on line.
 * \header log\\log.h
 * \source log\\pmt.c
 */
//void pmtPayment(byte mnuItem)
//int pmtPayment(byte mnuItem) //@agmr - BRI 
int pmtPayment(word mnuItem) //@agmr - BRI 
{
	int ret;
	word inf = 0;               //info screen to be displayed at the end
	char EntMod;                //how the card was presented: c|C|m|M|k
	char Trk2[lenTrk2 + 1];
	char Pan[lenPan + 1];
	char ExpDat[lenExpDat + 1];
	char tmp[dspW * 2 + 1];
	char *ptr;
	byte TxnType;

	char BlockedPan[lenPan+1];     
	char Srvcod[3+1];
	char CID[lenCID + 1];
	byte ServiceCode;   
	byte EmvTrnType;	
	byte FallBack;

	trcS("pmtPayment Beg\n");

	memset(BlockedPan, '*', sizeof BlockedPan);

    MAPGETBYTE(traTxnType, TxnType, lblDBA);
    
//++ @agmr - multi settle
    switch(TxnType)
    {
        case trtRefund:
        case trtSale:
        case trtInstallment:
        case trtAktivasi:
#ifdef BRIZZI            
        case trtTopUp:
        case trtDeposit:
#endif            
        	ret = batchSync();
        	if(ret < 0)
        	    goto lblStop;
        default:
            break;
    }
//-- @agmr - multi settle	

	ret = valLogIsNotFull();
	CHECK(ret > 0, lblStop);

    txnAsli = 0; //@agmr
    
//++@agmr - periksa inv number dan stan number
//          karena ada kemungkinan di pmtTxnOthers() berubah
{
    card cur, dup;
    
    MAPGETCARD(regInvNum, cur, lblDBA);
    MAPGETCARD(regDupInvNum, dup, lblDBA);
    if(dup > cur)
    {
        if(dup >= 1000000)
            MAPPUTCARD(regInvNum, dup-1000000, lblDBA);   
    }
    MAPPUTCARD(regDupInvNum, 0, lblDBA);                   
}      
//--@agmr - periksa inv number

	MAPGETBYTE(traEntMod, EntMod, lblDBA);
	
	
//++@agmr 
#ifdef PREPAID
    if(isPrepaidTrans(TxnType) == 1)
    {
        ret=prepaidTrans(TxnType);
        if(ret<0)
            goto lblEnd;
        goto lblJump;
    }
#endif
//--@agmr
	
	MAPGETBYTE(appServiceCode, ServiceCode, lblDBA);
	MAPGETBYTE(traFallback, FallBack, lblEnd);
	switch (EntMod)
	{
		case 'M':                //Mag card swiped in idle state
			MAPGET(traTrk2, Trk2, lblDBA);

			//get PAN from Track2
			ptr = Trk2;
			ret = fmtTok(0, ptr, "=");    // Search seperator '='
			CHECK(ret <= lenPan, lblInvalidTrk);
			ret = fmtTok(Pan, ptr, "=");  // Retrieve Pan from track 2
			VERIFY(ret <= lenPan);

			//get Expiry Date from Track2
			ptr += ret;
			VERIFY(*ptr == '=');
			ptr++;
			ret = fmtSbs(ExpDat, ptr, 0, lenExpDat);  // Retrieve expiration date from track 2
			VERIFY(ret == 4);
			MAPPUTSTR(traExpDat, ExpDat, lblDBA);
		   
		   ptr += 4; //For get Service code traSrvCod
		   ret = fmtSbs(Srvcod, ptr, 0, 3); // Retrieve SrvCod from track 2
		   VERIFY(ret == 3);
		   if(ServiceCode == 1){
			   MAPPUTSTR(traSrvCod, Srvcod, lblDBA);
				if((Srvcod[0] == '2')||(Srvcod[0] == '6'))
				  	UseChip = 1;
				else {
					UseChip = 0;
				}
		   }
			strcpy(tmp, Pan);
			strcat(tmp, "=");
			strcat(tmp, ExpDat);
			ret = mapPut(traTrk21, tmp, dspW);
			CHECK(ret >= 0, lblDBA);
			MAPPUTSTR(traTrk22, tmp + dspW, lblDBA);

			MAPPUTSTR(traPan, Pan, lblDBA);
            memcpy(BlockedPan + 12, Pan + 12, lenPan-12); //@agmr
            BlockedPan[sizeof(BlockedPan)-1]=0;	                      //@agmr			   
            MAPPUTSTR(traBlockPan, BlockedPan,lblDBA);   
			break;
		case 'C':                // Smart card inserted in idle state; will be processed as EMV
			break;
		default:
			switch (TxnType)
			{
			    case trtVoidTarikTunai: //@agmr - BRI
				case trtVoid:
					//ret = pmtTxnVoid();
					ret = pmtTxnOthers(TxnType);
					//++ @agmr
				    if(ret == -3)
				        goto lblDeclined;
				    //-- @agmr  
					CHECK(ret > 0, lblDBA);
					break;
				case trtManual:
					ret = pmtManInput();
					if(ret != kbdVAL)
					{
						ret = 0;
						goto lblEnd;
					}
					EntMod = 'k';   //'k'= card manually entered
					MAPPUTBYTE(traEntMod, (byte) EntMod, lblDBA);
					break;
				default:			
					ret = pmtCardInput();   //Card is not swiped yet; organize the dialog
					CHECK(ret > 0, lblStop);
					EntMod = (char) ret;    // 'm'= card swiped, 'c'= card inserted, 'k'= card manually entered
					MAPPUTBYTE(traEntMod, (byte) EntMod, lblDBA);
					break;
			}
			break;
	}

	switch (EntMod)
	{
		case 'f':
			if((TxnType != trtVoid) || (TxnType != trtManual || 
			    TxnType != trtVoidTarikTunai))
			{
				MAPPUTSTR(traChipSwipe, "-FALLBACK",lblStop);
				ret = pmtMagStripe();				
			}

			break;
		case 'm':
		case 'M':                //Payment to be done by Magnetic card
			if((TxnType != trtVoid) || (TxnType != trtManual ||
			    TxnType != trtVoidTarikTunai))
			{
				MAPPUTSTR(traChipSwipe, "-SWIPE", lblStop);
				ret = pmtMagStripe();				
			}
			break;
		case 'c':
		case 'C':                //Payment to be done by Smart card
#ifdef __EMV__
//    	    MAPPUTSTR(traChipSwipe, "-DIP", lblStop); //@agmr
            MAPPUTSTR(traChipSwipe, " (Chip)", lblStop);
    	    emvFullProcessSet();    	   
            ret = pmtSmartCard();
            if(ret < 0)    	 //@agmr
                goto lblEnd;   //@agmr
#endif	 
			if(CheckFallBack(ret) == 1)
			{
				dspClear();
	        		usrInfo(infCardErr);
				//usrInfo(infUseMagStripe);
	        		removeCard();

				MAPPUTBYTE(traEntMod, 'f', lblDBA);
				MAPPUTSTR(traChipSwipe, "-FALLBACK", lblStop);
				pmtSale();
				
				goto lblFinish;
			}
			//End
			if (emvFullProcessCheck() == 0)
			{
				byte emvTrack2Bcd[lenEmvTrk2+ 1];   
				card track2Len;					
				char emvTrack2Hex[lenTrk2+ 1];   	
		      		MEM_ZERO(emvTrack2Hex);										
		      		MEM_ZERO(emvTrack2Bcd);										
		      		mapGet(emvTrk2, emvTrack2Bcd, sizeof(emvTrack2Bcd));        		      
		      		bin2num(&track2Len, &emvTrack2Bcd[0], 1);							
		      		bin2hex(emvTrack2Hex, emvTrack2Bcd+1,track2Len);					
		      		mapPut(traTrk2, emvTrack2Hex, (track2Len*2)-1);						

				MAPGET(traTrk2, Trk2, lblDBA);

				//get PAN from Track2
				ptr = Trk2;
				ret = fmtTok(0, ptr, "D");    // Search seperator '='
				CHECK(ret <= lenPan, lblInvalidTrk);
				ret = fmtTok(Pan, ptr, "D");  // Retrieve Pan from track 2
				VERIFY(ret <= lenPan);

				//get Expiry Date from Track2
				ptr += ret;
				VERIFY(*ptr == 'D');
				ptr++;
				ret = fmtSbs(ExpDat, ptr, 0, lenExpDat);  // Retrieve expiration date from track 2
				VERIFY(ret == 4);
				MAPPUTSTR(traExpDat, ExpDat, lblDBA);

				strcpy(tmp, Pan);
				strcat(tmp, "=");
				strcat(tmp, ExpDat);
				ret = mapPut(traTrk21, tmp, dspW);
				CHECK(ret >= 0, lblDBA);
				MAPPUTSTR(traTrk22, tmp + dspW, lblDBA);

				MAPPUTSTR(traPan, Pan, lblDBA);
                memcpy(BlockedPan + 12, Pan + 12, lenPan-12); //@agmr
                BlockedPan[sizeof(BlockedPan)-1]=0;	                      //@agmr		   
				MAPPUTSTR(traBlockPan, BlockedPan,lblDBA);   

				MAPPUTSTR(traChipSwipe, "-DIP", lblStop);			
				ret = pmtMagStripe(); //@agmr - tambahin ret=
		}
		// END SW 20100824 - NON FULL EMV Handling Text
          break;

      default:                 //Manual PAN input was performed
          if((TxnType != trtVoid) && (TxnType != trtVoidTarikTunai)) {
              CHECK(EntMod == 'k', lblInvalidTrk);
              ret = pmtMagStripe();
          }
          break;
    }
	//AB: return code processing and reversal check to be done here
#ifdef PREPAID
lblJump://@agmr      
#endif	
    CHECK(ret >= 0, lblStop);
    MAPGETBYTE(traTxnType, TxnType, lblDBA); 
    if(TxnType != trtOffline && TxnType != trtPrepaidPayment) {
#if 1
    if((emvFullProcessCheck() == 1) && (mnuItem != mnuVoid) && (mnuItem != mnuVoidTarikTunai))
//	if(emvFullProcessCheck() == 1)
		{
			MAPGET(emvCID, CID, lblDBA);
			MAPGETBYTE(traEmvTrnType, EmvTrnType, lblDBA);
			switch(CID[1] & 0xC0)
			{
				case 0x40:  //offline approval
				    if(EmvTrnType == CID_GEN1){
						TxnType = trtOffline;
						 ret = incCard(regAutCodOff);	//For Increment AutCode number even if its an Offline Transaction
						 CHECK(ret >= 0, lblStop);
				    }
					else if(EmvTrnType == CID_GEN2)
					{
						;//TxnType = trtSale; //@agmr - ditutup
					}
					break;
				case 0x80://online
//++ @agmr
			        ret = valRspCod();
			        CHECK(ret > 0, lblDeclined);
//-- @agmr
					break;
				case 0xC0://Referal
//++ @agmr
					ret = valRspCod();
					CHECK(ret > 0, lblDeclined);
//-- @agmr
					break;
				case 0x00:
//++ @agmr				    
                	if((mnuItem != mnuVoid) && (mnuItem != mnuVoidTarikTunai))
                	{
                		ret = incCard(regInvNum);   //Increment Invoice number / ROC when trx is approved
                		CHECK(ret > 0, lblStop);
                	}
//-- @agmr	                					    
					goto lblDeclined;
			}

		}
		else//End
#endif
		{
	        ret = valRspCod();
	        CHECK(ret > 0, lblDeclined);
		}
		
    } else {
        ret = incCard(regSTAN); //Increment Trace number even if its an Offline Transaction
        CHECK(ret >= 0, lblStop);
    }
    //@agmr - BRI - tukar tempat dengan pmtReceipt
	ret = pmtLogSave();         // Save transaction into log table (Batch)
	CHECK(ret > 0, lblStop);
	ret = pmtReceipt(TxnType);
	CHECK(ret > 0, lblStop);    // Print transaction receipt
	if((mnuItem != mnuVoid) && (mnuItem != mnuVoidTarikTunai))
	{
		ret = incCard(regInvNum);   //Increment Invoice number / ROC when trx is approved
		CHECK(ret > 0, lblStop);
	}

	if(mnuItem == mnuVoid)
		logLoadCurContext(); //<-- @agmr regSTAN menjadi regSTAN-1
    //++ agmr - BRI		
    if(mnuItem == mnuVoidTarikTunai)
        briLogLoadCurContext(TUNAI_LOG);//<-- @agmr regSTAN menjadi regSTAN-1
    if((mnuItem == mnuVoid) || (mnuItem == mnuVoidTarikTunai))
    {
        //redSTAN 
        card dup;
        
        MAPGETCARD(regDupSTAN,dup,lblDBA);
        MAPPUTCARD(regSTAN,dup,lblDBA);
        incCard(regSTAN); //STAN dinaikan lagi
    }
    //-- agmr - BRI		

	ret = 1;
	goto lblEnd;

lblInvalidTrk:
	inf = infInvalidTrack;
	ret = -1;//@agmr - BRI
	goto lblEnd;

lblDBA:
	if(!ret)
		goto lblEnd;
    ret = -2; //@agmr - BRI
	inf = infDataAccessError;
	goto lblEnd;

lblDeclined:
    inf = inf;
    ret = -3; //@agmr - BRI
    goto lblEnd;
lblStop:
    if(!ret)
        goto lblEnd;
    if(ret == -100) //BRI3 sudah menampilkan error
        goto lblEnd;            
    inf = infProcessingError;
    ret = 0;
    goto lblEnd;
lblEnd:
    if(inf)
        usrInfo(inf);
    if(EntMod == 'c' || EntMod == 'C') {
        dspClear();
        usrInfo(infRemoveCard);
        removeCard();
    }
	lblFinish:
	return ret; //@agmr - BRI
    trcS("pmtPayment End\n");
}

/** Sale transaction processing.
 * \header log\\log.h
 * \source log\\pmt.c
 */
void pmtSale(void)
{
	int ret;
	char traName[dspW + 1];

	trcS("pmtSale Beg\n");

	mapMove(rqsBeg, (word) (trtSale - 1));
	MAPGET(rqsDisp, traName, lblKO);
	trcFS("traName: %s\n", traName);

	MAPPUTSTR(traTransType, traName, lblKO);
	MAPPUTSTR(traTypeName, traName, lblKO);
	MAPPUTBYTE(traTxnType, trtSale, lblKO);

	MAPPUTBYTE(regLocType, 'T', lblKO);
	
	pmtPayment(mnuSale);
	goto lblEnd;
lblKO:
lblEnd:
	trcS("pmtSale End\n");
}

/** Preauth transaction processing.
 * \header log\\log.h
 * \source log\\pmt.c
 */
void pmtPreaut(void)
{
	int ret;
	char traName[dspW + 1];

	trcS("pmtPreaut Beg\n");

	mapMove(rqsBeg, (word) (trtPreAut - 1));
	MAPGET(rqsDisp, traName, lblKO);
	trcFS("traName: %s\n", traName);

    MAPPUTBYTE(regLocType, 'T', lblKO); //@agmr
	MAPPUTSTR(traTransType, traName, lblKO);
	MAPPUTBYTE(traTxnType, trtPreAut, lblKO);
	pmtPayment(mnuPreaut);
	goto lblEnd;
lblKO:
lblEnd:
	trcS("pmtPreaut End\n");
}

//++ @agmr - BRI5
void pmtAut(void)
{
	int ret;
	char traName[dspW + 1];

	trcS("pmtAut Beg\n");

	mapMove(rqsBeg, (word) (trtAut - 1));
	MAPGET(rqsDisp, traName, lblKO);
	trcFS("traName: %s\n", traName);
    
    MAPPUTBYTE(regLocType, 'T', lblKO); //@agmr
	MAPPUTSTR(traTransType, traName, lblKO);
	MAPPUTBYTE(traTxnType, trtAut, lblKO);
	pmtPayment(mnuAut);
	goto lblEnd;
lblKO:
lblEnd:
	trcS("pmtAut End\n");
}
//-- @agmr - BRI5

/** Refund transaction processing.
 * \header log\\log.h
 * \source log\\pmt.c
 */
void pmtRefund(void)
{
	int ret;
	char traName[dspW + 1];
	byte LocOpt;

	trcS("pmtRefund Beg\n");

	MAPGETBYTE(appLocOpt, LocOpt, lblKO);
	if(LocOpt & 0x04)
	{
		ret = valAskPwd(appRefPwd);
		CHECK((ret > 0) && (ret != kbdANN), lblKO);
	}

	mapMove(rqsBeg, (word) (trtRefund - 1));
	MAPGET(rqsDisp, traName, lblKO);
	trcFS("traName: %s\n", traName);

    MAPPUTBYTE(regLocType, 'T', lblKO); //@agmr
	MAPPUTSTR(traTransType, traName, lblKO);
	MAPPUTBYTE(traTxnType, trtRefund, lblKO);
	pmtPayment(mnuRefund);
	goto lblEnd;
lblKO:
lblEnd:
	trcS("pmtRefund End\n");
}

/** Offline transaction processing.
 * \header log\\log.h
 * \source log\\pmt.c
 */
void pmtOffline(void)
{         //not to be implemented in prototype
	int ret;
	char traName[dspW + 1];

	trcS("pmtOffline Beg\n");

	mapMove(rqsBeg, (word) (trtOffline - 1));
	MAPGET(rqsDisp, traName, lblKO);
	trcFS("traName: %s\n", traName);

	MAPPUTSTR(traTransType, traName, lblKO);
	MAPPUTBYTE(traTxnType, trtOffline, lblKO);
	pmtPayment(mnuOffline);
	goto lblEnd;
lblKO:
lblEnd:
	trcS("pmtOffline End\n");
}

/** Void transaction processing.
 * \header log\\log.h
 * \source log\\pmt.c
 */
void pmtVoid(void)
{
	int ret;
	char traName[dspW + 1];
	byte LocOpt;

	trcS("pmtVoid Beg\n");

	MAPGETBYTE(appLocOpt, LocOpt, lblKO);
	if(LocOpt & 0x02)
	{
		ret = valAskPwd(appVoidPwd);
		CHECK((ret > 0) && (ret != kbdANN), lblKO);
	}

	mapMove(rqsBeg, (word) (trtVoid - 1));
	MAPGET(rqsDisp, traName, lblKO);
	trcFS("traName: %s\n", traName);

    MAPPUTBYTE(regLocType, 'T', lblKO); //@agmr
	MAPPUTSTR(traTransType, traName, lblKO);
	MAPPUTBYTE(traTxnType, trtVoid, lblKO);
	pmtPayment(mnuVoid);
	goto lblEnd;
lblKO:
lblEnd:
	trcS("pmtVoid End\n");
}

/** Tip adjust processing
 * \header log\\log.h
 * \source log\\pmt.c
 */
void pmtAdjust(void)
{          //not to be implemented in prototype
	int ret;
	char traName[dspW + 1];

	trcS("pmtAdjust Beg\n");

	mapMove(rqsBeg, (word) (trtAdjust - 1));
	MAPGET(rqsDisp, traName, lblKO);
	trcFS("traName: %s\n", traName);

	MAPPUTSTR(traTransType, traName, lblKO);
	//MAPPUTBYTE(traTxnType, trtAdjust, lblKO);
	//pmtPayment(mnuAdjust);
	goto lblEnd;
lblKO:
lblEnd:
	trcS("pmtAdjust End\n");
}

/** Manual transaction processing.
 * \header log\\log.h
 * \source log\\pmt.c
 */
void pmtManual(void)
{
	int ret;
	char traName[dspW + 1];

	trcS("pmtManual Beg\n");

	mapMove(rqsBeg, (word) (trtManual - 1));
	MAPGET(rqsDisp, traName, lblKO);
	trcFS("traName: %s\n", traName);

	MAPPUTSTR(traTransType, traName, lblKO);
	MAPPUTBYTE(traTxnType, trtManual, lblKO);
	goto lblEnd;
lblKO:
lblEnd:
	trcS("pmtManual End\n");
}


/** Function to masking PAN
 *  In Process Printing
 */
void BlockPan(char *origPan,char *blockedPan)
{ 
  int numBeginBlocked=0,numEndBlocked=0,i;
  int len=0,nn=0;
  while(origPan[len]!=0)
  len++;

   mapGet(appPANBlockBegin,&numBeginBlocked,sizeof(int));
   mapGet(appPANBlockEnd,&numEndBlocked,sizeof(int));

   memcpy(blockedPan,origPan,len);
   blockedPan[len] = 0;

   for(i=0;i<len;i++)
    { 
       if(i<numBeginBlocked)
	  blockedPan[i] = 'X';
    }

   for(i=len-1;i>=0;i--)
   	{
   	   if(nn<numEndBlocked)
	    { blockedPan[i] = 'X'; 
	       nn++;
   	    }
   	}
}


//Fallback txn
//Bao add this Function 20100812
int pmtFallback()
{
	int ret;
	
	dspClear();
	//usrInfo(infUseMagStripe);
	removeCard();
	mapPutWord(traMnuItm, mnuSale);
//	MAPPUTBYTE(appFallBackMod, 1, lblKO);
	MAPPUTBYTE(traFallback, 1, lblKO);
	mapPutByte(traEntMod, 'f');
	mapPutStr(traChipSwipe, "-FALLBACK");
	pmtSale();	//Sale is default
//	MAPPUTBYTE(appFallBackMod, 0, lblKO);
	return 1;
	lblKO:
		return 0;
		;
}

/** Preauth transaction processing.
 * \header log\\log.h
 * \source log\\pmt.c
 */
void pmtBalance(void)
{
	int ret;
	char traName[dspW + 1];

	trcS("pmtBalance Beg\n");

	mapMove(rqsBeg, (word) (trtBalance - 1));
	MAPGET(rqsDisp, traName, lblKO);
	trcFS("traName: %s\n", traName);

	MAPPUTSTR(traTransType, traName, lblKO);
	MAPPUTBYTE(traTxnType, trtBalance, lblKO);
	pmtPayment(mnuBalance);
	goto lblEnd;
lblKO:
lblEnd:
	trcS("pmtBalance End\n");
}

// ++ @agmr - BRI

void pmtVoidTarikTunai(void) {
    int ret;
    char traName[dspW + 1];
    byte LocOpt;

    trcS("pmtVoid Beg\n");

#ifdef TEST_VOID
	voidFlag = 1;
#endif

    MAPGETBYTE(appLocOpt, LocOpt, lblKO);
    if(LocOpt & 0x02) {
        ret = valAskPwd(appVoidPwd);
        CHECK((ret > 0) && (ret != kbdANN), lblKO);
    }

    mapMove(rqsBeg, (word) (trtVoidTarikTunai - 1));
    MAPGET(rqsDisp, traName, lblKO);
    trcFS("traName: %s\n", traName);

    MAPPUTSTR(traTransType, traName, lblKO);
    MAPPUTBYTE(traTxnType, trtVoidTarikTunai, lblKO);

    pmtPayment(mnuVoidTarikTunai);
    goto lblEnd;
  lblKO:
  lblEnd:
    trcS("pmtVoid End\n");
}

void pmtBRI(void)
{
    int ret;
    word key, txnType, subType = 0, nextTxn = 0;
    char traName[dspW + 1];
    byte isInquiry = 0;

    trcS("pmtBRI Beg\n");
    
    ret = mapGetWord(traMnuItm, key);
    CHECK(ret >= 0, lblKO);
    VERIFY(key);

    switch(key)
    {
        case mnuInfoSaldo:
            txnType = trtInfoSaldo;
            strcpy(traName,"INFO SALDO");
            break;
        case mnuInfoSaldoBankLain:
            strcpy(traName,"INFO BANK LAIN");
            txnType = trtInfoSaldoBankLain;
            break;
		
		case mnuMiniStatement:
            txnType = trtMiniStatement;
            strcpy(traName,"MINI STATEMENT");
            break;


        case mnuTransferSesamaBRI:
            isInquiry = 1;
            strcpy(traName,"TRNF SESAMA BRI");
            txnType = trtTransferSesamaBRIInquiry;
            nextTxn = trtTransferSesamaBRI;
            break;
        case mnuTransferAntarBank:
            isInquiry = 1;
            strcpy(traName,"TRNF ANTAR BANK");
            txnType = trtTransferAntarBankInquiry;
            nextTxn = trtTransferAntarBank;
            break;
        case mnuInfoKodeBank:
            strcpy(traName,"INFO KODE BANK");
            txnType = trtInfoKodeBank;
            break;
        case mnuPembayaranPLN:
            isInquiry = 1;
            strcpy(traName,"PEMBAYARAN PLN");
            txnType = trtPembayaranPLNPascaInquiry;
            nextTxn = trtPembayaranPLNPasca;
            break;
        case mnuPembayaranCicilanFIF:
            isInquiry = 1;
            strcpy(traName,"CICILAN FIF");
            subType = CICILAN_FIF;
            txnType = trtPembayaranCicilanInquiry;
            nextTxn = trtPembayaranCicilan;
            break;
        case mnuPembayaranCicilanBAF:
            isInquiry = 1;
            strcpy(traName,"CICILAN BAF");
            subType = CICILAN_BAF;
            txnType = trtPembayaranCicilanInquiry;
            nextTxn = trtPembayaranCicilan;
            break;
        case mnuPembayaranCicilanOTO:
            isInquiry = 1;
            strcpy(traName,"CICILAN OTO");
            subType = CICILAN_OTO;
            txnType = trtPembayaranCicilanInquiry;
            nextTxn = trtPembayaranCicilan;
            break;
        case mnuPembayaranCicilanFinansia:
            isInquiry = 1;
            strcpy(traName,"CICILAN FINANSIA");
            subType = CICILAN_FNS;
            txnType = trtPembayaranCicilanInquiry;
            nextTxn = trtPembayaranCicilan;
            break;
        case mnuPembayaranCicilanVerena:
            isInquiry = 1;
            strcpy(traName,"CICILAN VERENA");
            subType = CICILAN_VER;
            txnType = trtPembayaranCicilanInquiry;
            nextTxn = trtPembayaranCicilan;
            break;
        case mnuPembayaranCicilanWOM:
            isInquiry = 1;
            strcpy(traName,"CICILAN WOM");
            subType = CICILAN_WOM;
            txnType = trtPembayaranCicilanInquiry;
            nextTxn = trtPembayaranCicilan;
            break;
        case mnuPembayaranSimpati50:
            strcpy(traName,"PEMB. SIMPATI");
            subType = SIMPATI_50;
            txnType = trtPembayaranPulsa;
            MAPPUTSTR(traAmt,"50000",lblKO);
            break;
        case mnuPembayaranSimpati100:
            strcpy(traName,"PEMB. SIMPATI");
            subType = SIMPATI_100;
            txnType = trtPembayaranPulsa;
            MAPPUTSTR(traAmt,"100000",lblKO);
            break;
 /*       case mnuPembayaranSimpati150:
            strcpy(traName,"PEMB. SIMPATI");
            subType = SIMPATI_150;
            txnType = trtPembayaranPulsa;
            MAPPUTSTR(traAmt,"150000",lblKO);
          break;
  */ 
        case mnuPembayaranSimpati200:
            strcpy(traName,"PEMB. SIMPATI");
            subType = SIMPATI_200;
            txnType = trtPembayaranPulsa;
            MAPPUTSTR(traAmt,"200000",lblKO);
            break;
        case mnuPembayaranSimpati300:
            strcpy(traName,"PEMB. SIMPATI");
            subType = SIMPATI_300;
            txnType = trtPembayaranPulsa;
            MAPPUTSTR(traAmt,"300000",lblKO);
            break;
        case mnuPembayaranSimpati500:
            strcpy(traName,"PEMB. SIMPATI");
            subType = SIMPATI_500;
            txnType = trtPembayaranPulsa;
            MAPPUTSTR(traAmt,"500000",lblKO);
            break;
        case mnuPembayaranSimpati1000:
            strcpy(traName,"PEMB. SIMPATI");
            subType = SIMPATI_1000;
            txnType = trtPembayaranPulsa;
            MAPPUTSTR(traAmt,"1000000",lblKO);
            break;
		 case mnuPembayaranMentari25:
            strcpy(traName,"PEMB. MENTARI");
            subType = MENTARI_25;
            txnType = trtPembayaranPulsa;
            MAPPUTSTR(traAmt,"25000",lblKO);
            break;
        case mnuPembayaranMentari50:
            strcpy(traName,"PEMB. MENTARI");
            subType = MENTARI_50;
            txnType = trtPembayaranPulsa;
            MAPPUTSTR(traAmt,"50000",lblKO);
            break;
        case mnuPembayaranMentari100:
            strcpy(traName,"PEMB. MENTARI");
            subType = MENTARI_100;
            txnType = trtPembayaranPulsa;
            MAPPUTSTR(traAmt,"100000",lblKO);
            break;
        case mnuPembayaranMentari250:
            strcpy(traName,"PEMB. MENTARI");
            subType = MENTARI_250;
            txnType = trtPembayaranPulsa;
            MAPPUTSTR(traAmt,"250000",lblKO);
            break;
        case mnuPembayaranMentari500:
            strcpy(traName,"PEMB. MENTARI");
            subType = MENTARI_500;
            txnType = trtPembayaranPulsa;
            MAPPUTSTR(traAmt,"500000",lblKO);
            break;
        case mnuPembayaranMentari1000:
            strcpy(traName,"PEMB. MENTARI");
            subType = MENTARI_1000;
            txnType = trtPembayaranPulsa;
            MAPPUTSTR(traAmt,"1000000",lblKO);
            break;
        case mnuPembayaranIm325:
            strcpy(traName,"PEMBELIAN IM3");
            subType = IM3_25;
            txnType = trtPembayaranPulsa;
            MAPPUTSTR(traAmt,"25000",lblKO);
            break;
        case mnuPembayaranIm350:
            strcpy(traName,"PEMBELIAN IM3");
            subType = IM3_50;
            txnType = trtPembayaranPulsa;
            MAPPUTSTR(traAmt,"50000",lblKO);
            break;
        case mnuPembayaranIm375:
            strcpy(traName,"PEMBELIAN IM3");
            subType = IM3_75;
            txnType = trtPembayaranPulsa;
            MAPPUTSTR(traAmt,"75000",lblKO);
            break;
        case mnuPembayaranIm3100:
            strcpy(traName,"PEMBELIAN IM3");
            subType = IM3_100;
            txnType = trtPembayaranPulsa;
            MAPPUTSTR(traAmt,"100000",lblKO);
            break;
        case mnuPembayaranIm3150:
            strcpy(traName,"PEMBELIAN IM3");
            subType = IM3_150;
            txnType = trtPembayaranPulsa;
            MAPPUTSTR(traAmt,"150000",lblKO);
            break;
        case mnuPembayaranIm3200:
            strcpy(traName,"PEMBELIAN IM3");
            subType = IM3_200;
            txnType = trtPembayaranPulsa;
            MAPPUTSTR(traAmt,"200000",lblKO);
            break;
        case mnuPembayaranIm3500:
            strcpy(traName,"PEMBELIAN IM3");
            subType = IM3_500;
            txnType = trtPembayaranPulsa;
            MAPPUTSTR(traAmt,"500000",lblKO);
            break;
        case mnuPembayaranIm31000:
            strcpy(traName,"PEMBELIAN IM3");
            subType = IM3_1000;
            txnType = trtPembayaranPulsa;
            MAPPUTSTR(traAmt,"1000000",lblKO);
            break;
        case mnuPembayaranXl25:
            strcpy(traName,"PEMBELIAN XL");
            subType = XL_25;
            txnType = trtPembayaranPulsa;
            MAPPUTSTR(traAmt,"20000",lblKO);
            break;
        case mnuPembayaranXl50:
            strcpy(traName,"PEMBELIAN XL");
            subType = XL_50;
            txnType = trtPembayaranPulsa;
            MAPPUTSTR(traAmt,"50000",lblKO);
            break;
        case mnuPembayaranXl75:
            strcpy(traName,"PEMBELIAN XL");
            subType = XL_75;
            txnType = trtPembayaranPulsa;
            MAPPUTSTR(traAmt,"75000",lblKO);
            break;
        case mnuPembayaranXl100:
            strcpy(traName,"PEMBELIAN XL");
            subType = XL_100;
            txnType = trtPembayaranPulsa;
            MAPPUTSTR(traAmt,"100000",lblKO);
            break;
        case mnuPembayaranXl150: 
            strcpy(traName,"PEMBELIAN XL");
            subType = XL_150;
            txnType = trtPembayaranPulsa;
            MAPPUTSTR(traAmt,"150000",lblKO);
            break;
        case mnuPembayaranXl200:
            strcpy(traName,"PEMBELIAN XL");
            subType = XL_200;
            txnType = trtPembayaranPulsa;
            MAPPUTSTR(traAmt,"200000",lblKO);
            break;
        case mnuPembayaranXl300:
            strcpy(traName,"PEMBELIAN XL");
            subType = XL_300;
            txnType = trtPembayaranPulsa;
            MAPPUTSTR(traAmt,"300000",lblKO);
            break;
        case mnuPembayaranXl500:
            strcpy(traName,"PEMBELIAN XL");
            subType = XL_500;
            txnType = trtPembayaranPulsa;
            MAPPUTSTR(traAmt,"500000",lblKO);
            break;
        case mnuPembayaranZakatDhuafa:
            strcpy(traName,"PEMB. ZAKAT");
            subType = ZAKAT_DHUAFA;
            txnType = trtPembayaranZakat;
            break; 
        case mnuPembayaranZakatYBM:
            strcpy(traName,"PEMB. ZAKAT");
            subType = ZAKAT_YBM;
            txnType = trtPembayaranZakat;
            break;  
        case mnuPembayaranInfaqDhuafa:
            strcpy(traName,"PEMB. INFAQ");
            subType = INFAQ_DHUAFA;
            txnType = trtPembayaranZakat;
            break; 
        case mnuPembayaranInfaqYBM:
            strcpy(traName,"PEMB. INFAQ");
            subType = INFAQ_YBM;
            txnType = trtPembayaranZakat;
            break;  
        case mnuPembayaranDPLKDhuafa:
            strcpy(traName,"PEMB. DPLK");
            subType = DPLK_DHUAFA;
            txnType = trtPembayaranZakat;
            break; 
        case mnuPembayaranDPLKYBM:
            strcpy(traName,"PEMB. DPLK");
            subType = DPLK_YBM;
            txnType = trtPembayaranZakat;
            break;
        case mnuSetorSimpanan:
//            MAPPUTSTR(acqNII,"011",lblKO);
            strcpy(traName,"SETOR SIMPANAN");
            isInquiry = 1;
            txnType = trtSetorSimpananInquiry;
            nextTxn = trtSetorSimpanan;
            break;
        case mnuSetorPinjaman:
//            MAPPUTSTR(acqNII,"011",lblKO);
            strcpy(traName,"SETOR PINJAMAN");
            isInquiry = 1;
            txnType = trtSetorPinjamanInquiry;
            nextTxn = trtSetorPinjaman;
            break;
        case mnuTarikTunai:
//            MAPPUTSTR(acqNII,"011",lblKO);
            strcpy(traName,"TARIK TUNAI");
            txnType = trtTarikTunai;
            break;
        case mnuVoidTarikTunai:
//            MAPPUTSTR(acqNII,"011",lblKO);
            strcpy(traName,"VOID TRK TUNAI");
            txnType = trtVoidTarikTunai;
            break;
        case mnuRegistrasiInternetBanking:
            strcpy(traName,"REG INT BANKING");
            txnType = trtRegistrasiInternetBanking;
            break;
        case mnuRegistrasiPhoneBanking:
            strcpy(traName,"REG PHN BANKING");
            txnType = trtRegistrasiPhoneBanking;
            break;  
        case mnuGantiPIN:
            strcpy(traName,"UBAH PIN");
            txnType = trtGantiPIN;
            break;
        case mnuRenewalKartu:
            strcpy(traName,"RENEWAL KARTU");   
//            isInquiry = 1;
            txnType = trtRenewalKartuOtorisasi;
//            nextTxn = trtRenewalKartuInquiry;
            break;
        case mnuAktivasiKartu:
            strcpy(traName,"AKTIVASI KARTU");
//            isInquiry = 1;
            txnType = trtAktivasiKartuOtorisasi;
//            nextTxn = trtAktivasiKartuInquiry;
            break;
        case mnuReissuePIN:
            strcpy(traName,"REISSUE PIN");
//            isInquiry = 1;
            txnType = trtReissuePINOtorisasi;
//            nextTxn = trtReissuePIN;
            break;
        case mnuGantiPasswordSPV:
            strcpy(traName,"UBAH PASS SPV");
            txnType = trtGantiPasswordSPV;
            break;            
        case mnuInstallment:
            //++@agmr
//            if(checkPendingSettlement()!=0)
//                return;
            //--@agmr
            strcpy(traName,"CICILAN BRING");
            txnType = trtInstallment;
            break;
         case mnuPembayaranHalo:
            strcpy(traName,"HALO");
            isInquiry = 1;
            subType = PASCA_BAYAR_HALO;
            txnType = trtPembayaranPascaBayarInquiry;
            nextTxn = trtPembayaranPascaBayar;
            break;
         case mnuPembayaranMatrix:
            strcpy(traName,"MATRIX");
            isInquiry = 1;
            subType = PASCA_BAYAR_MATRIX;
            txnType = trtPembayaranPascaBayarInquiry;
            nextTxn = trtPembayaranPascaBayar;
            break;            
            
    }
    
    while(1)
    {
        txnAsli = 0;
    	mapPut(traRspCod, "", 2);
    	MAPPUTBYTE(regLocType,'T',lblKO);
        MAPPUTBYTE(traSubType,subType,lblKO);
        MAPPUTBYTE(traInquiryFlag,isInquiry,lblKO);
        MAPPUTBYTE(traNextTxn,nextTxn,lblKO);
        mapMove(rqsBeg, (word) (txnType - 1));
        trcFS("traName: %s\n", traName);
    
        MAPPUTSTR(traTransType, traName, lblKO);
        MAPPUTSTR(traTypeName, traName, lblKO);
        MAPPUTBYTE(traTxnType, txnType, lblKO);
        if( pmtPayment(key) != 1) //@agmr - BRI
            goto lblEnd;  
        
        nextTxn = 0;
        isInquiry = 0;
        switch(txnType)
        {
            case trtRenewalKartuOtorisasi:           
                strcpy(traName,"RENEWAL KARTU");   
                txnType = trtRenewalKartuInquiry;
                break;
            case trtRenewalKartuInquiry:            
                strcpy(traName,"RENEWAL KARTU");   
                txnType = trtRenewalKartu;                 
                break;                  
            case trtAktivasiKartuOtorisasi:
                strcpy(traName,"AKTIVASI");   
                isInquiry = 1;
                txnType = trtAktivasiKartuInquiry;
                nextTxn = trtAktivasiKartu;              
                break;
            case trtReissuePINOtorisasi:
                strcpy(traName,"REISSUE PIN");   
                txnType = trtReissuePIN;                  
                break;
            default:
                goto lblEnd;      
                                                                          
        }            
    }

    goto lblEnd;
  lblKO:
  lblEnd:
    trcS("pmtBRI End\n");       
}

#if 0
int magStripeInput(void)
{
	int ret;
	word key;
	byte PanDisplay;
	char Amt[lenAmt + 1];
	byte txnType;    
	
	trcS("magStripeInput Beg\n");
	//Bao add for EMV
	if(UseChip == 1){
		dspClear();
		usrInfo(infEMVChipDetect);
		Beep();
		ret = 0;
		goto lblEnd;
	}	
    ret = valCard();            // Track2 analysis 
    CHECK(ret > 0, lblKO);

	MAPGETBYTE(appTermOpt1, PanDisplay, lblKO);
	if(PanDisplay & 0x02)
	{
		ret = usrInfo(infConfirmPan);
		CHECK(ret >= 0, lblKO);
		CHECK(ret == kbdVAL, lblExit);
	}
	ret = 1;
	goto lblEnd;

lblExit:                     //smooth exit
	ret = 0;
	goto lblEnd;
lblKO:
	goto lblEnd;
lblEnd:
	trcFN("pmtMagStripe: ret=%d\n", ret);
	return ret;	
}


int smartCardInput()
{
    int ret;
    
#ifdef __EMV__
    MAPPUTSTR(traChipSwipe, "-DIP", lblStop);
	emvFullProcessSet();
   
    ret = pmtSmartCard();
#endif	 
	if(CheckFallBack(ret) == 1)
	{
		dspClear();
    		usrInfo(infCardErr);
		//usrInfo(infUseMagStripe);
    		removeCard();

		MAPPUTBYTE(traEntMod, 'f', lblDBA);
		MAPPUTSTR(traChipSwipe, "-FALLBACK", lblStop);
//		pmtSale();
		
		goto lblFinish;
	}
	//End
	if (emvFullProcessCheck() == 0)
	{
		byte emvTrack2Bcd[lenEmvTrk2+ 1];   
		card track2Len;					
		char emvTrack2Hex[lenTrk2+ 1];   	
      		MEM_ZERO(emvTrack2Hex);										
      		MEM_ZERO(emvTrack2Bcd);										
      		mapGet(emvTrk2, emvTrack2Bcd, sizeof(emvTrack2Bcd));        		      
      		bin2num(&track2Len, &emvTrack2Bcd[0], 1);							
      		bin2hex(emvTrack2Hex, emvTrack2Bcd+1,track2Len);					
      		mapPut(traTrk2, emvTrack2Hex, (track2Len*2)-1);						

		MAPGET(traTrk2, Trk2, lblDBA);

		//get PAN from Track2
		ptr = Trk2;
		ret = fmtTok(0, ptr, "D");    // Search seperator '='
		CHECK(ret <= lenPan, lblInvalidTrk);
		ret = fmtTok(Pan, ptr, "D");  // Retrieve Pan from track 2
		VERIFY(ret <= lenPan);

		//get Expiry Date from Track2
		ptr += ret;
		VERIFY(*ptr == 'D');
		ptr++;
		ret = fmtSbs(ExpDat, ptr, 0, lenExpDat);  // Retrieve expiration date from track 2
		VERIFY(ret == 4);
		MAPPUTSTR(traExpDat, ExpDat, lblDBA);

		strcpy(tmp, Pan);
		strcat(tmp, "=");
		strcat(tmp, ExpDat);
		ret = mapPut(traTrk21, tmp, dspW);
		CHECK(ret >= 0, lblDBA);
		MAPPUTSTR(traTrk22, tmp + dspW, lblDBA);

		MAPPUTSTR(traPan, Pan, lblDBA);
        memcpy(BlockedPan + 12, Pan + 12, lenPan-12); //@agmr
        BlockedPan[sizeof(BlockedPan)-1]=0;	                      //@agmr		   
		MAPPUTSTR(traBlockPan, BlockedPan,lblDBA);   

		MAPPUTSTR(traChipSwipe, "-DIP", lblStop);			
		pmtMagStripe();
    }    
}

#endif

int fmtAmtReceipt2(word key, char *amt)
{
    return(fmtAmtReceipt(key, amt));
}
/*
int inquiryToPaymentData(void)
{
    byte txnType;
    byte amount[15];
    int ret;
    byte nextTxn = 0;
    
    MAPGETBYTE(traTxnType, txnType, lblKO);
    if(txnType == 0)
        return 0;
        
    if(txnType == trtAktivasiKartuRenewalInquiry)
    {
        nextTxn = trtAktivasiKartuRenewal;
        MAPPUTBYTE(traInquiryFlag,1,lblKO); //3 kali transaksi
    }
    else
    {
        MAPGETBYTE(traNextTxn, nextTxn, lblKO);
        if(nextTxn == 0)
            return 0;
    }

    MAPPUTBYTE(traTxnType, nextTxn, lblKO);

//    MAPPUTBYTE(traMnuItm,mnuInquiryConfirmation,lblKO);
    mapPut(traAmt,amount,12);    
    return 1;
    
  lblKO:
    return -1;    
}
*/

//void pmtInquiryTrans(void) {         
//    int ret;
//    char traName[dspW + 1];
//
//    trcS("pmtInquiryTrans Beg\n");
//
//    MAPPUTBYTE(traInquiryFlag,1,lblKO);
//    mapMove(rqsBeg, (word) (trtInquiryTrans - 1));
//    MAPGET(rqsDisp, traName, lblKO);
//    trcFS("traName: %s\n", traName);
//
//    MAPPUTSTR(traTransType, traName, lblKO);
//    MAPPUTBYTE(traTxnType, trtInquiryTrans, lblKO);
//    pmtPayment(mnuInquiry);
//    goto lblEnd;
//  lblKO:
//  lblEnd:
//    trcS("pmtInquiryTrans End\n");
//}

// -- @agmr - BRI
