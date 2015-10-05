//Subversion reference
//$HeadURL: https://svn.ingenico.com/UTAPROJECTS/sea-partner/5W/trunk/ISO5WPH/log/req.c $
//$Id: req.c 1944 2009-04-28 10:40:17Z ajloreto $

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "log.h"
#include "iso.h"
#include "emv.h"
#include "str.h"
#include "BRI.h"
#include "LineEncryption.h"  //@@AS0.0.10
//#include "prepaid.h"

#define ISO_BUFFER_LEN 2048 //@agmr

#define CHK CHECK(ret>=0,lblKO)
static char reqtyp = 'A';       //'A': authorization; 'S': settlement; 'P': parameters download(TMS)
static byte LocationType;       //location on where to get certain information?     // 'L' - log

// ++ @agmr - BRI - TLE
#ifdef LINE_ENCRYPTION
#define STORE_TLE_FIELD 57
//unsigned char encryptedField[] = {2,4,48,0};
#endif

byte txnAsli = 0; //@agmr - BRI2 untuk menyimpan txn aslinya, digunakan sesudah proses reversal;

// ++ @agmr - BRI
int getIso57(tBuffer * val);
// -- @agmr - BRI

enum eDnlSta {
    autBeg,                     //engine states: authorisation
    autSendReq,
    autRecvRsp,
    autEnd,

    dnlBeg,                     //engine states: TMS parameters download
    dnlSendLoginReq,
    dnlRecvLoginRsp,
    dnlSendConTblReq,
    dnlRecvConTblRsp,
    dnlSendBinTblReq,
    dnlRecvBinTblRsp,
    dnlSendIssTblReq,
    dnlRecvIssTblRsp,
    dnlSendAcqTblReq,
    dnlRecvAcqTblRsp,
    dnlSendEmvReq,
    dnlRecvEmvRsp,
    dnlSendEmvFileReq,
    dnlRecvEmvFileRsp,
    dnlSendEmvFileOpnReq,
    dnlRecvEmvFileOpnRsp,
    dnlSendEmvFileRedReq,
    dnlRecvEmvFileRedRsp,
    dnlSendEmvFileClsReq,
    dnlRecvEmvFileClsRsp,
    dnlSendLogOffReq,
    dnlRecvLogOffRsp,
    dnlEnd
};

static int getVal(tBuffer * val, word key)
{
	int ret;
	byte buf[256];
	char bcd[lenExpDat / 2];

	VERIFY(val);
	ret = mapGet(key, buf, 256);
	CHK;

	if(key == traExpDat || key == logExpDat){//bit 14 reversal & TC
		hex2bin(bcd, buf, 0);
		ret = bufApp(val, bcd, 2);CHK;	
		return ret;
	}
	VERIFY(strlen((char *) buf) <= 256);
	bufReset(val);
	ret = bufApp(val, buf, bufLen(val));
	CHK;
	return ret;
lblKO:
	return -1;
}

//++@agm - BRI
static int getCurrencyCode(tBuffer * val, word key) {
    int ret;
    byte buf[256];

    VERIFY(val);
    ret = mapGet(key, buf, 256);
//ShowData(buf,8,0,1,8);    
    CHK;
    VERIFY(strlen((char *) buf) <= 256);
    bufReset(val);
    ret = bufApp(val, buf+1, 2);
    CHK;
    return ret;
  lblKO:
    return -1;
}
//--@agmr - BRI

// ++ @agm - tambahan untuk perbaikan expiry date
/*
static int getExpiryDate(tBuffer * val) {
    int ret;
    byte buf[256];
    char bcdBuf[50];
    byte bRet;

    VERIFY(val);
    ret = mapGet(traExpDat, buf, 256);
    CHK;
    VERIFY(strlen((char *) buf) <= 256);
    bufReset(val);
    
    bRet = hex2bin(bcdBuf,buf,sizeof(bcdBuf));
    ret = bufApp(val, bcdBuf,bRet);
    CHK;
    
    return ret;
  lblKO:
    return -1;
}
*/
// -- @agmr - tambahan untuk perbaikan expiry date

static int getPrcCod(tBuffer * val)
{
	int ret;
	byte PrcCod[1 + lenPrcCod];
	word key;
    byte tmp = 0, tmp2 = 0, jenis; //@agmr - BRI - Processing code
    byte transTypeOri; //@agmr
    
	VERIFY(val);

	switch(LocationType)
	{
		case 'R':
			key = revPrcCod;
			break;
		default:
			key = rqsPrcCod;
			break;
    }
	ret = mapGet(key, PrcCod, sizeof(PrcCod));
	CHK;

	trcS("getPrcCod:");
	trcBN(PrcCod + 1, lenPrcCod);
	trcS("\n");

// ++ @agmr - BRI - processing code
    MAPGETBYTE(traTxnType,tmp,lblKO);
    MAPGETBYTE(traJenisRekening,jenis,lblKO);
    //MAPPUTBYTE(traTxnTypeTempSale,0,lblKO);
    switch(tmp)
    {
        case trtTransferAntarBankInquiry:
            if(jenis == REK_TABUNGAN)
                memcpy(PrcCod, "\x03\x30\x40\x00",4);
            else
                memcpy(PrcCod, "\x03\x30\x40\x20",4);               
            break;
        case trtTransferAntarBank:
            if(jenis == REK_TABUNGAN)
                memcpy(PrcCod, "\x03\x40\x10\x00",4);
            else
                memcpy(PrcCod, "\x03\x40\x20\x00",4);                 
                break;
        case trtInfoSaldoBankLain:
            if(jenis == REK_TABUNGAN)
                memcpy(PrcCod, "\x03\x30\x10\x00",4);  
            else
                memcpy(PrcCod, "\x03\x30\x20\x00",4);  
                break;
	 case trtSaleRedeem: //@ar - BRI
	 	MAPGETBYTE(traKategoriKartu,jenis,lblKO);
            if((jenis == DEBIT_CARD) || (jenis == NON_BRI_DEBIT_CARD))
            {
            	memcpy(PrcCod, "\x03\x00\x10\x00",4);
            }
            else if(jenis == CREDIT_CARD)
		 			memcpy(PrcCod, "\x03\x00\x00\x00",4);
            MAPPUTBYTE(traTxnTypeTempSale,trtSaleRedeem,lblKO);
            break;
        case trtSale:
        	MAPGETBYTE(traKategoriKartu,jenis,lblKO);
            if((jenis == DEBIT_CARD) || (jenis == NON_BRI_DEBIT_CARD))
            {
            	memcpy(PrcCod, "\x03\x00\x10\x00",4);
            }
            else if(jenis == CREDIT_CARD)
		 			memcpy(PrcCod, "\x03\x00\x00\x00",4);
            MAPPUTBYTE(traTxnTypeTempSale,trtSale,lblKO);
            break;
        case trtInstallment:
            MAPGETBYTE(traKategoriKartu,jenis,lblKO);
            if(jenis == DEBIT_CARD || jenis == NON_BRI_DEBIT_CARD)
            {
            	memcpy(PrcCod, "\x03\x00\x10\x00",4);
            }
		break;
        case trtRefund:
            MAPGETBYTE(traKategoriKartu,jenis,lblKO);
            if(jenis == DEBIT_CARD || jenis == NON_BRI_DEBIT_CARD)
            {
            	memcpy(PrcCod, "\x03\x20\x10\x00",4);
            }
            break; 
            
        case trtVoid:
            //valBin();  
            MAPGETBYTE(traKategoriKartu,jenis,lblKO);
            if(jenis == DEBIT_CARD || jenis == NON_BRI_DEBIT_CARD)
            {
            	MAPGETBYTE(traOriginalTxnType, transTypeOri, lblKO);
//                MAPGETWORD(logMnuItm,mnuItm,lblKO);
                if((transTypeOri == trtSale) || (transTypeOri == trtSaleRedeem)) //@ar - BRI
                    memcpy(PrcCod, "\x03\x02\x10\x00",4);  
                else if(transTypeOri == trtRefund)
                    memcpy(PrcCod, "\x03\x20\x10\x00",4);
            }            
            break;             
                
//#ifdef PREPAID
//        case trtPrepaidSettlement:
//        {
//            card count;
//            word idx;
//            
//        	MAPGETCARD(totAppSaleCnt, count, lblKO);
//        	MAPGETWORD(traIdx, idx, lblKO);
//        	idx--;
//        	
//        	if(idx == count)
//        	    memcpy(PrcCod, "\x03\x80\x81\x88",4);
//        	else
//                 memcpy(PrcCod, "\x03\x80\x81\x00",4);
//        }
//#endif                                              
        default:
            break;        
    }
	if(key == revPrcCod)
	{
		if (tmp == trtReversal)
		{
			MAPGETBYTE(traTxnTypeTempSale,tmp2,lblKO);
			if((tmp2 == trtSale) || (tmp2 == trtSaleRedeem))
			{
        			MAPGETBYTE(traKategoriKartu,jenis,lblKO);
            			if((jenis == DEBIT_CARD) || (jenis == NON_BRI_DEBIT_CARD))
            			{
            				memcpy(PrcCod, "\x03\x00\x10\x00",4);
            			}
            			else if(jenis == CREDIT_CARD)
            				memcpy(PrcCod, "\x03\x00\x00\x00",4);
			}
		}
	}

// -- @agmr - BRI - processing code



	ret = bufApp(val, PrcCod + 1, lenPrcCod);
	CHK;

	//ret = bufApp(val, "\x00\x00\x00", 3); CHK;

	return ret;
lblKO:
	return -1;
}

static int getAmt(tBuffer * val)
{
	int ret;
	char amt[lenAmt + 1];
	char Tipamt[lenAmt + 1];
	byte bcd[lenAmtBcd];
	word Amtkey, TipAmtkey;
	byte AmtExp, txnType;
//	byte trtType; //@agmr - BRI

	VERIFY(val);

	switch (LocationType)
	{
		case 'R':
			Amtkey = revAmt;
			TipAmtkey = revTipAmt;
			break;
		case 'L':
			Amtkey = logTraAmt;
			TipAmtkey = logTipAmt;
			break;
		case 'U':
			Amtkey = TCAmt;
			TipAmtkey = TCTipAmt;
			break;
		case 'T':
		default:
			Amtkey = traAmt;
			TipAmtkey = traTipAmt;
			break;
	}

	MAPGET(Amtkey, amt, lblKO);
	MAPGET(TipAmtkey, Tipamt, lblKO);

	addStr(amt, amt, Tipamt);

//++ @agmr - BRI
//    MAPGETBYTE(traTxnType,trtType,lblKO);
//    if(trtType == trtPembayaranPLNPasca)
//        fmtPad(amt, -lenAmt, '0');
//    else
//    {
//-- @agmr - BRI    
	MAPGETBYTE(appExp, AmtExp, lblKO);
 	MAPGETBYTE(traTxnType,txnType,lblKO);
	if((txnType == trtPembayaranTelkom) || (txnType == trtPembayaranTelkomInquiry)||
	   (txnType == trtPembayaranPLNPasca) || (txnType == trtPembayaranPLNPascaInquiry)||
	   (txnType == trtPembayaranPLNToken) 
	   // || (txnType == trtPembayaranBriva)
	   // || (txnType == trtPembayaranBrivaInquiry)
	   ||(txnType == trtPembayaranPdamInquiry)
	   ||(txnType == trtPembayaranPdam)
	   ||(txnType == trtPembayaranSSB) || (txnType == trtPembayaranSSBInquiry)
      ){
		fmtPad(amt, -lenAmt, '0');
	}else{
       	if(AmtExp == 0){
	     		fmtPad(amt, -10, '0');
	     		//memcpy(amt + 10, "00", 2); ==>permintaan Friska
       	}
		else
       		fmtPad(amt, -lenAmt, '0');
       } //@agmr - BRI   
    	hex2bin(bcd, amt, lenAmt);
	   
	trcS("getAmt: ");
	trcBN(bcd, sizeof(bcd));
	trcS("\n");

	ret = bufApp(val, bcd, lenAmtBcd);
	CHK;

	return ret;
lblKO:
	return -1;
}

static int getDatTim(tBuffer * val)
{
	int ret;
	char datTim[lenDatTim + 1];
	byte bcd[(lenDatTim - 4) / 2];  //remove YY and cc of dateTime
	word key;

	VERIFY(val);

	switch (LocationType)
	{
		case 'L':
			key = logDatTim;
			break;
		case 'U':
			key = TCDatTim;
			break;
		case 'T':
		default:
			key = traDatTim;
			break;
	}
	ret = mapGet(key, datTim, sizeof(datTim));
	CHK;

	hex2bin(bcd, datTim + 2, 0);    //remove the first 2 and last 2 char in DateTime containing YY & cc

	trcS("getDatTim: ");
	trcBN(bcd, sizeof(bcd));
	trcS("\n");

	ret = bufApp(val, bcd, sizeof(bcd));
	CHK;
	return ret;
lblKO:
	return -1;
}

static int getTim(tBuffer * val)
{
	int ret;
	char Tim[lenDatTim + 1];
	byte bcd[(lenDatTim - 8) / 2];  //remove YYMMDD and cc of dateTime
	word key;

	VERIFY(val);

	switch (LocationType)
	{
		case 'R':
			key = revDatTim;
			break;
		case 'L':
			key = logDatTim;
			break;
		case 'U':
			key = TCDatTim;
			break;
		case 'T':
		default:
			key = traDatTim;
			break;
	}
	ret = mapGet(key, Tim, sizeof(Tim));
	CHK;

	hex2bin(bcd, Tim + 8, 0);   //remove the first 6 and last 2 char in DateTime containing YYMMDD & cc

	trcS("getTim: ");
	trcBN(bcd, sizeof(bcd));
	trcS("\n");

	//ret = bufApp(val, "\x10\x56\x14", 3); CHK;
	ret = bufApp(val, bcd, sizeof(bcd));

	CHK;
	return ret;
lblKO:
	return -1;
}

static int getDat(tBuffer * val)
{
	int ret;
	char Dat[lenDatTim + 1];
	byte bcd[(lenDatTim - 10) / 2]; //remove YY and hhmmsscc of dateTime
	word key;

	VERIFY(val);

	switch (LocationType)
	{
		case 'R':
			key = revDatTim;
			break;
		case 'L':
			key = logDatTim;
			break;
		case 'U':
			key = TCDatTim;
			break;
		case 'T':
		default:
			key = traDatTim;
			break;
	}
	ret = mapGet(key, Dat, sizeof(Dat));
	CHK;

	hex2bin(bcd, Dat + 4, 0);   //remove the first 2 and last 8 char in DateTime containing YY & hhmmsscc

	trcS("getDat: ");
	trcBN(bcd, sizeof(bcd));
	trcS("\n");

	//ret = bufApp(val, "\x12\x03", 3); CHK;
	ret = bufApp(val, bcd, sizeof(bcd));

	CHK;
	return ret;
lblKO:
	return -1;
}

static int getSTAN(tBuffer * val)
{
	int ret;
	card STAN;
	char dec[2 * lenSTAN + 1];
	byte bcd[lenSTAN];
	word key;
	byte txnType; //@agmr

	VERIFY(val);

	switch (LocationType)
	{
		case 'R':
			key = revSTAN;
//			key = regSTAN; //@agmr - BRI - auto reversal - Stan harus naik terus
			break;
		case 'L':
			key = logSTAN;
			break;
		case 'T':
		case 'U':	
		default:
			key = regSTAN;
			break;
	}
//++ @agmr
	MAPGETBYTE(traTxnType,txnType,lblKO);
	if(txnType == trtSettlement)
		key = regSTAN;
//-- @agmr
	ret = mapGetCard(key, STAN);
	CHK;

	trcFN("getSTAN: %d\n", STAN);

	num2dec(dec, STAN, 2 * lenSTAN);
	hex2bin(bcd, dec, 0);
	ret = bufApp(val, bcd, lenSTAN);
	CHK;

	return ret;
lblKO:
	return -1;
}

static int getCardSeq(tBuffer * val)
{
	int ret;
	char crdSeq[lenCrdSeq + 1];
	byte bcd[lenCrdSeqBCD];
	char panSeq[2+1];
	

	if(LocationType == 'U')
	{
		MAPGET(TCPanSeq, panSeq, lblKO);
		return bufApp(val, panSeq, 2);
	}

	ret = mapGet(emvPANSeq, crdSeq, sizeof(crdSeq));                     
	CHK;
	MEM_ZERO(bcd);
	bcd[1] = crdSeq[1];

	trcS("getCardSeq: ");
	trcBN(bcd, sizeof(bcd));
	trcS("\n");

	ret = mapPut(traPanSeq, bcd, 2);
	CHK;
		
	ret = bufApp(val, bcd, lenCrdSeqBCD);
	CHK;
	
	return ret;
lblKO:
	return -1;
}

/*
static int getPosEntMod(tBuffer * val)
{
	int ret;
	char entMod;
	char POSE[lenPOSE];			
	byte ab;					
	byte pinOpt;				
	byte bcd[lenPOSEBCD];		
	word key;					
	

	VERIFY(val);
	memset(POSE, 0x00, sizeof(POSE));

	switch (LocationType)
	{
		case 'R':
			key = revEntMod;
			break;
		case 'L':
			key = logEntMod;
			break;
		case 'T':
		default:
			key = traEntMod;
			break;
	}
	ret = mapGetByte(key, entMod);
	CHK;

	switch (entMod)
	{
		case 'k':
		case 'K':
			strcat(POSE, "01");
			break;
		case 'c':
		case 'C':
			strcat(POSE, "05");
			break;
		case 'm':
		case 'M':
			strcat(POSE, "02");
			break;
		default:
			strcat(POSE, "00");
			break;
	}

	ret = mapGetByte(logIssIdx, ab);
	CHK;

	ret = mapMove(issBeg, (word) ab);
	CHK;

	ret = mapGetByte(issPinOpt, pinOpt);
	CHK;

	switch (pinOpt)
	{
		case '0':                //No Pin
			strcat(POSE, "20");
			break;
		case '2':                //required
			strcat(POSE, "10");
			break;
		case '1':                //Optional
		default:
			strcat(POSE, "00");
			;
			break;
	}
	hex2bin(bcd, POSE, 0);

	trcS("getPosEntMod: ");
	trcBN(bcd, sizeof(bcd));
	trcS("\n");

	ret = bufApp(val, bcd, lenPOSE);
	//ret = bufApp(val, (byte *) "\x00\x20", 2);
	CHK;

	return ret;
lblKO:
	return -1;
}
*/



static int getPosEntMod(tBuffer * val) {                
	int ret;
	char EntMod;
	char POSE[2+1];
	word key;
	byte		res;


	VERIFY(val);
	memset(POSE, 0x00, sizeof(POSE));

	switch(LocationType)
	{
		case 'U':
			key = TCEntMod;
			break;
		case 'L':
			key = logEntMod;
			break;
		default:
			key = traEntMod;
			break;
	}
	
	MAPGETBYTE(key, EntMod, lblKO);

	switch(EntMod){
		case 'f':
			ret = bufApp(val, (byte *) "\x08\x01", 2);//For Fallback
			CHK;
			break;
#ifdef PREPAID
        case 't':
        case 'T':
#endif			
		case 'm':
		case 'M':
			ret = bufApp(val, (byte *) "\x09\x01", 2);
			CHK;
		    break;
		case 'c':
		case 'C':
			MAPGETBYTE(traPinReq,res,lblKO);
			if(res==1)
			{
				ret = bufApp(val, (byte *) "\x00\x51", 2);
				mapPutCard(traField22Len,8);
				mapPut(traField22,(byte *) "\x00\x51",2);
			}
			else
			{
				ret = bufApp(val, (byte *) "\x00\x52", 2);
				mapPutCard(traField22Len,2);
				mapPut(traField22,(byte *) "\x00\x52",2);
			}
			break;
		case 'k':
		case 'K':
			ret = bufApp(val, (byte *) "\x00\x12", 2);
			break;
		default:
			break;			
	}

	
	return ret;
	
	lblKO: return -1;
}


static int getNII(tBuffer * val)
{
	int ret;
	char Nii[lenNII + 1];
	byte bcdNii[lenNii];
	word key;

	VERIFY(val);
	key = ((reqtyp == 'P') ? appNII : acqNII);
	ret = mapGet(key, Nii, sizeof(Nii));
	CHK;
	
#ifdef PREPAID
    ret = getPrepaidNii(Nii);
    CHECK(ret > 0,lblKO);
    
#endif	
	
#ifndef REVERSAL_TERPISAH
    if(reqtyp == 'R')
    {
        char buf[lenNII+1];
        
        memset(buf,0,sizeof(buf));
        ret = mapGet(revNII,buf,lenNII);
        CHK;
        if(buf[0]!=0) //transaksi MINI ATM, TUNAI ATAU CARD SERVICE
        {
            memcpy(Nii,buf,lenNII);
        }
    }
//    else
//    {
//        ret = getJenisLog();
//        if(ret > 0) //trans Mini ATM, Tunai dan Card Service
//        {
//            ret = mapPut(traRevNII,Nii,lenNII);
//            CHK;
//        }
//        else
//        {
//            ret = mapPut(traRevNII,"\x00\x00\x00",lenNII);
//            CHK;
//        }
//    }
#endif	
	
	fmtPad(Nii, -(lenNII + 1), '0');
	hex2bin(bcdNii, Nii, 0);

	trcS("getNII: ");
	trcBN(bcdNii, sizeof(bcdNii));
	trcS("\n");

	ret = bufApp(val, bcdNii, lenNii);
	CHK;

	//ret = bufApp(val, "\x02\x00\x64", 2); CHK;

	return ret;
lblKO:
	return -1;
}

static int getPosCndCod(tBuffer * val)
{
	int ret;
	byte POSC = 0, txnType;

	switch (LocationType)
	{
		case 'L':
			MAPGETBYTE(logConCode, POSC, lblEnd);
			break;
		case 'T':                // Point of Service Condition Code as per ISO8583
		default:
			MAPGETBYTE(traTxnType, txnType, lblKO);
			switch (txnType)
			{
				case trtSaleRedeem: //@ar-BRI
				case trtSale:
					POSC = 0;
					break;
				case trtPreAut:
			    case trtAut: //@agmr - BRI5
					POSC = 0x06;
					break;
				case trtRefund:
					POSC = 0x17;
					break;
			    case trtVoidTarikTunai:
				case trtVoid:
					POSC = 0x21;
					break;
			}
			POSC = 0x00; //@agmr - BRI - dibikin 00 semua
			MAPPUTBYTE(traConCode, POSC, lblEnd);
			break;
	}

	trcS("getPosCndCod: ");
	trcBN(&POSC, 1);
	trcS("\n");
	ret = bufApp(val, &POSC, 1);
	//ret = bufApp(val, (byte *) "\x00", 1);
	CHK;

	goto lblEnd;
lblKO:
	return -1;
lblEnd:
	return ret;
}

static int getTrack2(tBuffer * val)
{
	int ret, ctr, len;
	char track2[lenTrk2 + 1];
	word key;
	byte emvTrack2Bcd[lenEmvTrk2+ 1];   
    	card track2Len;					
    	char emvTrack2Hex[lenTrk2+ 1];   	
	byte EntMod;			


	VERIFY(val);
	switch (LocationType)
	{
		case 'R':
			key = revTrk2;
			break;
		case 'L':
			key = logTrk2;
			break;
		default:
			MAPGETBYTE(traEntMod, EntMod, lblKO); 							
	 		 if (EntMod == 'C' || EntMod == 'c') 
			 {									
		      		MEM_ZERO(emvTrack2Hex);										
		      		MEM_ZERO(emvTrack2Bcd);							      
		      		ret = mapGet(emvTrk2, emvTrack2Bcd, sizeof(emvTrack2Bcd));        		        
		      		CHK;
		      		bin2num(&track2Len, &emvTrack2Bcd[0], 1);							 
		      		bin2hex(emvTrack2Hex, emvTrack2Bcd+1,track2Len);					 
		      		ret = mapPut(traTrk2, emvTrack2Hex, (track2Len*2)-1);						 
		      		CHK;
			}
			key = traTrk2;
			break;
	}

	ret = mapGet(key, track2, sizeof(track2));
	CHK;

	len = strlen(track2);
	for (ctr = 0; ctr < len; ctr++)
	{
		if(track2[ctr] == '=')
		{
			track2[ctr] = 'D';
		}
	}

	trcFS("getTrack2: %s\n", track2);
	ret = bufAppStr(val, track2);
	CHK;

	return ret;
lblKO:
	return -1;
}

static int RevLogICC(word key, tBuffer * val)
{
	int ret;
	char buffer[256];
	word lenEmv;

	MAPGETWORD(TCEmvLen, lenEmv, lblKO);
	
	ret = mapGet(key, buffer, lenEmvICC);
	CHK;

	 ret = bufApp(val, buffer, lenEmv);
        CHK;

	return ret;
	lblKO:
		return -1;
}

#ifdef __EMV__
static int getICCData(tBuffer * val)
{
	int ret = 0;
	word ctl[] = {
		tagCID,
		tagIssTrnCrt,
		tagTVR,
		tagCVMRes,
		tagAmtNum,
		tagAmtOthNum,
		tagTrnCurCod,
		tagTrnDat,
		tagUnpNum,
		tagAIP,
		tagDFNam,
		tagIAD,
		tagATC,
		tagTrnTyp,
		tagAccCntCod,
		tagTrmCap,
		tagTrmTyp,
		tagIFDSerNum,
		tagTrmAvn,
		//tagTCC,
		0
	};
	word key;
	word *ptr;
	byte tag[2];
	byte dat[256];
	word mnu; //@agmr
	char Aid[lenAID + 1];

	VERIFY(val);

	switch(LocationType)
	{
		case 'R':
			return RevLogICC(revICC, val);
			break;
		case 'L':
			return RevLogICC(logEmvICC, val);
			break;
		case 'U':
			return RevLogICC(TCEmvICC, val);
			break;
		default:
			break;
	}
	
//++ @agmr BRI11
	MAPGETWORD(traMnuItm,mnu,lblKO);
	if(mnu == mnuVoid)
	    return RevLogICC(logEmvICC, val);
//-- @agmr BRI11

	MAPGET(emvAid, Aid, lblKO);
	
		
	ptr = ctl;
	while(*ptr)
	{
		key = mapKeyTag(*ptr);
		if((memcmp(&Aid[1], "\xA0\x00\x00\x03\x33", 5)==0) && (key == tagTCC))
			continue;
		ret = mapGet(key, dat, sizeof(dat));
		CHK;
		//mapGet(key,...,
		//append Tag Len Value
		tag[0] = *ptr >> 8;
		tag[1] = *ptr & 0xFF;
		if(*tag)
		{
			ret = bufApp(val, tag, 1);
			CHK;
		}
		ret = bufApp(val, tag + 1, 1);
		CHK;

		trcS("Tag: \n");
		trcBN(bufPtr(val), bufLen(val));
		trcS("\n");

		ret = bufApp(val, dat, 1);
		CHK;
		trcS("Tag Len: \n");
		trcBN(bufPtr(val), bufLen(val));
		trcS("\n");

		ret = bufApp(val, dat + 1, *dat);
		CHK;
		trcS("Tag Len Val: \n");
		trcBN(bufPtr(val), bufLen(val));
		trcS("\n");

		ptr++;
	}

	ret = mapPut(traEmvICC, bufPtr(val), bufLen(val));
	CHK;
	MAPPUTWORD(TCEmvLen, bufLen(val), lblKO);
		
	return ret;
lblKO:
	return -1;
}
#endif

static int getPIN(tBuffer * val)
{
	int ret;
	char PIN[1 + lenPinBlk];
	word key;
	byte txnType; //@agmr - BRI

	VERIFY(val);

    MAPGETBYTE(traTxnType, txnType, lblKO); //@agmr - BRI
    switch (LocationType) {
      case 'R':
          key = revPinBlk;
          break;
      case 'L':
          key = logPinBlk;
          break;
		  
      default:
//++@agmr - BRI        
        if(txnType == trtAktivasiKartu ||
           txnType == trtRenewalKartu  ||
           txnType == trtReissuePIN)
            key = traNewPinBlk;
        else
//--@agmr - BRI              
	        key = traPinBlk;
	  break;
    	}

	ret = mapGet(key, PIN, sizeof(PIN));
	CHK;
	trcS("getPIN: ");
	trcBN((byte *) PIN, 1 + lenPIN);
	trcS("\n");
	
//++ @agmr - BRI
    MAPGETBYTE(traTxnType,txnType,lblKO);
    if(txnType == trtAktivasiKartuInquiry)
        memset((byte*)PIN,0,sizeof(PIN));
//-- @agmr - BRI	

	ret = bufApp(val, (byte *) PIN, lenPinBlk);
	CHK;

	return ret;
lblKO:
	return -1;
}

static int getAddAmt(tBuffer * val)
{
	int ret;
	char TipAmt[lenAmt + 1];
	word key;
	byte AmtExp;

	VERIFY(val);

	switch (LocationType)
	{
		case 'R':
			key = revTipAmt;
			break;
		case 'L':
			key = logTipAmt;
			break;
		case 'U':
			key = TCTipAmt;
			break;
		case 'T':
		default:
			key = traTipAmt;
			break;
	}
	ret = mapGet(key, TipAmt, sizeof(TipAmt));
	CHK;

	//fmtPad(TipAmt, -lenAmt, '0');
	
       MAPGETBYTE(appExp, AmtExp, lblKO);
       if(AmtExp == 0){
	     fmtPad(TipAmt, -10, '0');                         
            memcpy(TipAmt+10, "00", 2);			    
       }
	else
            fmtPad(TipAmt, -lenAmt, '0');

	trcS("getAddAmt: ");
	trcBN((byte *) TipAmt, lenAmt);
	trcS("\n");

	ret = bufApp(val, (byte *) TipAmt, lenAmt);
	CHK;

	return ret;
lblKO:
	return -1;
}

static int getRoc(tBuffer * val)
{
	int ret;
	char InvNum[lenInvNum + 1];
	char buff[lenInvNum + 1];
	word key;
	byte txnType;

	VERIFY(val);

	switch (LocationType)
	{
		case 'R':
			key = cvtRevInvNum;
			break;
		case 'L':
			key = cvtRptInvNum;
			break;
		case 'U':
			key = cvtTCInvNum;
			break;	
		case 'T':
		default:
			key = cvtInvNum;
			break;
	}
	ret = mapGet(key, InvNum, sizeof(InvNum));
	CHK;

	trcFS("getRoc: %s\n", InvNum);

	MAPGETBYTE(traTxnType, txnType, lblKO);
	//@@REPAIR - start
#if 0
	if((txnType == trtPreCom)||(txnType == trtPreCancel))
	{
		memset(buff, 0x00, lenAutCod);
		MAPGET(traPINInternetBanking,buff,lblKO);
		ret = bufApp(val, (byte *) buff, lenInvNum);
	}
	else
#endif
	//@@REPAIR - end
		ret = bufApp(val, (byte *) InvNum, lenInvNum);
	CHK;

	//ret = bufAppStr(val, "000024");

	return ret;
lblKO:
	return -1;
}

static int getTotal(tBuffer * val)
{
	int ret;
	word count;
	char amt[lenTotAmt + 1];
	char total[3 + 1];
//	char amtPad[75+1];
	byte AmtExp;
    char valBuf[100];
    char *ptr; //@agmr
    byte settleOpt=0; //@agmr

	VERIFY(val);

	ret = logCalcTot();

//++ @agmr
    ptr = valBuf;
    {
//        byte settlementAcq;
        
        memset(valBuf,'0',90);
        valBuf[90] = 0x00;
        
        MAPGETBYTE(acqStlDialOpt,settleOpt,lblKO);
        if(settleOpt & 0x04 ) //debit bri acq
        {

	//		fmtPad(buf,30,'0');  //padding settel debit aquirer
			ptr+=30;


        }
    }
//-- @agmr

	MAPGETCARD(totAppSaleCnt, count, lblKO);
	sprintf(total, "%03d", count);

	trcFS("Total Sale: %s\n", total);
	memcpy(ptr,total,3);
	ptr+=3;
	
//	ret = bufAppStr(val, total);

	ret = mapGet(totAppSaleAmt, amt, sizeof(amt));
	CHK;
	
//	fmtPad(amt, -lenAmt, '0');
//++BRI10
	MAPGETBYTE(appExp, AmtExp, lblKO);
    if(AmtExp == 0){
        fmtPad(amt, -10, '0');
        memcpy(amt + 10, "00", 2);
    }
	else
//--BRI10	    
       fmtPad(amt, -lenAmt, '0');
       
	trcFS("getSaleTotal: %s\n", amt);
	
	memcpy(ptr,amt,12);
	ptr+=12;
	
//	ret = bufAppStr(val, amt);
	CHK;

	MAPGETCARD(totAppRefundCnt, count, lblKO);

	sprintf(total, "%03d", count);
	trcFS("Total Refund: %s\n", total);
	memcpy(ptr,total,3);
	ptr+=3;	
	
//	ret = bufAppStr(val, total);

	ret = mapGet(totAppRefundAmt, amt, sizeof(amt));
	CHK;
//++BRI10
    if(AmtExp == 0){
        fmtPad(amt, -10, '0');
        memcpy(amt + 10, "00", 2);
    }
	else
//--BRI10	    	
	    fmtPad(amt, -lenAmt, '0');

	trcFS("getRefundTotal: %s\n", amt);
	memcpy(ptr,amt,12);
	ptr+=12;
//	ret = bufAppStr(val, amt);
//	CHK;

    ret = bufApp(val, (byte *) valBuf, 90);
	CHK;

//	memset(amtPad, 0x30, sizeof(amtPad));
//	ret = bufApp(val, (byte *) amtPad, 75);
//	CHK;

	//ret = bufAppStr(val, "001000000000100001000000000100");

	return ret;
lblKO:
	return -1;
}

static int getIso8(tBuffer * val)
{
	int ret;

	VERIFY(val);
	switch (reqtyp)
	{
		case 'P':
			ret = bufApp(val, (byte *) "\x00\x00\x05\x00", 4);
			CHK;
			break;
		default:
			ret = -1;
			break;
	}

	return ret;
lblKO:
	return -1;
}

static int getBatchUpload(tBuffer * val) {
    int ret;
    card STAN;
    char dec[2 * lenSTAN + 1];
    byte mti[1 + lenMti];
    char HexMti[1 + lenMti*2];
    VERIFY(val);

    MAPGET(logMTI, mti, lblKO);
    bin2hex(HexMti, mti, lenMti);
    ret = bufApp(val, (byte *) HexMti, 2 * lenMti);
    CHK;
           
    ret = mapGetCard(logSTAN, STAN);
    CHK;	
	
    num2dec(dec, STAN, 2 * lenSTAN);
    ret = bufApp(val, dec, 2 * lenSTAN);
    CHK;


   return ret;
   lblKO:
     return -1;
}

static int getBatch(tBuffer * val)
{
	int ret;
	char BatchNum[lenInvNum + 1];
	word key;

	VERIFY(val);

	switch (LocationType)
	{
		case 'R':
			key = revBatNum;
		case 'L':
			key = logBatNum;
			break;
		case 'T':
		default:
			key = acqCurBat;
			break;
	}
	ret = mapGet(key, BatchNum, sizeof(BatchNum));
	CHK;

	trcFS("getbatch: %s\n", BatchNum);

	ret = bufApp(val, (byte *) BatchNum, lenInvNum);
	CHK;

	//ret = bufAppStr(val, "000123");

	return ret;
lblKO:
	return -1;
}

static int fillbit60(tBuffer * val)
{
	int ret = 0;
	byte sta, ReqRecCnt = 1, mreRec = 0;
	byte buffer[lenFleNme + 1];
	card offset = 0;

	VERIFY(val);
	ret = mapGetByte(traSta, sta);
	CHK;
	ret = bufApp(val, (byte *) "\x00\x66\x00\x03\x01\x01\x90\x01", 8);  // initialize buffer with con table req.
	CHK;
	switch (sta)
	{
		case dnlSendLoginReq:
		case dnlSendLogOffReq:
			bufReset(val);
			break;
		case dnlSendConTblReq:
			break;
		case dnlSendBinTblReq:
#ifdef OLD_CODE
			ret = bufOwr(val, 3, (byte *) "\x01\x02", 2); //change the apiVersion and table number required, bin table is table 2
			CHK;
			ret = bufOwr(val, 5, &ReqRecCnt, 1);  //overwrite the needed record count for request
			CHK;
			//\x00\x66\x00\x01\x02\x01\x90\x01
#else //@@AS0.0.32
			ret = bufOwr(val, 3, (byte *) "\x01\x02", 2); //change the apiVersion and table number required, bin table is table 2
			CHK;
			ret = mapGetByte(traTmsMreRec, mreRec);
			CHK;
			if(mreRec)
			{
				ret = mapGetByte(regBinCnt, ReqRecCnt);
				CHK;
				ReqRecCnt += 1;
				ret = bufOwr(val, 5, &ReqRecCnt, 1);  //overwrite the needed record count for request
				CHK;
			}
			//\x00\x66\x00\x01\x02\x01\x90\x01
#endif
			break;
		case dnlSendIssTblReq:
			ret = bufOwr(val, 3, (byte *) "\x02\x03", 2); //change the apiVersion and table number required, iss table is table 1
			CHK;
			ret = mapGetByte(traTmsMreRec, mreRec);
			CHK;
			if(mreRec)
			{
				ret = mapGetByte(regIssCnt, ReqRecCnt);
				CHK;
				ReqRecCnt += 1;
				ret = bufOwr(val, 5, &ReqRecCnt, 1);  //overwrite the needed record count for request
				CHK;
			}
			//\x00\x66\x00\x02\x03\x01\x90\x01
			break;
		case dnlSendAcqTblReq:
			ret = bufOwr(val, 3, (byte *) "\x02\x04", 2); //change the apiVersion and table number required, acq table is table 1
			CHK;
			ret = mapGetByte(traTmsMreRec, mreRec);
			CHK;
			if(mreRec)
			{
				ret = mapGetByte(regAcqCnt, ReqRecCnt);
				CHK;
				ReqRecCnt += 1;
				ret = bufOwr(val, 5, &ReqRecCnt, 1);  //overwrite the needed record count for request
				CHK;
			}
			//\x00\x66\x00\x02\x04\x01\x90\x01
			break;
		case dnlSendEmvReq:
			bufReset(val);        //change buffer for emv request
			ret = bufApp(val, (byte *) "\x00\x66\x00\x01\x00\x00\x01", 7);    // initialize buffer for emv request
			CHK;
			//x00x07x00x66x00x01x00x00x01
			break;
		case dnlSendEmvFileReq:
			bufReset(val);        //change buffer for emv request
			CHK;
			ret = bufApp(val, (byte *) "\x01", 1);    // GENERIC_FILE_OPEN
			CHK;
			ret = mapGet(traTmsEmvFleNme, buffer, lenFleNme);
			CHK;
			ret = bufApp(val, buffer, strlen((char *) buffer) + 1);   // initialize buffer for emv request
			CHK;
			ret = bufApp(val, (byte *) "\x72\x62\x00", 3);    //fopen mode "rb"
			CHK;
			break;
		case dnlSendEmvFileOpnReq:
			bufReset(val);        //change buffer for emv request
			CHK;
			ret = bufApp(val, (byte *) "\x02", 1);    // GENERIC_FILE_INFO
			CHK;
			ret = mapGet(traTmsEmvFleHdl, buffer, sizeof(buffer));
			CHK;
			ret = bufApp(val, buffer, lenFleHdl); // initialize buffer for emv file open
			CHK;
			break;
		case dnlSendEmvFileRedReq:
			bufReset(val);        //change buffer for emv request
			CHK;
			ret = bufApp(val, (byte *) "\x03", 1);    // GENERIC_FILE_READ
			CHK;
			ret = mapGet(traTmsEmvFleHdl, buffer, sizeof(buffer));
			CHK;
			ret = bufApp(val, buffer, lenFleHdl); // initialize buffer for emv file open
			CHK;
			ret = mapGetCard(traTmsEmvFleOff, offset);
			CHK;
			memset(buffer, 0x00, sizeof(buffer));
			ret = num2bin((byte *) buffer, offset, lenFleHdl);
			CHK;
			ret = bufApp(val, buffer, lenFleHdl); // initialize buffer for emv file read
			CHK;
			ret = bufApp(val, (byte *) "\x01\xBB", sizeof(word)); // size of chunk request is always 443
			CHK;
			break;
		case dnlSendEmvFileClsReq:
			bufReset(val);        //change buffer for emv request
			CHK;
			ret = bufApp(val, (byte *) "\x05", 1);    // GENERIC_FILE_CLOSE
			CHK;
			ret = mapGet(traTmsEmvFleHdl, buffer, sizeof(buffer));
			CHK;
			ret = bufApp(val, buffer, lenFleHdl); // initialize buffer for emv file open
			CHK;
			break;
		default:
			ret = -1;
			break;
	}
	trcS("fillbit60: ");
	trcBN(bufPtr(val), 25);
	trcS("\n");
	return ret;
lblKO:
	return -1;
}

static int getBit60TCUpload(tBuffer * val)
{
    int ret;
    card STAN;
    char dec[2 * lenSTAN + 1];
    byte mti[1 + lenMti];
    char HexMti[1 + lenMti*2];
    VERIFY(val);

    MAPGET(TCMti, mti, lblKO);
    bin2hex(HexMti, mti, lenMti);
    ret = bufApp(val, (byte *) HexMti, 2 * lenMti);
    CHK;
           
    ret = mapGetCard(TCSTAN, STAN);
    CHK;	
	
    num2dec(dec, STAN, 2 * lenSTAN);
    ret = bufApp(val, dec, 2 * lenSTAN);
    CHK;

   return ret;
   lblKO:
     return -1;

	
}

static int getIso60(tBuffer * val)
{
	int ret = 0;
	byte idx;

	VERIFY(val);
	switch (reqtyp)
	{
        case 'A':
#ifdef LINE_ENCRYPTION
          getField60(val); //@agmr - BRI
#endif          
          break;	    
		case 'P':
			fillbit60(val);
			break;
		case 'S':
			MAPGETBYTE(traTxnType, idx, lblKO);
            switch (idx) {
		      case trtBatchUpload:
			  getBatchUpload(val);
			    break;			  
				
		      default:
			      	getBatch(val);
				break;
         	}
            break;
		case 'U':	
			getBit60TCUpload(val);
			break;
		default:
			ret = -1;
			break;
	}

	return ret;
	  lblKO:
	    return -1;
}

static int getRrn(tBuffer * val) {
	int ret;
	char Rrn[1+lenRrn];
	word key;

	VERIFY(val);
	memset(Rrn, 0x00, sizeof(Rrn));

	switch(LocationType)
	{
		case 'U':
			key = TCRrn;
			break;
		case 'L':
			key = logRrn;
			break;
		default:
			key = traRrn;
			break;
	}
	
	MAPGET(key, Rrn, lblKO);

	ret = bufApp(val, Rrn, lenRrn);
	CHK;
	
	return ret;
	
	lblKO: 
		return -1;
}

static int getAutCod(tBuffer * val) {
	int ret;
	char AutCod[1+lenAutCod];
	byte data[1024];
	word key;
	byte res;

	VERIFY(val);
	memset(AutCod, 0x00, sizeof(AutCod));

	switch(LocationType)
	{
		case 'U':
			key = TCAutCod;
			break;
		case 'L':
			key = logAutCod;
			break;
		default:
			key = traAutCod;
			break;
	}
	MAPGETBYTE(appBriziDiscFlag,res,lblKO);
	if(res==1)
	{
		MAPGET(appBriziDiscValue,AutCod,lblKO);
		MAPPUTBYTE(appBriziDiscFlag,0,lblKO);
	}
	else
	{
		MAPGET(key, AutCod, lblKO);
	}
	
	ret = bufApp(val, AutCod, lenAutCod);
	CHK;
	
	return ret;
	
	lblKO: 
		return -1;
}

static int getRspCode(tBuffer * val) {
	int ret;
	char rspCod[1+lenAutCod];
	word key;

	VERIFY(val);
	memset(rspCod, 0x00, sizeof(rspCod));

	switch(LocationType)
	{
		case 'U':
			key = TCRspCod;
			break;
		case 'L':
			key = logRspCod;
			break;
		default:
			key = traRspCod;
			break;
	}
	
	MAPGET(key, rspCod, lblKO);

	ret = bufApp(val, rspCod, lenRspCod);
	CHK;
	
	return ret;
	
	lblKO: 
		return -1;
}

static int getPan(tBuffer * val) {
	int ret;
	char Pan[1+lenPan];
	word key;

	VERIFY(val);
	memset(Pan, 0x00, sizeof(Pan));

	switch(LocationType)
	{
		case 'U':
			key = TCPan;
			break;
		case 'L':
			key = logPan;
			break;
			
		default:
			key = traPan;
			break;
	}
	
	MAPGET(key, Pan, lblKO);


//	ret = bufApp(val, Pan, lenPan); //@agmr - tutup, panjang pan belum tentu 19
	ret = bufApp(val, Pan, strlen(Pan));

	CHK;
	
	return ret;
	
	lblKO: 
		return -1;
}

static int getFld(tBuffer * val, byte bit)
{
	word key;
//	byte TxnType;
	int ret;
//	byte trtType = 0; //@agmr - BRI

//	MAPGETBYTE(traTxnType,TxnType,lblKO);

//{
//    char buf1[50];
//    
//    sprintf(buf1,"bit=%d txn=%i",bit,trtType);
//    prtS(buf1);
//}		 
		
	VERIFY(val);
	trcFN("getFld bit: %d\n", bit);
	bufReset(val);
	
//	MAPGETBYTE(traTxnType, TxnType, lblKO);

	switch (bit)
	{
		case isoPan:
			return getPan(val);
		case isoPrcCod:
			return getPrcCod(val);
		case isoAmt:
			return getAmt(val);
		case isoDatTim:
			return getDatTim(val);
		case isoSTAN:
			return getSTAN(val);
		case isoTim:
			return getTim(val);
		case isoDat:
			return getDat(val);
		case isoDatExp:
//++ @agmr
//            key = traExpDat;
        	switch(LocationType)
        	{
        		case 'L':
        			key = logExpDat;
        			break;
// ++@agmr - BRI11          			
        	    case 'R':
        	        key = revExpDat;
        	        break;
// --@agmr - BRI11             			
        		default:
        			key = traExpDat;
        			break;
        	}
//-- @agmr			
			return getVal(val, key);
		case isoPosEntMod:
			return getPosEntMod(val);
		case isoCrdSeq:
			return getCardSeq(val);
		case isoNII:
			return getNII(val);
		case isoPosCndCod:
			return getPosCndCod(val);
		case isoTrk2:
			{
				byte 	txnType;
				char track2[lenTrk2 + 1];
				int    len, ctr;
				
				MAPGETBYTE(traTxnType, txnType, lblKO);
				if(txnType!=trtTCash)
					return getTrack2(val);
				else
					{
						strcpy(track2, "9999999999999999=10010000000000000000");
						len = strlen(track2);
						for (ctr = 0; ctr < len; ctr++)
						{
							if(track2[ctr] == '=')
							{
								track2[ctr] = 'D';
							}
						}
						return bufAppStr(val, track2);
					}
			}
	      case	isoRrn:
		  	return getRrn(val);
	      case isoAutCod:
		  	return getAutCod(val);
	      case	isoRspCod:
		  	return getRspCode(val);
		case isoTid:
			key = ((reqtyp == 'P') ? appTid : acqTID);
			return getVal(val, key);
		case isoMid:
			key = ((reqtyp == 'P') ? regTmsRefNum : acqMID);  //change later
			return getVal(val, key);
      //    ++ @agmr - BRI - field 48
        case isoPinMacKey:
            return getField48(val);
      //    -- @agmr - BRI  
      case isoCur:
            return getCurrencyCode(val, emvTrnCurCod);      			
		case isoPinDat:          //052  PIN
			return getPIN(val);
		case isoAddAmt:          //054  Additional Amount
			return getAddAmt(val);
#ifdef __EMV__

		case isoEmvPds:          //055  ICC System related data   
			return getICCData(val);
#endif

// ++ @agmr - BRI          
      case iso057:
        return getIso57(val);
        break;
// -- @agmr - BRI

		case isoMaxBuf:          //008  maximum buffer to be received by application
			return getIso8(val);
		case isoBatNum:          //060  Reserved Private
			return getIso60(val);
// ++ @agmr - BRI          
      case isoAmtOrg:
            return getField61(val);    
// -- @agmr - BRI       			
		case isoRoc:             //062  ROC
			return getRoc(val);
		case isoAddDat:          //063  Settlement Totals
//++ @agmr - BRI    
            {
                word mnuItm;
		  byte  txnType;
                MAPGETWORD(traMnuItm, mnuItm, lblKO);   
		  MAPGETBYTE(traTxnType,txnType,lblKO);
                if((isBRITransaction() || mnuItm == mnuReversal )
#ifdef PREPAID                    
                    || (isPrepaidTransaction(0))
#endif                    
			|| (mnuItm == mnuSaleRedeem)
			|| (txnType == trtLogOn)
			|| (txnType == trtDebitLogon)
			|| (txnType == trtDebitLogon)
			|| (txnType ==  trtEchoTest) 
                  )
                    return getField63(val);                  
            }
            
//-- @agmr - BRI       		    
			return getTotal(val);
		default:
			break;
	}
	trcS("Error Get FLD\n");
	lblKO:
		return -1;
}

static int asc2bcd(byte * bcd, const char *asc)
{
	char tmp[270 + 1];
	word len;

	VERIFY(bcd);
	VERIFY(asc);

	len = strlen(asc);
	VERIFY(len <= 270);
	strcpy(tmp, asc);
	if(len % 2 != 0)
		tmp[len] = 'F';

	hex2bin(bcd, tmp, 0);
	return len;
}

#ifdef LINE_ENCRYPTION
static int makeSingleFld(byte bit,const tBuffer *val,byte *outData){
	int fmt;
	char tmp[3 + 1];
	byte buf[ISO_BUFFER_LEN]; //@agmr - ganti dari 512
	byte bcd[4];
	word len,dataLen;
	byte cnt;

	trcFN("makeSingleFld %d: ", bit);
	trcBN(bufPtr(val), bufLen(val));
	trcS("\n");
	memset(bcd, 0, sizeof(bcd));
	memset(buf, 0, sizeof(buf));
	dataLen = 0;

	fmt = isoFmt(bit);

	if(fmt < 0)
	{               //LLVAR or LLLVAR
		len = bufLen(val);
		switch (-fmt)
		{
			case 2:
			case 4:
				cnt = 2;
				break;
			case 3:
			case 5:
				cnt = 3;
				break;
			default:
				cnt = 0;
				break;
		}
		VERIFY(cnt);
		switch (-fmt)
		{
			case 2:
			case 3:              //BCD
				asc2bcd(buf, (char *) bufPtr(val));
				num2dec(tmp, len, (cnt - 1) * 2);
				hex2bin(bcd, tmp, 0);
				len = (len + 1) / 2;
				break;
			case 4:
			case 5:              //ASC
				memcpy(buf, bufPtr(val), len);
				num2dec(tmp, len, (cnt - 1) * 2);
				hex2bin(bcd, tmp, 0);
				break;
			default:
				cnt = 0;
				break;
		}

		//memcpy(outData,bcd,cnt-1);
		//dataLen += (cnt-1);
	}
	else
	{
		if(fmt % 2 != 0)
			fmt++;
		len = fmt / 2;
		memcpy(buf, bufPtr(val), len);
	} 
	trcFN("makeSingleFld isoFmt: %d\n", len);

	memcpy(outData+dataLen,buf,len);
	dataLen += len;

	return (dataLen);
	//ret=1;
	//ret = bufApp(req, buf, len);
	//CHK;

	//return bufLen(req);
//lblKO:
//	return -1;
}

#endif

static int appFld(tBuffer * req, byte bit, const tBuffer * val)
{
	int ret;
	int fmt;
	char tmp[3 + 1];
//	byte buf[270];
	byte buf[1024];	// @@OA - buf kurang gede
	byte bcd[4];
	word len;
	byte cnt;

	trcFN("appFld %d: ", bit);
	trcBN(bufPtr(val), bufLen(val));
	trcS("\n");
	memset(bcd, 0, sizeof(bcd));
	memset(buf, 0, sizeof(buf));

	fmt = isoFmt(bit);

	if(fmt < 0)
	{               //LLVAR or LLLVAR
		len = bufLen(val);
		switch (-fmt)
		{
			case 2:
			case 4:
				cnt = 2;
				break;
			case 3:
			case 5:
				cnt = 3;
				break;
			default:
				cnt = 0;
				break;
		}
		VERIFY(cnt);
		switch (-fmt)
		{
			case 2:
			case 3:              //BCD
				asc2bcd(buf, (char *) bufPtr(val));
				num2dec(tmp, len, (cnt - 1) * 2);
				hex2bin(bcd, tmp, 0);
				len = (len + 1) / 2;
				break;
			case 4:
			case 5:              //ASC
				memcpy(buf, bufPtr(val), len);
				num2dec(tmp, len, (cnt - 1) * 2);
				hex2bin(bcd, tmp, 0);
				break;
			default:
				cnt = 0;
				break;
		}

		ret = bufApp(req, bcd, cnt - 1);
	}
	else
	{
		if(fmt % 2 != 0)
			fmt++;
		len = fmt / 2;
		memcpy(buf, bufPtr(val), len);
	}
	trcFN("appFld isoFmt: %d\n", len);

	ret = bufApp(req, buf, len);
	CHK;

	return bufLen(req);
lblKO:
	return -1;
}

static int modifyBitmap(byte * BitMap) {
	int ret;
	char EntMod;
	char tipAmt[lenAmt +1];
	word mnuItm;
	char PanSeq[1+lenPANSeq];
	byte buf[20];
	byte trnType;
	byte res;
	word key;
	byte txnType1,jenis;
	char Aid[lenAID + 1];
   

	//@@AS0.0.26 : start
	MAPGETBYTE(traTxnType, trnType, lblKO);
	if (trnType==trtDebitLogon)
		return 1;
	//@@AS0.0.26 : end
	
	MAPGETWORD(traMnuItm, mnuItm, lblKO);
	MAPGETBYTE(traKategoriKartu,jenis,lblKO);
	
	if(mnuItm == mnuReversal)
	{
	    card len;
	    
	    mapGetCard(revField48len,len);
	    if(len<=0)
	        bitOff(BitMap, isoPinMacKey);

	    mapGetCard(revField63len,len);
	    if(len<=0)
	        bitOff(BitMap, isoAddDat);    
	        
	    mapGetCard(revField57len,len);
	    if(len<=0)
	        bitOff(BitMap, iso057);   
	    
	    memset(buf,0,sizeof(buf));  
	    mapGet(revPinBlk,buf,lenPinBlk);
	    if(memcmp(buf,"\x00\x00\x00\x00\x00\x00\x00\x00",8)==0)
	        bitOn(BitMap, isoPinDat);
//++ @agmr - BRI11	    
	    MAPGETBYTE(revTxnType, txnType1, lblKO);
	    if(txnType1 == trtSale || txnType1 == trtInstallment || txnType1 == trtVoid || txnType1 == trtSaleRedeem)//@ar-BRI
	    {
            if(jenis == CREDIT_CARD || jenis == NON_BRI_DEBIT_CARD
               || jenis == NON_BRI_CREDIT_CARD)
            {
    	        bitOn(BitMap,isoPan);
    	        bitOn(BitMap,isoDatExp);
    	        bitOff(BitMap,isoTrk2);                
            }      	        
	    }	        
	}
	
	if(mnuItm == mnuVoid)
	{
        if(jenis == CREDIT_CARD || jenis == NON_BRI_DEBIT_CARD
           || jenis == NON_BRI_CREDIT_CARD)
        {
	        bitOn(BitMap,isoPan);
	        bitOn(BitMap,isoDatExp);
	        bitOff(BitMap,isoTrk2);                
        }      	        
	}	
//-- @agmr - BRI11	 

	//TBD: modify bitmap according the context:
	// if pin online, bitOn(52)
	// if manual input, activate PAN and ExpDate;
	//etc
	VERIFY(BitMap);
	trcS("modifyBitmap\n");
	
	switch (LocationType)
	{
		case 'L':
			key = logEntMod;
			break;
		case 'T':
		default:
			key = traEntMod;
			break;
	}
	ret = mapGetByte(key, EntMod);
	CHK;
	
//	ret = mapGetByte(traEntMod, EntMod);
	switch (EntMod) {
	case 'k':
		bitOff(BitMap, isoTrk2);
		bitOff(BitMap, isoPinDat);
		bitOff(BitMap, isoEmvPds);
		bitOn(BitMap, isoPan);
		bitOn(BitMap, isoDatExp);
		break;
	case 'c':
	case 'C':
	    if (emvFullProcessCheck() != 0)
//		if (emvFullProcessCheck() == 1)
			bitOn(BitMap, isoEmvPds);
		
		MAPGETBYTE(traPinReq, res,lblKO);
		if(res==1)
			bitOn(BitMap, isoPinDat);
		else
			bitOff(BitMap, isoPinDat);
		MAPGET(emvPANSeq, PanSeq, lblKO);
		if(PanSeq[0] != 0) 
			bitOn(BitMap, isoCrdSeq);
		/*
		if((trnType==trtPreAuth)||(trnType==trtPreCancel))
			bitOff(BitMap, isoPinDat);
		*/
		/*
		MAPGET(emvAid, Aid, lblKO);
		if(memcmp(&Aid[1], "\xA0\x00\x00\x03\x33", 5)==0)
		{
			bitOn(BitMap, isoRsvPvt);
		}
		*/
		break;
	case 'm':
	case 'M':	
		bitOn(BitMap, isoTrk2);
		MAPGETBYTE(traPinReq, res,lblKO);
		if(res==1) //@changedByAli -- SIMAS PIN BYPASSED
		{
			if(valIsPinRequired()>0 && mnuItm != mnuReversal)
					bitOn(BitMap, isoPinDat);
		}
		else
			bitOff(BitMap, isoPinDat);

		break;
	default:
		break;
	}

	MAPGET(traTipAmt, tipAmt, lblKO);
	if(atol(tipAmt) > 0)
		bitOn(BitMap, isoAddAmt);	       

	return 1;
	lblKO:
		return -1;
}

int reqBuild(tBuffer * req)
{
	int ret;
	byte bit, idx;
	byte mti[1 + lenMti];
	byte Bitmap[1 + lenBitmap];
	tBuffer buf;
	byte txnId = 0;
	//byte EntMod;
//	byte LocType;
	byte trtType = 0; //@agmr - BRI
	byte dat[ISO_BUFFER_LEN];
	word key;
	byte transTypeOris; //@@REPAIR2 
	char Aid[132];
#ifdef LINE_ENCRYPTION
	byte encBuff[ISO_BUFFER_LEN],tag[2],fldBuff[ISO_BUFFER_LEN]; //@@AS0.0.10
	byte aTmp [50], tlv_buf [ISO_BUFFER_LEN]; //@@AS0.0.10
	word encBuffLen,tagLen,tlv_len; //@@AS0.0.10
	char serialNum[20 + 1];     //@@AS0.0.10 
	char sap[16 + 1];  //@@AS0.0.10
	char dvc; //@@AS0.0.10
	byte need2Encrypt=0; //@@AS0.0.10
	word fldLen;//@@AS-SIMAS
	byte encryptedBitmap[lenBitmap];//@@AS-SIMAS
	byte null_buff[ISO_BUFFER_LEN]; //@@AS-SIMAS
	byte tak[16];//@@AS-SIMAS
	byte mac[8];//@@AS-SIMAS
	byte tmkRefNo[8];//@@AS-SIMAS
	
#endif
	VERIFY(req);
	trcS("reqBuild Beg\n");

    MAPGETBYTE(traTxnType,trtType,lblKO); //@agmr - BRI
	ret = mapGetByte(regLocType, LocationType);
	CHK;
	//MAPGETBYTE(traOriginalTxnType, transTypeOri, lblKO);
	ret = mapGetWord(traMnuItm, key);
	CHECK(ret >= 0, lblKO);
	VERIFY(key);
	switch (key)
	{
		case mnuSettlement:
			reqtyp = 'S';
			break;
		case mnuTMSLP:
			reqtyp = 'P';
			break;
		case mnuSale:
			txnId = rqtSale;
			reqtyp = 'A';
			break;
		case mnuSaleRedeem:
			txnId = rqtSaleRedeem;
			reqtyp = 'A';
			break;
		case mnuTCUpload:
			txnId = rqtTCUpload;//For TC Upload
			reqtyp = 'U';	
			break;
		case mnuPreaut:
			txnId = rqtPreAuth;
			reqtyp = 'A';
			break;
//++ @agmr - BRI5			
		case mnuAut:
			txnId = rqtAuth;
			reqtyp = 'A';
			break;

		case mnuPreAuth:
			txnId = rqtPreAuth;
			reqtyp = 'A';
			break;

		case mnuPreCancel:
			txnId = rqtPreCancel;
			reqtyp = 'A';
			break;

		case mnuPreCom:
			txnId = rqtPreCom;
			reqtyp = 'A';
			break;
//-- @agmr - BRI5						
		case mnuRefund:
			txnId = rqtRefund;
			reqtyp = 'A';
			break;
		case mnuVoid:
			txnId = rqtVoid;
			reqtyp = 'A';
			break;
		case mnuOffline:
			txnId = rqtOflSale;
			reqtyp = 'A';
			break;
		case mnuLogon:
			txnId = rqtLogOn;
			reqtyp = 'A';
			break;
		case mnuReversal:
		    if(txnAsli == 0)
		        txnAsli = trtType; //@agmr - BRI2 - nanti dikembalikan sesudah proses reversal
		        //MAPGETBYTE(traEntMod, EntMod, lblKO);
			txnId = rqtReversal;
			reqtyp = 'R';
			break;
			
// ++ @agmr - BRI
    case mnuInfoSaldo:
        txnId = rqtInfoSaldo;
        reqtyp = 'A';
        break;
    case mnuInfoSaldoBankLain:
        txnId = rqtInfoSaldoBankLain;
        reqtyp = 'A';
        break;
    case mnuMiniStatement:
        txnId = rqtMiniStatement;
        reqtyp = 'A';
        break;
    case mnuMutRek:
        txnId = rqtMutRek;
        reqtyp = 'A';
        break;
	case mnuPrevilege:
    	txnId = rqtPrevilege;
    	reqtyp = 'A';
    	break;
		
    case mnuTransferSesamaBRI:
        if(trtType == trtTransferSesamaBRIInquiry)
            txnId = rqtTransferSesamaBRIInquiry;
        else
            txnId = rqtTransferSesamaBRI;    
        reqtyp = 'A';
        break;
    case mnuTransferAntarBank:
        if(trtType == trtTransferAntarBankInquiry)
            txnId = rqtTransferAntarBankInquiry;
        else
            txnId = rqtTransferAntarBank;       
        reqtyp = 'A';
        break;
	/*Changed By Ali*/
	case mnuPembayaranTvBerlangganan:
	case mnuPembayaranTvInd:
	case mnuPembayaranTvOkTv:
	case mnuPembayaranTvTopTv:
		if(trtType == trtPembayaranTVBerlanggananInquiry)
            txnId = rqtPembayaranTVBerlanggananInquiry;
        else
            txnId = rqtPembayaranTVBerlangganan;        
        reqtyp = 'A';
        break;

	
	case mnuPembayaranTiketPesawat:
	case mnuPembayaranTiketGaruda:
	case mnuPembayaranTiketLionAir:
	case mnuPembayaranTiketSriwijaya:
	case mnuPembayaranTiketMandala:
		if(trtType == trtPembayaranTiketPesawatInquiry)
            txnId = rqtPembayaranTiketPesawatInquiry;
        else
            txnId = rqtPembayaranTiketPesawat;        
        reqtyp = 'A';
        break;

		
	case mnuPembayaranPdam:
	case mnuPembayaranPdamSby:
		if(trtType == trtPembayaranPdamInquiry)
            txnId = rqtPembayaranPdamInquiry;
        else
            txnId = rqtPembayaranPdam;        
        reqtyp = 'A';
        break;
	case mnuPembayaranTelkom:
		if(trtType == trtPembayaranTelkomInquiry)
            txnId = rqtPembayaranTelkomInquiry;
        else
            txnId = rqtPembayaranTelkom;        
        reqtyp = 'A';
		break;
	case mnuPembayaranDPLKR:
		
            txnId = rqtPembayaranDPLKR;        
        reqtyp = 'A';
		break;
	case mnuPembayaranBriva:
		if(trtType == trtPembayaranBrivaInquiry)
            txnId = rqtPembayaranBrivaInquiry;
        else
            txnId = rqtPembayaranBriva;        
        reqtyp = 'A';
		break;
	case mnuRegistrasiSmsBanking:
		txnId = rqtRegistrasiSmsBanking;
        reqtyp = 'A';
		break;
	case mnuRegistrasiTrxTransfer:
		if(trtType == trtTrxPhoneBankingTransferInquiry)
            txnId = rqtTrxPhoneBankingTransferInquiry;
        else
            txnId = rqtTrxPhoneBankingTransfer;        
        reqtyp = 'A';
		break;
	case mnuRegistrasiTrxPulsa:
        txnId = rqtTrxPhoneBankingPulsa;  
        reqtyp = 'A';
		break;
	case mnuPembayaranKKBRI:
		if(trtType == trtPembayaranKKBRIInquiry)
            txnId = rqtPembayaranKKBRIInquiry;
        else
            txnId = rqtPembayaranKKBRI;        		
        reqtyp = 'A';
        break; 
	case mnuPembayaranKKANZ:
		txnId = rqtPembayaranKKANZ;
        reqtyp = 'A';
        break; 
	case mnuPembayaranKKCITIKK:
		txnId = rqtPembayaranKKCITI;
        reqtyp = 'A';
        break; 
	case mnuPembayaranKKCITIKTA:
		txnId = rqtPembayaranKTACITI;
        reqtyp = 'A';
        break; 
	case mnuPembayaranKKHSBCKK:
		txnId = rqtPembayaranKKHSBC;
        reqtyp = 'A';
        break; 
	case mnuPembayaranKKHSBCKTA:
		txnId = rqtPembayaranKTAHSBC;
        reqtyp = 'A';
        break; 
	case mnuPembayaranKKRBSKK:
		txnId = rqtPembayaranKKRBS;
        reqtyp = 'A';
        break; 
	case mnuPembayaranKKRBSKTA:
		txnId = rqtPembayaranKTARBS;
        reqtyp = 'A';
        break; 
	case mnuPembayaranKKStanCharKK:
		txnId = rqtPembayaranKKStanChar;
        reqtyp = 'A';
        break; 
	case mnuPembayaranKKStanCharKTA:
		txnId = rqtPembayaranKTAStanChar;
        reqtyp = 'A';
        break; 
	case mnuPembayaranSPP:
		if(trtType == trtPembayaranSPPInquiry)
            txnId = rqtPembayaranSPPInquiry;
        else
            txnId = rqtPembayaranSPP;        
        reqtyp = 'A';
		break;
	/*End of Changed By Ali*/        
    case mnuPembayaranPLNPasca:
        if(trtType == trtPembayaranPLNPascaInquiry)
            txnId = rqtPembayaranPLNPascaInquiry;
        else
            txnId = trtPembayaranPLNPasca;        
        reqtyp = 'A';
        break;
	case mnuPembayaranPLNPra:
        if(trtType == trtPembayaranPLNPraInquiry)
            txnId = rqtPembayaranPLNPraInquiry;
        else
            txnId = rqtPembayaranPLNPra;        
        reqtyp = 'A';
        break;
	case mnuPembayaranPLNToken:
		if(trtType == trtPembayaranPLNToken)
            txnId = rqtPembayaranPLNToken;
        reqtyp = 'A';
        break;
    case mnuPembayaranCicilanFIF:
    case mnuPembayaranCicilanBAF:
    case mnuPembayaranCicilanOTO:
    case mnuPembayaranCicilanFinansia:
    case mnuPembayaranCicilanVerena:
    case mnuPembayaranCicilanWOM:
        if(trtType == trtPembayaranCicilanInquiry)
            txnId = rqtPembayaranCicilanInquiry;
        else
            txnId = rqtPembayaranCicilan;        
        reqtyp = 'A';
        break;
    case mnuPembayaranSimpati50:
    case mnuPembayaranSimpati100:
//    case mnuPembayaranSimpati150:
    case mnuPembayaranSimpati200:
    case mnuPembayaranSimpati300:
    case mnuPembayaranSimpati500:
    case mnuPembayaranSimpati1000:
    case mnuPembayaranMentari25:
    case mnuPembayaranMentari50:
    case mnuPembayaranMentari100:
    case mnuPembayaranMentari250:
    case mnuPembayaranMentari500:
    case mnuPembayaranMentari1000:
    case mnuPembayaranIm325:    
    case mnuPembayaranIm350:    
    case mnuPembayaranIm375:    
    case mnuPembayaranIm3100:   
    case mnuPembayaranIm3150:   
    case mnuPembayaranIm3200:   
    case mnuPembayaranIm3500:   
    case mnuPembayaranIm31000:  
/* @@OA
    case mnuPembayaranXlVoucher25: 
    case mnuPembayaranXlVoucher50: 
    case mnuPembayaranXlVoucher100:
    case mnuPembayaranXlVoucher200:
    case mnuPembayaranXlBebas50:
    case mnuPembayaranXlBebas100:
*/	
    /*Changed By Ali*/
	case mnuPembayaranEsia25: 
    case mnuPembayaranEsia50: 
    case mnuPembayaranEsia75: 
    case mnuPembayaranEsia100: 
    case mnuPembayaranEsia150: 
    case mnuPembayaranEsia200:
		
	case mnuPembayaranSmart20: 
	case mnuPembayaranSmart25:
	case mnuPembayaranSmart50: 
	case mnuPembayaranSmart100:
	case mnuPembayaranSmart150: 
	case mnuPembayaranSmart200:
	case mnuPembayaranSmart300:
	case mnuPembayaranSmart500:


	case mnuPembayaranFren20: 
    case mnuPembayaranFren25: 
    case mnuPembayaranFren50: 
    case mnuPembayaranFren100: 
	case mnuPembayaranFren150: 
    case mnuPembayaranFren200: 
    case mnuPembayaranFren300: 
    case mnuPembayaranFren500:

	case mnuPembayaranThree20:
	case mnuPembayaranThree50:
	case mnuPembayaranThree75:
	case mnuPembayaranThree100:
	case mnuPembayaranThree150:
    case mnuPembayaranThree300:
    case mnuPembayaranThree500:

	case mnuPembayaranAxis25:
	case mnuPembayaranAxis50:
	case mnuPembayaranAxis75:
	case mnuPembayaranAxis100:
	case mnuPembayaranAxis150:
    case mnuPembayaranAxis200:
    case mnuPembayaranAxis300:
	/* End Of Changed By Ali*/
    case mnuPembayaranXl25: 
    case mnuPembayaranXl50: 
    case mnuPembayaranXl75: 
    case mnuPembayaranXl100: 
    case mnuPembayaranXl150: 
    case mnuPembayaranXl200: 
    case mnuPembayaranXl300: 
    case mnuPembayaranXl500: 
        txnId = rqtPembayaranPulsa;
        reqtyp = 'A';
        break;      
    case mnuPembayaranZakat:
    case mnuPembayaranZakatDhuafa:
    case mnuPembayaranZakatYBM:
    case mnuPembayaranInfaqDhuafa:
    case mnuPembayaranInfaqYBM:
    case mnuPembayaranDPLKDhuafa:
    	txnId = rqtPembayaranZakat;
    	reqtyp = 'A';
    	break;
    case mnuSetorPasti:
    	txnId = rqtSetorPasti;
    	reqtyp = 'A';
    	break;
	
    case mnuSetorSimpanan:
        if(trtType == trtSetorSimpananInquiry)
            txnId = rqtSetorSimpananInquiry;
        else
            txnId = rqtSetorSimpanan;            
        reqtyp = 'A';
        break;                 
    case mnuSetorPinjaman:
        if(trtType == trtSetorPinjamanInquiry)
            txnId = rqtSetorPinjamanInquiry;
        else
            txnId = rqtSetorPinjaman;            
        reqtyp = 'A';
        break;         
    case mnuTarikTunai:
        txnId = rqtTarikTunai;
        reqtyp = 'A';
        break;         
    case mnuVoidTarikTunai:
        txnId = rqtVoidTarikTunai;
        reqtyp = 'A';
        break;

#ifdef T_BANK
    case mnuCashIn:
        txnId = rqtTCash;
        reqtyp = 'A';
        break; 
    case mnuCashOut:
        txnId = rqtTCashOut;
        reqtyp = 'A';
        break; 
#endif
    case mnuRegistrasiInternetBanking:
        txnId = rqtRegistrasiInternetBanking;
        reqtyp = 'A';
        break;         
    case mnuRegistrasiPhoneBanking:
        txnId = rqtRegistrasiPhoneBanking;
        reqtyp = 'A';
        break;         
    case mnuAktivasiKartu:
        if(trtType == trtAktivasiKartuOtorisasi)
            txnId = rqtOtorisasi;
        else if(trtType == trtAktivasiKartuInquiry)
            txnId = rqtAktivasiKartuInquiry;            
        else
            txnId = rqtAktivasiKartu;   
        reqtyp = 'A';
        break;    
    case mnuRenewalKartu:
        if(trtType == trtRenewalKartuOtorisasi)
        {
            txnId = rqtOtorisasi1;
        }
        else if(trtType == trtRenewalKartuInquiry)
        {         
            txnId = rqtRenewalKartuInquiry;            
        }
        else
        {                 
            txnId = rqtRenewalKartu;   
        }
        reqtyp = 'A';
        break;
    case mnuReissuePIN:
        if(trtType == trtReissuePINOtorisasi)
            txnId = rqtOtorisasi2;
        else
            txnId = rqtReissuePIN;    
        reqtyp = 'A';
        break;  
    case mnuPIN:
    case mnuGantiPIN:
        txnId = rqtGantiPIN;
        reqtyp = 'A';
        break;             
    case mnuGantiPasswordSPV:
        txnId = rqtGantiPasswordSPV;
        reqtyp = 'A';
        break;
    case mnuInstallment:        
        txnId = rqtInstallment;
        reqtyp = 'A';
        break;     
    
    case mnuPembayaranHalo:
    case mnuPembayaranMatrix:
        if(trtType == trtPembayaranPascaBayarInquiry)
            txnId = rqtPembayaranPascaBayarInquiry;
        else
            txnId = rqtPembayaranPascaBayar;
    	reqtyp = 'A';
    	break;
    
    case mnuTLEKeyDownload:
    	txnId = rqtTLEKeyDownload;
    	reqtyp = 'A';
    	break;
#ifdef PREPAID
    case mnuInfoDeposit:
        txnId = rqtInfoDeposit;
        reqtyp = 'A';
        break;
    case mnuPrepaidPayment:
    	txnId = rqtPrepaidPayment;
    	reqtyp = 'A';
    	break;           
    case mnuPrepaidPaymentDisc:
	txnId = rqtPrepaidPaymentDisc;
    	reqtyp = 'A';
	break;
    case mnuTopUpDeposit:
    	txnId = rqtDeposit;
    	reqtyp = 'A';
    	break;            
    case mnuTopUpOnline:
    	txnId = rqtTopUp;
    	reqtyp = 'A';
    	break;  
    case mnuPrepaidRedeem:
    	if(trtType == trtPrepaidRedeemInquiry)
    		txnId = rqtPrepaidRedeemInquiry;
    	else
    		txnId = rqtPrepaidRedeem;
    	reqtyp = 'A';
    	break;    	        

    //++@agmr - brizzi2
    case mnuPrepaidReaktivasi:
    	if(trtType == trtPrepaidReaktivasiInquiry)
    		txnId = rqtPrepaidReaktivasiInquiry;
    	else
    		txnId = rqtPrepaidReaktivasi;
    	reqtyp = 'A';
        break; 
    case mnuPrepaidVoid:
    	if(trtType == trtPrepaidVoidInquiry)
    		txnId = rqtPrepaidVoidInquiry;
    	else
    		txnId = rqtPrepaidVoid;
    	reqtyp = 'A'; 
        break;
//--@agmr - brizzi2   

    case mnuAktivasiDeposit:
    	txnId = rqtAktivasi;
    	reqtyp = 'A';
    	break;
    case mnuPrepaidSettlement:
    	txnId = rqtPrepaidSettlement;
    	reqtyp = 'A';
    	break;
#endif                
// -- @agmr - BRI	
#ifdef ABSENSI
		case mnuAbsen:
		case mnuDatang:
		case mnuPulangs:
		case mnuIstirahatA:
		case mnuIstirahatB:
		case mnuLemburA:
		case mnuLemburB:
		case mnuShifts:
		case mnuShiftsIA:
		case mnuShiftsIB:
		case mnuShiftsIIA:
		case mnuShiftsIIB:
		case mnuShiftsIIIA:
		case mnuShiftsIIIB:
			txnId = rqtAbsensi;
			reqtyp = 'A';
			break;
#endif
	case mnuSIMBaruA :
	case mnuSIMBaruAU :
	case mnuSIMBaruB1 :
	case mnuSIMBaruB1U :
	case mnuSIMBaruB2 :
	case mnuSIMBaruB2U :
	case mnuSIMBaruC :
	case mnuSIMBaruD :
	case mnuSIMPanjangA :
	case mnuSIMPanjangAU :
	case mnuSIMPanjangB1 :
	case mnuSIMPanjangB1U :
	case mnuSIMPanjangB2 :
	case mnuSIMPanjangB2U :
	case mnuSIMPanjangC :
	case mnuSIMPanjangD :
	case mnuSIMPeningkatanAU :
	case mnuSIMPeningkatanB1 :
	case mnuSIMPeningkatanB1U :
	case mnuSIMPeningkatanB2 :
	case mnuSIMPeningkatanB2U :
	case mnuSIMPenurunanA :
	case mnuSIMPenurunanAU :
	case mnuSIMPenurunanB1 :
	case mnuSIMPenurunanB1U :
	case mnuSIMPenurunanB2 :
	case mnuSIMPenurunanB2U :
	case mnuSIMGolSamA :
	case mnuSIMGolSamAU :
	case mnuSIMGolSamB1 :
	case mnuSIMGolSamB1U :
	case mnuSIMGolSamB2 :
	case mnuSIMGolSamB2U :
	case mnuSIMGolSamC :
	case mnuSIMGolSamD :
	case mnuSIMMutasiPeningkatanAU :
	case mnuSIMMutasiPeningkatanB1 :
	case mnuSIMMutasiPeningkatanB1U :
	case mnuSIMMutasiPeningkatanB2 :
	case mnuSIMMutasiPeningkatanB2U :
	case mnuSIMMutasiPenurunanA :
	case mnuSIMMutasiPenurunanAU :
	case mnuSIMMutasiPenurunanB1 :
	case mnuSIMMutasiPenurunanB1U :
	case mnuSIMMutasiPenurunanB2 :
	case mnuSIMMutasiPenurunanB2U :
	case mnuSIMHilangA :
	case mnuSIMHilangAU :
	case mnuSIMHilangB1 :
	case mnuSIMHilangB1U :
	case mnuSIMHilangB2 :
	case mnuSIMHilangB2U :
	case mnuSIMHilangC :
	case mnuSIMHilangD :
	case mnuSIMHilangPeningkatanAU :
	case mnuSIMHilangPeningkatanB1 :
	case mnuSIMHilangPeningkatanB1U :
	case mnuSIMHilangPeningkatanB2 :
	case mnuSIMHilangPeningkatanB2U :
	case mnuSIMHilangPenurunanA :
	case mnuSIMHilangPenurunanAU :
	case mnuSIMHilangPenurunanB1 :
	case mnuSIMHilangPenurunanB1U :
	case mnuSIMHilangPenurunanB2 :
	case mnuUjiSim :
	case mnuBPKBBaruRoda2 :
	case mnuBPKBBaruRoda4 :
	case mnuBPKBGantiRoda2 :
	case mnuBPKBGantiRoda4 :
	case mnuSTCK :
	case mnuMutasiRanRoda2 :
	case mnuMutasiRanRoda4 :
		if(trtType == trtPembayaranSSBInquiry)
    			txnId = rqtPembayaranSSBInquiry;
    		else
    			txnId = rqtPembayaranSSB;
 		reqtyp = 'A'; 
        	break;

    case mnuDebitLogon:
        	txnId = rqtDebitLogon;
        	reqtyp = 'A';
        	break;
        				
		case mnuBalance:
			txnId = rqtBalance;
			reqtyp = 'A';
			break;
		default:
			MAPGETBYTE(traTxnId, txnId, lblKO);
			reqtyp = 'A';
			break;
	}
	if((key != mnuSettlement) && (key != mnuTMSLP))
	{
		ret = mapPutByte(traTxnType, txnId);
		CHK;
	}
	ret = mapGetByte(traTxnType, idx);
	CHK;
	trcFN("idx: %d\n", idx);
	VERIFY(rqtBeg < idx);
	VERIFY(idx < rqtEnd);
	mapMove(rqsBeg, (word) (idx - 1));

	ret = mapGet(rqsMti, mti, sizeof(mti));
	CHK;
	MAPGETBYTE(traOriginalTxnType, transTypeOris, lblKO);//@@REPAIR2
	if ((idx==rqtVoid) && (transTypeOris==trtPreCom)) //@@REPAIR2
		memcpy(mti+1,"\x02\x20",2); //@@REPAIR2 - overwrite the MTI for Void Precomm
		
	MAPGET(emvAid, Aid, lblKO);
	if((memcmp(&Aid[1], "\xA0\x00\x00\x03\x33", 5)==0)&&(idx==rqtTCUpload))//overwrite for TCUpload UP
		memcpy(mti+1,"\x06\x20",2);
	
	trcS("MessageType:");
	trcBN(mti + 1, lenMti);
	trcS("\n");
	ret = bufApp(req, mti + 1, lenMti);

	MAPPUTSTR(traMti, mti + 1, lblKO);//For batch upload & TC Upload

	memset(Bitmap, 0, sizeof(Bitmap));
	ret = mapGet(rqsBitmap, Bitmap, sizeof(Bitmap));
	CHK;

	ret = modifyBitmap(Bitmap + 1);
	CHK;

#ifdef LINE_ENCRYPTION
	//bitOn(Bitmap+1,STORE_TLE_FIELD); //@@AS0.0.10

	// populate the encrypted field

	if ((memcmp(mti+1,"\x01\x00",lenMti)==0) || (memcmp(mti+1,"\x02\x00",lenMti)==0) ||
	    (memcmp(mti+1,"\x02\x20",lenMti)==0) || (memcmp(mti+1,"\x03\x20",lenMti)==0) ||
	    (memcmp(mti+1,"\x04\x00",lenMti)==0) || (memcmp(mti+1,"\x05\x00",lenMti)==0) ||
	    (memcmp(mti+1,"\x06\x20",lenMti)==0)
#ifdef PREPAID
//        || (memcmp(mti+1,"\x05\x00",lenMti)==0)
#endif	    
        )
		need2Encrypt =1;

	//@@AS-SIMAS - start
	if (need2Encrypt) {
		encBuffLen=8; //start from index 8, first 8 bytes will be occupied by enc.bitmap
		tlv_len=0;
		memset(encBuff,0x00,ISO_BUFFER_LEN);

		bufInit(&buf, dat, ISO_BUFFER_LEN);   //initialize temporary buffer for current field
		memset(encryptedBitmap,0x00,lenBitmap);
		for (bit = 2; bit <= 8 * lenBitmap; bit++)
		{
			if(!bitTest(Bitmap + 1, bit))
				continue;

			if (!isEncryptedField(bit))
				continue;

			ret = getFld(&buf, bit);
			CHK;                    //retrieve the value from the data base

			ret=makeSingleFld(bit,&buf,fldBuff);
			CHK;
			fldLen=ret;


			bitOn(encryptedBitmap,bit); //set the encrypted Bitmap to on, if there is enc.field

			memcpy(encBuff+encBuffLen,fldBuff,fldLen);

			encBuffLen += fldLen;

		}

		memcpy(encBuff,encryptedBitmap,lenBitmap);
#ifdef _DEBUG_TLE_2
		hex_dump_char("--populated data--",encBuff,encBuffLen);
#endif //_DEBUG_TLE

        ret=mapGet(acqTLE,xsec_TLE,lenKEY);

#ifdef _DEBUG_TLE
		hex_dump_char("--TLE-KEY--",xsec_TLE,16);
#endif

		/* encrypt TLE data */
		if (! xsec_encrypt_data (xsec_TLE, encBuff, &encBuffLen))
			goto lblKO;

		memcpy(tlv_buf,"\x00\x03",2); // Version of encryption scheme
		tlv_len+=2;
		memcpy(tlv_buf+tlv_len,encBuff,encBuffLen);
		tlv_len+=encBuffLen;

		//tlv_buf and tlv_len contain the data for storedEncryptedField
		bitOn(Bitmap+1,STORE_TLE_FIELD);

		bitOn(Bitmap+1, isoMac);
	} //need2Encrypt

	//disable fDE-63 for Logon
	if (memcmp(mti+1,"\x08\x00",lenMti)==0) {
		if (trtType == trtDebitLogon) {
			bitOff(Bitmap+1, isoAddDat);
			bitOn(Bitmap+1,STORE_TLE_FIELD);
			bitOn(Bitmap+1, isoMac);
		}
	}


 	//@@AS-SIMAS - end
#endif //LINE_ENCRYPTION
	trcS("bmp:");
	trcBN(Bitmap + 1, lenBitmap);
	trcS("\n");
	ret = bufApp(req, Bitmap + 1, lenBitmap);
	CHK;

	bufInit(&buf, dat, ISO_BUFFER_LEN);   //initialize temporary buffer for current field
	for (bit = 2; bit <= 8 * lenBitmap; bit++)
	{
		if(!bitTest(Bitmap + 1, bit))
			continue;


#ifdef LINE_ENCRYPTION
		if (bit==STORE_TLE_FIELD) {
			if ((memcmp(mti+1,"\x08\x00",lenMti)==0) && (trtType == trtDebitLogon)){
				memcpy(tlv_buf,"\x00\x03",2);
				tlv_len=2;
				ret=mapGet(acqTMKRefNo,tmkRefNo,8);
				memcpy(&tlv_buf[tlv_len],tmkRefNo,8);
				tlv_len+=8;
				bufReset(&buf);
				ret = bufApp(&buf, (byte *) tlv_buf,tlv_len);
				CHK;
			}
			else {
				if (tlv_len>0) {
					bufReset(&buf);
					ret = bufApp(&buf, (byte *) tlv_buf,tlv_len);
					CHK;
				}
			}
		}
		else {
			if (bit != isoMac) { //@@AS-SIMAS : For DE-64 will be added in the next section
				ret = getFld(&buf, bit);
				CHK;                    //retrieve the value from the data base
				if (isEncryptedField(bit)) {
					//overwrite with null
					memset(null_buff,0x00,sizeof(null_buff));
					ret=bufOwr(&buf, 0, null_buff,bufLen(&buf));
					CHK;
				}
		}
		}
#else //LINE_ENCRYPTION
		ret = getFld(&buf, bit);
		CHK;                    //retrieve the value from the data base
#endif //LINE_ENCRYPTION
		if (bit != isoMac) { //@@AS-SIMAS : For DE-64 will be added in the next section
			ret = appFld(req, bit, &buf);
			CHK;                    //append it to the iso message
		}


		trcS("reqBuild req=\n");
		trcBN(bufPtr(req), bufLen(req));
		trcS("\n");
	}

#ifdef LINE_ENCRYPTION
	//calculate MAC
	if (memcmp(mti+1,"\x08\x00",lenMti)==0) {
		if (trtType == trtDebitLogon) {
#ifdef _DEBUG_TLE
			hex_dump_char("--MAC INPUT--",bufPtr(req),bufLen(req));
#endif
			ret=mapGet(acqTAKLogon,tak,lenKEY);
			if(ret<0) return ret;
#ifdef _DEBUG_TLE
			hex_dump_char("--TAK LOGON--",tak,lenKEY);
#endif
			calcMAC(tak,bufPtr(req),bufLen(req),mac);
#ifdef _DEBUG_TLE
			hex_dump_char("--MAC--",mac,8);
#endif
			bufReset(&buf);
			ret = bufApp(&buf, (byte *) mac,8);
			CHK;

			ret = appFld(req, isoMac, &buf);
			CHK;                    //append it to the iso message
		}
	}
	else {
#ifdef _DEBUG_TLE
			hex_dump_char("--MAC INPUT--",bufPtr(req),bufLen(req));
#endif
			ret=mapGet(acqTAK,tak,lenKEY);
			if(ret<0) return ret;
#ifdef _DEBUG_TLE
			hex_dump_char("--TAK--",tak,lenKEY);
#endif
			calcMAC(tak,bufPtr(req),bufLen(req),mac);
#ifdef _DEBUG_TLE
			hex_dump_char("--MAC--",mac,8);
#endif
			bufReset(&buf);
			ret = bufApp(&buf, (byte *) mac,8);
			CHK;

			ret = appFld(req, isoMac, &buf);
			CHK;                    //append it to the iso message

	}
#endif //LINE_ENCRYPTION
	return bufLen(req);
lblKO:
    //++@agmr
    if(ret == -101 || ret == -102)
        return ret;
    //--@agmr
	return -1;
}
