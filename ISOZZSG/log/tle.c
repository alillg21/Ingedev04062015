//Subversion reference
//$HeadURL: https://svn.ingenico.com/UTAPROJECTS/sea-partner/5W/trunk/ISO5WPH/log/tle.c $
//$Id: tle.c 2047 2009-05-22 03:58:55Z ajloreto $
#include <stdlib.h>
#include "string.h"
#include "stdio.h"
#include "log.h"
#include "bri.h"
#include "iso.h"
#include "prepaid.h" //@agmr

#include "quicklz.h"

#define ISO_BUFFER_LEN 2048 //@agmr

#ifdef DUMP_ISO_MESSAGE
extern int dumpIsoMessage(const byte * rsp, word len);
#endif

extern byte txnAsli; //@agmr BRI2
static char reqtyp = 0;         //'A': authorization; 'S': settlement; 'P': parameters download(TMS)
//#define ECHO_TEST_DATA "\x08\x00\xA0\x00\x01\x00\x00\x80\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x30\x30\x30\x36\x39\x20\x20\x20\x20\x20\x20\x20\x20\x01\x00"
#define ECHO_TEST_DATA "\x08\x00\xA0\x00\x01\x00\x00\x80\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x99\x00\x00\x00\x00\x20\x20\x20\x20\x20\x20\x20\x20\x00\x01"
enum eSta {
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
    dnlEnd,

    revAutBeg,
    revAutCheck,
    revAutSendReq,
    revAutRecvRsp,
    revAutEnd,

    tcUpBeg,
    tcUpSendReq,
    tcUpRecvRsp,
    tcUpEnd,

#ifdef PREPAID
    prepaidSetBeg,
    prepaidSetSendReq,
    prepaidSetRecvRsp,
    prepaidSetEnd,
#endif

    setBeg,                     //engine states: Settlement
    setSendReq,
    setRecvRsp,
    setBatSendReq,
    setBatRecvRsp,
    setEnd,
};

typedef struct sTleParam
{
	byte tmrG;                  //global timeout
	byte tmrF;                  //frame timeout
	byte tmrC;                  //character timeout
	byte tryS;                  //number of send trial
	byte tryR;                  //number of receive trial
}
tTleParam;

#define CHK if(ret<0) return ret;
static char chn[lenChn + 1];    //'M': Modem; 'T': TCPIP; 'R': RS232

#ifdef PREPAID
word uploadedPrepaidIdx[MAX_PREPAID_SETTLEMENT+1]; //uploadedPrepaidIdx[0] = banyaknya idx
#endif

static int CheckTMSRspCode(void)
{
	int ret;
	char RespCode[lenRspCod + 1];

	trcS("CheckTMSRspCode Beg\n");
	MAPGET(traRspCod, RespCode, lblKO);
	trcS("TMS Resp Code: ");
	trcBN((byte *) RespCode, lenRspCod);
	trcS("\n");
	CHECK(memcmp(RespCode, "00", lenRspCod) == 0, lblKO);

	trcS("Resp Code is 00\n");
	ret = 0;
	goto lblEnd;
lblKO:
	switch (RespCode[1])
	{
		case '1':
			usrInfo(infFileMiss); // Cancelled System Error N1: Requested file download does not exist or is missing
			break;
		case '2':
			usrInfo(infTMSUnAvail);   // Cancelled System Error N2: TMS server unavailable
			break;
		case '3':
			usrInfo(infTMSQueryFailed);   // Cancelled System Error N3: Database Query Failed
			break;
		case '4':
			usrInfo(infTMSDLTooLarge);    // Cancelled System Error N4: request download too large
			break;
		case '5':
			usrInfo(infTMSVerErr);    // Cancelled System Error N5: TMS2 cannot read version file for a software download
			break;
		case '6':
			usrInfo(infTMSBufOvrFlw); // Cancelled System Error N6: TMS error buffer overflow
			break;
		case '7':
			usrInfo(infTMSInvDLReq);  // Cancelled System Error N7: Invalid Download request
			break;
		case '8':
			usrInfo(infTMSRefNumInv); // Cancelled System Error N8: Invalid TMS Reference Number
			break;
		case 'A':
			usrInfo(infTMSUnknwnReq); // Cancelled System Error NA: Unknown Request
			break;
		case 'B':
			usrInfo(infTMSOutofBounds);   // Cancelled System Error NB: internal TMS2 error, value out of bounds
			break;
		case 'C':
			usrInfo(infTMSFileError); // Cancelled System Error NC: internal TMS2 error: file has not been created for download
			break;

		case 'Z':
			usrInfo(infTMSUndefError);    // Cancelled System Error NZ: undefined error
			break;

	}
	ret = -1;

lblEnd:
	trcS("CheckTMSRspCode End\n");
	return ret;

}

static int procAdviceUpload(void)
{
	byte txntype;
	char Rrn[lenRrn + 1];
	char buf[lenMnu + 1];
	int ret;
	word mnuItem, trxNum, idxCnt;
	byte TxnType;
	word TxnMnuItm;

	trcS("procAdviceUpload Beg\n");
	MAPGETBYTE(traTxnType, txntype, lblKO);

	switch (txntype)
	{
		case trtOffline:
			MAPGET(traRrn, Rrn, lblKO);
			MAPPUTSTR(logRrn, Rrn, lblKO);
			MAPGETWORD(regTrxNo, trxNum, lblKO);
			MAPGETWORD(traIdx, idxCnt, lblKO);
			while(idxCnt < trxNum)
			{
				mapMove(logBeg, (word) idxCnt);
				MAPGETWORD(logMnuItm, mnuItem, lblKO);
				if(mnuItem == mnuOffline)
				{
					MAPGET(logRrn, Rrn, lblKO);
					if(strlen(Rrn) == 0)
						break;
				}
				idxCnt++;
			}
			if((idxCnt <= trxNum) && (mnuItem == mnuOffline))
			{
				MAPPUTBYTE(traTxnType, trtOffline, lblKO);
				idxCnt++;
				MAPPUTWORD(traIdx, idxCnt, lblKO);
				ret = autSendReq;
			}
			else
			{
				MAPPUTWORD(regOfflineCnt, 0, lblKO);
				MAPGETBYTE(traTxnTypeBeforeOffline, TxnType, lblKO);
				MAPGETWORD(traMnuItmBeforeOffline, TxnMnuItm, lblKO);
				MAPPUTBYTE(traTxnType, TxnType, lblKO);
				MAPPUTWORD(traMnuItm, TxnMnuItm, lblKO);
				MAPGET(TxnMnuItm, buf, lblKO);
				MAPPUTSTR(traCtx, buf + 2, lblKO);

				MAPPUTBYTE(regLocType, 'T', lblKO);

				ret = autSendReq;
				if(TxnMnuItm == mnuSettlement)
					ret = setEnd;
				else
					ret = autSendReq;
			}

			break;

		default:
			ret = setEnd;
			break;
	}
	trcFN("procAdviceUpload: state=%d\n", ret);
	return ret;
lblKO:
	trcS("procAdviceUpload: state=0xFF \n");
	return 0xFF;
}

static int procBatchUpload(void)
{
	int ret;
	word idxCnt, trxNum;

	MAPGETWORD(regTrxNo, trxNum, lblKO);
	MAPGETWORD(traIdx, idxCnt, lblKO);
	if(idxCnt < trxNum)
	{
		MAPPUTBYTE(traTxnType, trtBatchUpload, lblKO);
		mapMove(logBeg, (word) idxCnt);
		idxCnt++;
		MAPPUTWORD(traIdx, idxCnt, lblKO);
		ret = setBatSendReq;
	}
	else
	{
		MAPPUTBYTE(traTxnType, trtSettlementReconcile, lblKO);
		MAPPUTBYTE(regLocType, 'T', lblKO);
		ret = setSendReq;
	}
	return ret;
lblKO:
	return 0xFF;
}

static byte getNextState(byte sta)
{
	int ret;
	card RspCod, totSze, offset;
	word mnuItem;
	byte mreRec;
//	byte txntype;
// @@OA - mulai
	byte Rc[3];

	memset (Rc, 0, sizeof(Rc));
	mapGet(traRspCod, Rc,2);
// @@OA - selesai
	MAPGETCARD(cvtRspCod, RspCod, lblKO);
// @@OA - mulai
	if( (Rc[0] < '0' || Rc[0] > '9') || (Rc[1] < '0' || Rc[1] > '9') )
		RspCod = 0xFFFF;
// @@OA - selesai

	MAPGETBYTE(traTmsMreRec, mreRec, lblKO);
	trcFN("resp code: %d\n", RspCod);
	trcFN("Mre Record: %d\n", mreRec);

	switch (sta)
	{
		case tcUpRecvRsp:
			MAPGETWORD(traMnuItmContext, mnuItem, lblKO);
			MAPPUTWORD(traMnuItm, mnuItem, lblKO);
			MAPPUTBYTE(regLocType, 'T', lblKO);
			ret = autSendReq;
			MAPPUTBYTE(TCFlag, 0, lblKO);
			mapReset(tclogBeg);//clear TC record
			break;
		case revAutRecvRsp:
			MAPGETWORD(traMnuItmContext, mnuItem, lblKO);
//++ @agmr - BRI - auto reversal - tidak perlu kirim msg 200 lagi.
			if(mnuItem == 0)
			{
				ret = setEnd;
				break;
			}
//-- @agmr - BRI - auto reversal
			MAPPUTWORD(traMnuItm, mnuItem, lblKO);
			MAPPUTBYTE(regLocType, 'T', lblKO);

			//++ @agmr - BRI2 - reversal
			MAPPUTBYTE(traTxnType,txnAsli,lblKO);
			txnAsli = 0;
			//-- @agmr - BRI2 - reversal
//++ @agmr
			if(mnuItem == mnuSettlement)
			    ret = setSendReq;
			else
//-- @agmr
			    ret = autSendReq;
			break;

		case autRecvRsp:
			switch (RspCod)
			{
				case 0:
                    // ++ @agmr - inquiry transaction - BRI

                    ret = pmtBRIKonfirmasiSesudahInquiry();
                    if(ret == 9999 || ret == 0)
                    {
                        ret = setEnd;
                        break;
                    }
                    else if(ret >= 1)
                    {
                    	mapPut(traRspCod, "", 2);
                        ret = autSendReq;
                    }
                    else if(ret < 0)
                        goto lblKO;

                    else
                    // -- @agmr - inquiry transaction - BRI
					    ret = procAdviceUpload();
					break;
				default:
					ret = setEnd;
					break;
			}
			break;
		case setRecvRsp:
			switch (RspCod)
			{
				case 0:
					ret = setEnd;
					break;
				case 95:
					MAPPUTBYTE(traTxnType, trtBatchUpload, lblKO);
					MAPPUTWORD(traIdx, 1, lblKO);   // first txn will already be sent here
					MAPPUTBYTE(regLocType, 'L', lblKO);
					//++@agmr
					mapMove(logBeg, 0);
					ret = setBatSendReq;
                    //--@agmr
					break;
				default:
					ret = setEnd;
					break;
			}
			break;
		case setBatRecvRsp:
			switch (RspCod)
			{
				case 0:
#ifdef TEST_BATCH_UPLOAD
{//hapus - test reversal
    char key;

    dspClear();
    dspLS(2,"BIKIN KELUAR 1 ??");
	key = acceptable_key(kbdVAL_kbdANN, 1000);
	if(key == kbdVAL)
	{
	    ret = setEnd;
	    break;
	}
}

#endif
				    MAPPUTBYTE(logIsUploaded,1,lblKO); //@agmr

#ifdef TEST_BATCH_UPLOAD
{//hapus - test reversal
    char key;

    dspClear();
    dspLS(2,"BIKIN KELUAR 2 ??");
	key = acceptable_key(kbdVAL_kbdANN, 1000);
	if(key == kbdVAL)
	{
	    ret = setEnd;
	    break;
	}
}

#endif

					ret = procBatchUpload();
					break;
				default:
					ret = setEnd;
					break;
			}
			break;
		case dnlRecvLoginRsp:
			RspCod = CheckTMSRspCode();
			switch (RspCod)
			{
				case 0:
					ret = dnlSendConTblReq;
					break;
				default:
					ret = setEnd;
					break;
			}
			break;
		case dnlRecvConTblRsp:
			RspCod = CheckTMSRspCode();
			switch (RspCod)
			{
				case 0:
					ret = dnlSendBinTblReq;
					break;
				default:
					ret = setEnd;
					break;
			}
			break;
		case dnlRecvBinTblRsp:
			RspCod = CheckTMSRspCode();
			switch (RspCod)
			{
				case 0:
#ifdef OLD_CODE
					ret = dnlSendIssTblReq;
#else
					ret = (mreRec>0) ? dnlSendBinTblReq : dnlSendIssTblReq; //@@AS0.0.32
#endif
					break;
				default:
					ret = setEnd;
					break;
			}
			break;
		case dnlRecvIssTblRsp:
			RspCod = CheckTMSRspCode();
			switch (RspCod)
			{
				case 0:
					ret = (mreRec > 0) ? dnlSendIssTblReq : dnlSendAcqTblReq;
					break;
				default:
					ret = setEnd;
					break;
			}
			break;
		case dnlRecvAcqTblRsp:
			RspCod = CheckTMSRspCode();
			switch (RspCod)
			{
				case 0:
					//ret = (mreRec>0)? dnlSendAcqTblReq: dnlSendLogOffReq;
					ret = (mreRec > 0) ? dnlSendAcqTblReq : dnlSendEmvReq;
					break;
				default:
					ret = setEnd;
					break;
			}
			break;
		case dnlRecvLogOffRsp:
			RspCod = CheckTMSRspCode();
			switch (RspCod)
			{
				case 0:
				default:
					ret = setEnd;
					break;
			}
			break;
		case dnlRecvEmvRsp:
			RspCod = CheckTMSRspCode();
			switch (RspCod)
			{
				case 0:
					ret = dnlSendEmvFileReq;
					break;
				default:
					ret = setEnd;
					break;
			}
			break;
		case dnlRecvEmvFileRsp:
			RspCod = CheckTMSRspCode();
			switch (RspCod)
			{
				case 0:
					ret = dnlSendEmvFileOpnReq;
					break;
				default:
					ret = setEnd;
					break;
			}
			break;
		case dnlRecvEmvFileOpnRsp:
			RspCod = CheckTMSRspCode();
			switch (RspCod)
			{
				case 0:
					ret = dnlSendEmvFileRedReq;
					break;
				default:
					ret = setEnd;
					break;
			}
			break;
		case dnlRecvEmvFileRedRsp:
			RspCod = CheckTMSRspCode();
			switch (RspCod)
			{
				case 0:
					MAPGETCARD(traTmsEmvFleSze, totSze, lblKO);
					MAPGETCARD(traTmsEmvFleOff, offset, lblKO);
					ret =
					    (offset >=
					     totSze) ? dnlSendEmvFileClsReq : dnlSendEmvFileRedReq;
					break;
				default:
					ret = setEnd;
					break;
			}
			break;
		case dnlRecvEmvFileClsRsp:
			RspCod = CheckTMSRspCode();
			switch (RspCod)
			{
				case 0:
					ret = dnlSendLogOffReq;
					break;
				default:
					ret = setEnd;
					break;
			}
			break;

#ifdef PREPAID
        case prepaidSetRecvRsp:
#ifdef TEST_PREPAID_SETTLEMENT
{
    char key;

    dspClear();
    dspLS(2,"PUTUSIN??");
	key = acceptable_key(kbdVAL_kbdANN, 1000);
	if(key == kbdVAL)
	{
	    goto lblKO;
	    break;
	}
}
#endif
			switch (RspCod)
			{
				case 0:
                    setLogIsUploaded();
                    ret = prepaidSetSendReq;
					break;
				default:
					ret = setEnd;
					break;
			}


			break;
#endif //PREPAID
		default:
			ret = 0xFF;
			break;
	}
	return ret;
lblKO:
	return 0xFF;
}

static int sendReq(tTleParam * par, byte sta)
{
	byte dReq[ISO_BUFFER_LEN];
	byte dTPDUReq[lenTPDU + lenBCDMsg + 1];
	byte bcdLReq[lenBCDMsg];
	byte bcdNii[lenNii + 1];
	char Nii[lenNII + 1];
	char tpduHead[4 + 1];
	char tpduModem[8 + 1];
	char tpduTCPIP[8 + 1];
	char tpduRS232[8 + 1];
//	char decLReq[lenBCDMsg + 1];
	tBuffer bReq;
	tBuffer bTPDUReq;
	int ret;
	word key = 0;
	byte incStan = 0;
	byte txnType;
	word MnuItm; //@agmr

	VERIFY(par);
	trcS("sendReq: Beg\n");

	memset(dReq, 0, sizeof(dReq));
	memset(dTPDUReq, 0, sizeof(dTPDUReq));

	bufInit(&bReq, dReq, sizeof(dReq));
	bufInit(&bTPDUReq, dTPDUReq, sizeof(dTPDUReq));

	MAPGET(appChn, chn, lblKO);
	ret = 0;
	switch (reqtyp)
	{
		case 'R':
		case 'A':
		case 'S':
			incStan = 1;
			key = acqNII;
			break;
		case 'P':
			incStan = 0;
			key = appNII;
			break;
		case 'U':
			incStan = 0;
			key = acqNII;
			break;
		default:
			break;
	}
	MAPGET(key, Nii, lblKO);

#ifdef PREPAID
//++ @agmr
	ret = getPrepaidNii(Nii);
	CHECK(ret >= 0, lblKO);
//-- @agmr
#endif

#ifndef REVERSAL_TERPISAH
{
    byte locTyp;

    MAPGETBYTE(regLocType, locTyp, lblKO);
    if(locTyp == 'R')
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
    else
    {
        //simpan nii ke traRevNII
        ret = getJenisLog();
        if(ret >= 0) //trans Mini ATM, Tunai dan Card Service
        {
            ret = mapPut(traRevNII,Nii,lenNII);
            CHK;
        }
        else
        {
            ret = mapPut(traRevNII,"\x00\x00\x00",lenNII);
            CHK;
        }
    }
}
#endif

	fmtPad(Nii, -(lenNII + 1), '0');
	hex2bin(bcdNii, Nii, 0);

	if(incStan)
	{
	    MAPGETBYTE(traTxnType, txnType, lblKO);
	    //++ @agmr - trx void, stan yang dikirim = stan trx terakhir +1
        //           bukan stan trx yang divoid + 1
	    if((txnType == trtVoid || txnType == trtVoidTarikTunai
		|| txnType == trtPrepaidVoid //@agmr - brizzi2) 
		) && reqtyp != 'R')
	    {
	        card dupStan;

	        MAPGETCARD(regDupSTAN, dupStan, lblKO);
	        MAPPUTCARD(regSTAN, dupStan, lblKO);
	    }

		//if((MnuItm != mnuPreCancel) ||(MnuItm != mnuPreCom))
			ret = incCard(regSTAN);
		CHECK(ret >= 0, lblKO);
	}
	//--@agmr - echo test
	ret = reqBuild(&bReq);
	CHECK(ret > 0, lblKO);

	//Build TPDU
	ret = 0;
	MAPGET(apptpduHead, tpduHead, lblKO);
	switch (chn[0])
	{
		case 'M':                //modem
			MAPGET(apptpduModem, tpduModem, lblKO);
			ret = bufApp(&bTPDUReq, (byte *) tpduHead, 1);    //Standard
			ret = bufApp(&bTPDUReq, bcdNii, lenNii);  //Source NII
			ret = bufApp(&bTPDUReq, (byte *) tpduModem, 2);   //Destination NII 30 for Modem
			break;
		case 'T':                //TCP/IP
		case 'G':                //GPRS
		case 'P':
			MAPGET(apptpduTCPIP, tpduTCPIP, lblKO);
			num2bin(bcdLReq, bufLen(&bReq) + lenTPDU, sizeof(bcdLReq));

			ret = bufApp(&bTPDUReq, bcdLReq, 2);  //Message Length
			ret = bufApp(&bTPDUReq, (byte *) tpduHead, 1);    //Standard
			ret = bufApp(&bTPDUReq, bcdNii, lenNii);  //Source NII
			ret = bufApp(&bTPDUReq, (byte *) tpduTCPIP, 2);   //destination NII 80 for TCPIP
			break;
//		case 'T':                //TCP/IP
//		case 'G':
//			MAPGET(apptpduTCPIP, tpduTCPIP, lblKO);
//			num2dec(decLReq, bufLen(&bReq) + lenTPDU, sizeof(decLReq));
//			fmtPad(decLReq, -lenBCDMsg * 2, '0');
//			hex2bin(bcdLReq, decLReq, 0);
//
//			ret = bufApp(&bTPDUReq, bcdLReq, 2);  //Message Length
//			ret = bufApp(&bTPDUReq, (byte *) tpduHead, 1);    //Standard
//			ret = bufApp(&bTPDUReq, bcdNii, lenNii);  //Source NII
//			ret = bufApp(&bTPDUReq, (byte *) tpduTCPIP, 2);   //destination NII 80 for TCPIP
//			break;
		case 'R':
			MAPGET(apptpduRS232, tpduRS232, lblKO);
			ret = bufApp(&bTPDUReq, (byte *) tpduHead, 1);    //Standard
			ret = bufApp(&bTPDUReq, bcdNii, lenNii);  //Source NII
			ret = bufApp(&bTPDUReq, (byte *) tpduRS232, 2);   //Destination NII 665 for RS232
			break;
		default:
			break;
	}

	CHECK(ret >= 0, lblKO);
	VERIFY(ret);

	trcS("TPDU:");
	trcBN(bufPtr(&bTPDUReq), bufLen(&bTPDUReq));
	trcS("\n");

	ret = bufIns(&bReq, 0, bufPtr(&bTPDUReq), bufLen(&bTPDUReq));
	CHECK(ret > 0, lblKO);

	ret = isReversibleSend();
	CHECK(ret >= 0, lblKO);
	trcFN("ret = %d", ret);
	if(ret > 0)
	{
		revSave();
		MAPPUTBYTE(revReversalFlag, 1, lblKO);
	}

	mapPut(appRequest, bufPtr(&bReq), bufLen(&bReq));
	MAPPUTWORD(appReqLen, bufLen(&bReq), lblKO);

	if(debugReqFlag==1)
		hex_dump_char("Request", bufPtr(&bReq), bufLen(&bReq));

//    if(reqtyp == 'R') //@agmr - BRI2
    if(sta == revAutSendReq)
    {
			dspClear();
			dspLS(1, "    Reversal    ");
			dspLS(2, "  Transmitting  ");
			//hex_dump_char("Request", bufPtr(&bReq), bufLen(&bReq));
    }
    else if(reqtyp != 'P')
		{
			dspClear();
			dspLS(1, "   Proccessing  ");
			dspLS(2, "  Transmitting  ");
		}

#ifdef DUMP_ISO_MESSAGE
	 if(chn[0]=='T' || chn[0] =='G' || chn[0] == 'P')
		dumpIsoMessage(bufPtr(&bReq)+2, bufLen(&bReq));
	 else
	 	dumpIsoMessage(bufPtr(&bReq), bufLen(&bReq));
#endif

	ret = comSendBuf(bufPtr(&bReq), bufLen(&bReq)); //send the request
	CHECK(ret == bufLen(&bReq), lblKO);

	ret = sta += 1;
	goto lblEnd;
lblKO:
//++@agmr
    if(ret == -101)
    {
        return setEnd;

    }
	ret = isReversibleSend();
	if(ret > 0)
	{
		//revSave();
		MAPPUTBYTE(revReversalFlag, 0, lblKO);
	}
//--@agmr
	usrInfo(infSendingFailed);
	return -1;
lblEnd:
	trcFN("sendReq: ret=%d\n", ret);
	return ret;
}

static int UpdateDateTime()
{
	int ret;
	char Date[lenDat + 1];
	char Time[lenTim + 1];
	char Datebin[lenDat + 1];
	char Timebin[lenTim + 1];
	char DatTim[lenDatTim + 1];
	char NewDateTime[lenDatTim + 1];
	char traNew[lenDatTim + 1];

	MEM_ZERO(Timebin); MEM_ZERO(Datebin);
	MEM_ZERO(Date); MEM_ZERO(Time);
	MEM_ZERO(DatTim);

	MAPGET(traRspBit12, Timebin, lblKO);
	MAPGET(traRspBit13, Datebin, lblKO);
	MAPGET(traDatTim, DatTim, lblKO);

	bin2hex(Time, Timebin, 3);
	bin2hex(Date, Datebin, 2);

	memcpy(NewDateTime, DatTim + 2, 2);
	memcpy(NewDateTime + 2, Date, 4);
	memcpy(NewDateTime + 6, Time, 6);

	ret = setDateTime(NewDateTime);
	CHK;
	strcpy(traNew, "20");
	memcpy(traNew + 2, NewDateTime, lenDatTim - 2);
	MAPPUTSTR(traDatTim, traNew, lblKO);

	return ret;
	lblKO:
		return -1;
}

static int recvRsp(tTleParam * par, byte sta)
{
	int ret;
	byte dRsp[ISO_BUFFER_LEN];
	tBuffer bRsp;
	word MnuItm; //@agmr
	byte emvFlag=0; //@agmr
	char RspCod[lenRspCod+1];//@agmr

	VERIFY(par);
	trcS("recvRsp: Beg\n");

	memset(dRsp, 0, sizeof(dRsp));
	bufInit(&bRsp, dRsp, sizeof(dRsp));

//	if(reqtyp == 'R')
    if(sta == revAutRecvRsp)
	{
		dspClear();
		dspLS(1, "    Reversal   ");
		dspLS(2, "    Receiving  ");
	}
	else if(reqtyp == 'U')
	{
		dspClear();
		dspLS(1, "   TC Upload    ");
		dspLS(2, "  Please Wait   ");
	}
	else if(reqtyp != 'P')
    	{
		dspClear();
		dspLS(1, "   Processing  ");
		dspLS(2, "    Receiving  ");
	}

	ret = comRecvBuf(&bRsp, NULL, par->tmrF);

#ifdef TEST_TERIMA_GPRS
{
    char buf[2048];

    sprintf(buf,"len = %i",bufLen(&bRsp));
    prtS(buf);

    memset(buf,0,sizeof(buf));
    hex_dump_char("Response", bufPtr(&bRsp), bufLen(&bRsp));
}
#endif

#ifdef TEST_REVERSAL
{//hapus - test reversal
    char key;

    dspClear();
    dspLS(2,"BIKIN TIMEOUT ??");
	key = acceptable_key(kbdVAL_kbdANN, 1000);
	if(key == kbdVAL)
	    ret = 0;
}
#endif
//++ @agmr - BRI - auto reversal
  	if(ret<=0) // BUKAREVERSAL
  	{
  	    //@agmr - BRI4 - field 62 naik, bila timeout/gagal terima
//  	    if(reqtyp == 'A') //*** TIDAK BISA DIPAKAI KARENA static global reqtyp ada di 2 file
                              //
        if(sta == autRecvRsp)
        {
        	MAPGETWORD(traMnuItm, MnuItm, lblKO);
            	//if((MnuItm != mnuPreCancel) ||(MnuItm != mnuPreCom))
  	       	incCard(regInvNum); //jangan dikasih ret =
  	    //@agmr - BRI4
        }
  		if(isReversibleSend())
  		{
  			ret = -3;
  			goto lblEnd;
  		}
  	}
//-- @agmr - BRI - auto reversal
	CHECK(ret > 0, lblKO);
	trcS("buffrcv:");
	trcBN(dRsp, 20);
	trcS("\n");

#ifdef DUMP_ISO_MESSAGE
    switch (chn[0]) {
      case 'M':                //modem
      case 'R':                //RS232
          dumpIsoMessage(bufPtr(&bRsp), bufLen(&bRsp));
          break;
      case 'T':                //TCP/IP
      case 'G':                //GPRS
          dumpIsoMessage(bufPtr(&bRsp)+2, bufLen(&bRsp));
          break;
      default:
          break;
    }
#endif	//DUMP_ISO_MESSAGE

	mapPut(appRespon, bufPtr(&bRsp), bufLen(&bRsp));
	MAPPUTWORD(appRspLen, bufLen(&bRsp), lblKO);

	if(debugRspFlag==1)
		hex_dump_char("Response", bufPtr(&bRsp),bufLen(&bRsp));

	//Remove TPDU
	switch (chn[0])
	{
		case 'M':                //modem
		case 'R':                //RS232
			ret = bufDel(&bRsp, 0, lenTPDU);  //remove TPDU from the message
			break;
		case 'T':                //TCP/IP
		case 'G':                //GPRS
			ret = bufDel(&bRsp, 0, lenBCDMsg + lenTPDU);  //remove Message Length and TPDU from the message
			break;
		default:
			break;
	}
	CHECK(ret >= 0, lblKO);

    ret = rspParse(bufPtr(&bRsp), bufLen(&bRsp));   //parse response message
	//CHECK(ret >= 0, lblKO); // AJ note: dont know why this is commented out
	if (ret == -1)
        return -2;
	if(ret == -100)
	    goto lblEnd;

	CHECK(ret != -80, lblEMVFileNotFound);  // to processs exception where no emv files loaded in tms server

//++ @agmr - periksa TID, MID dan STAN
    if(sta == autRecvRsp || sta == revAutRecvRsp || sta == tcUpRecvRsp || sta ==setRecvRsp)
    {
        ret = checkMti();
        if(ret < 0)
            goto lblPlsTryAgain;
                    
        ret = checkTid();
        if(ret < 0)
            goto lblPlsTryAgain;

        ret = checkMid();
        if(ret < 0)
            goto lblPlsTryAgain;

        ret = checkStan();
        if(ret < 0)
            goto lblPlsTryAgain;
    }
//-- @agmr - periksa TID, MID dan STAN

	if(reqtyp != 'U')
		UpdateDateTime();

//++ @agmr - bila emv, resp code != 0, maka clear reversal
    memset(RspCod,0,sizeof(RspCod));
    MAPGET(traRspCod, RspCod, lblKO);
	if(memcmp(RspCod,"00",2)==0)
	    emvFlag = 0;
	else
	    emvFlag = 1;

//-- @agmr

	ret = emvFullProcessCheck();//if emv no clear reversal
//	if(ret == 0)
    if(ret != 1 || emvFlag == 1)
	{
		ret = isReversibleRecv();
		CHECK(ret >= 0, lblKO);
		if(ret)
		{
#ifdef PREPAID
            MAPGETWORD(traMnuItm, MnuItm, lblKO);
            if(MnuItm != mnuTopUpOnline
               && MnuItm != mnuPrepaidRedeem
               && MnuItm != mnuAktivasiDeposit
		//++ @agmr - brizzi2               
               && MnuItm != mnuPrepaidReaktivasi
               && MnuItm != mnuPrepaidVoid
//-- @agmr - brizzi2
		)
#endif
            {
			    MAPPUTBYTE(revReversalFlag, 0, lblKO);
			    mapReset(revBeg);
			}
		}
	}
	ret = getNextState(sta);

	goto lblEnd;

lblPlsTryAgain:
    ret = -100;
    usrInfo(infPlsTryAgain);
    goto lblEnd;
lblKO:
	ret = -1;
	usrInfo(infReceivingFailed);
	ret = -100; //BRI3 - sudah menampilkan error	 //BUKAREVERSAL
	goto lblEnd;
lblEMVFileNotFound:
	usrInfo(infEmvFileNotFound);
	ret = getNextState(setEnd); // force end of TMS download
lblEnd:
	trcFN("recvRsp: ret=%d\n", ret);
	return ret;
}

static int tleInit(tTleParam * par)
{
	int ret;
	word keyRxTO = 0, keyICTO = 0, keyConTO = 0;

	trcS("tleAut: Beg\n");

	switch (reqtyp)
	{
		case 'A':
			keyRxTO = acqTxnRxTO;
			keyICTO = acqTxnICTO;
			keyConTO = acqTxnConTO;
			break;
		case 'S':
			keyRxTO = acqTxnRxTO;
			keyICTO = acqTxnICTO;
			keyConTO = acqTxnConTO;
			break;
		default:
			keyRxTO = appDefRxTO;
			keyICTO = appDefICTO;
			keyConTO = appDefConTO;
			break;
	}

	MAPGETWORD(keyConTO, par->tmrG, lblKO);
	MAPGETWORD(keyRxTO, par->tmrF, lblKO);
	MAPGETWORD(keyICTO, par->tmrC, lblKO);
	par->tmrG = 180;            // tms settings for this is too low for connection,, reset to default
	par->tryS = 3;
	par->tryR = 3;

	trcFN("par->tmrG = %d\n", par->tmrG);
	trcFN("par->tmrF = %d\n", par->tmrF);
	trcFN("par->tmrC = %d\n", par->tmrC);
	goto lblEnd;
lblKO:
	return -1;
lblEnd:
	trcS("tleAut: End\n");
	return ret;
}

static int tleAut(tTleParam * par, byte sta)
{  //tle automate
	int ret, tmr = 0;

	VERIFY(par);

	trcS("tleAut: Beg\n");
	while(sta < setEnd)
	{
		trcFN("sta :%d\n", sta);
		MAPPUTBYTE(traSta, sta, lblKO);
		switch (sta)
		{
			case autSendReq:
				ret = sendReq(par, sta);
				break;
			case autRecvRsp:
				ret = recvRsp(par, sta);
				if(ret == -2)
					return -2;
				break;
			case revAutSendReq:
				ret = sendReq(par, sta);
				break;
			case revAutRecvRsp:
				ret = recvRsp(par, sta);
				break;
			case tcUpSendReq:
				ret = sendReq(par, sta);
				break;
			case tcUpRecvRsp:
				ret = recvRsp(par, sta);
				break;
			case setSendReq:
				usrInfo(infSettlement);
				ret = sendReq(par, sta);
				break;
			case setRecvRsp:
				ret = recvRsp(par, sta);
				break;
			case setBatSendReq:
				usrInfo(infBatUpl);
//++ @agmr - multi settle
                {
                    byte settlementAcq, currAcq;
                    byte tmp;

                    MAPGETBYTE(appAcqSettlementIdx,settlementAcq,lblKO);
                    MAPGETBYTE(logAcqIdx,currAcq,lblKO);
                    if(settlementAcq != currAcq)
                    {
                        mapPut(traRspCod, "00",2);
                        ret = getNextState(setBatRecvRsp);
                        break;
                    }
                    MAPGETBYTE(logIsUploaded,tmp,lblKO);
                    if(tmp==1)
                    {
                        mapPut(traRspCod, "00",2);
                        ret = getNextState(setBatRecvRsp);
                        break;
                    }
                }
//++ @agmr - multi settle
				ret = sendReq(par, sta);
				break;
			case setBatRecvRsp:
				ret = recvRsp(par, sta);
				break;
			case dnlSendLoginReq:
				MAPPUTBYTE(traTmsMsg, tmsLogin, lblKO);
				ret = sendReq(par, sta);
				usrInfo(infTrans);
				break;
			case dnlSendConTblReq:
				MAPPUTBYTE(traTmsMsg, tmsDnlConTbl, lblKO);
				MAPPUTBYTE(traTxnType, trtTMSDnlTbl, lblKO);
				ret = sendReq(par, sta);
				usrInfo(infEMVCredit);
				usrInfo(infTrmCon);
				break;
			case dnlSendBinTblReq:
				MAPPUTBYTE(traTmsMsg, tmsDnlBinTbl, lblKO);
				ret = sendReq(par, sta);
				usrInfo(infCrdRng);
				break;
			case dnlSendIssTblReq:
				MAPPUTBYTE(traTmsMsg, tmsDnlIssTbl, lblKO);
				ret = sendReq(par, sta);
				usrInfo(infIss);
				break;
			case dnlSendAcqTblReq:
				MAPPUTBYTE(traTmsMsg, tmsDnlAcqTbl, lblKO);
				ret = sendReq(par, sta);
				usrInfo(infAcq);
				break;
			case dnlSendLogOffReq:
				MAPPUTBYTE(traTmsMsg, tmsDnlConTbl, lblKO);
				MAPPUTBYTE(traTxnType, trtTMSLOGOFF, lblKO);
				CHK;
				ret = sendReq(par, sta);
				break;
			case dnlRecvLoginRsp:
				usrInfo(infRcving);
				ret = recvRsp(par, sta);
				break;
			case dnlRecvConTblRsp:
				ret = recvRsp(par, sta);
				break;
			case dnlRecvBinTblRsp:
				ret = recvRsp(par, sta);
				break;
			case dnlRecvIssTblRsp:
				ret = recvRsp(par, sta);
				break;
			case dnlRecvAcqTblRsp:
				ret = recvRsp(par, sta);
				break;
			case dnlRecvLogOffRsp:
				ret = recvRsp(par, sta);
				break;
			case dnlSendEmvReq:
				MAPPUTBYTE(traTmsMsg, tmsEmvReq, lblKO);
				MAPPUTBYTE(traTxnType, trtTMSEmvReq, lblKO);
				ret = sendReq(par, sta);
				usrInfo(infEmvPrm);
				break;
			case dnlRecvEmvRsp:
				ret = recvRsp(par, sta);
				break;
			case dnlSendEmvFileReq:
				MAPPUTBYTE(traTmsMsg, tmsEmvFileDnl, lblKO);
				MAPPUTBYTE(traTxnType, trtTMSEmvFleDnl, lblKO);
				ret = sendReq(par, sta);
				break;
			case dnlRecvEmvFileRsp:
				ret = recvRsp(par, sta);
				break;
			case dnlSendEmvFileOpnReq:
				MAPPUTBYTE(traTmsMsg, tmsEmvFileOpn, lblKO);
				ret = sendReq(par, sta);
				break;
			case dnlRecvEmvFileOpnRsp:
				ret = recvRsp(par, sta);
				break;
			case dnlSendEmvFileRedReq:
				MAPPUTBYTE(traTmsMsg, tmsEmvFileRed, lblKO);
				ret = sendReq(par, sta);
				break;
			case dnlRecvEmvFileRedRsp:
				ret = recvRsp(par, sta);
				break;
			case dnlSendEmvFileClsReq:
				MAPPUTBYTE(traTmsMsg, tmsEmvFileCls, lblKO);
				ret = sendReq(par, sta);
				break;
			case dnlRecvEmvFileClsRsp:
				ret = recvRsp(par, sta);
				break;
#ifdef PREPAID
            case prepaidSetSendReq:
//                ret=procPrepaidUpload();
//                if(ret>0)
                    ret = sendReq(par, sta);
//                else
//                    ret = setEnd;
                break;
            case prepaidSetRecvRsp:
                ret = recvRsp(par, sta);
                break;
#endif
			default:
				VERIFY((sta == setEnd) || (sta == dnlEnd) || (sta == autEnd)
				       || (sta == revAutEnd));
				break;
		}
		CHECK(ret >= 0, lblKO);
		tmr = tmrGet(3);
		trcFN("tmr = %d", tmr);
		CHECK(tmr, lblKO);
		//tmrPause(1); //added this to make rs232 tms connection work
		//tmrSleep(1000);
		if(kbdKey() == kbdANN)
			sta = setEnd;
		sta = ret;

	}
	ret = 1;
	goto lblEnd;
lblKO:
	if(ret != -3 && ret != -100)
		ret = -1;
lblEnd:
	kbdStop();
	tmrStop(3);
	trcFN("tleAut: ret=%d\n", ret);
	return ret;
}

static byte tleSetFirstState(char ses)
{
	byte revflag,tcflag,sta = 0;
	int ret;
	word mnuItem;
#ifdef REVERSAL_TERPISAH
	byte idx; //@agmr - BRI - auto reversal
#endif
    byte txnType; //@agmr - BRI

	trcS("tleSetFirstState: Beg\n");

//++@agmr - echo test
    MAPGETBYTE(traTxnType,txnType,lblKO);
    if(txnType == trtEchoTest ||
       txnType == trtDebitLogon || //@@AS0.0.26
       txnType == trtTLEKeyDownload)
    {
        return autSendReq;
    }
//--@agmr - echo test

	reqtyp = ses;
	switch (ses)
	{
		case 'R':
		case 'A':
		case 'U':
		case 'S':
		    //++ @agmr - BRI3
#ifdef REVERSAL_TERPISAH
			MAPGETBYTE(traAcqIdx, idx, lblKO);
			mapMove(revBeg,idx);
#else
            ret = getBriReversalIdx();
            if(ret <=0)
                goto lblKO;
#endif
            //-- @agmr - BRI3

			MAPGETBYTE(revReversalFlag, revflag, lblKO);
			MAPGETBYTE(TCFlag, tcflag, lblKO);
			MAPGETWORD(traMnuItm, mnuItem, lblKO);
			MAPPUTWORD(traMnuItmContext, mnuItem, lblKO);

			if(revflag == 1)
			{
				switch (mnuItem)
				{
					case mnuReversal:
						sta = autSendReq;
						break;

					default:
						MAPPUTBYTE(regLocType, 'R', lblKO);
						MAPPUTWORD(traMnuItm, mnuReversal, lblKO);
						sta = revAutSendReq;
						break;
				}
			}
			else if(tcflag == 1)
			{
				MAPPUTBYTE(regLocType, 'U', lblKO);
				MAPPUTWORD(traMnuItm, mnuTCUpload, lblKO);
				sta = tcUpSendReq;
			}
			else if(ses == 'A')
			{
				sta = autSendReq;
			}
			else
			{
				sta = setSendReq;
			}
			break;
		case 'P':
			sta = dnlSendLoginReq;
			break;
//		case 'S':
//			sta = setSendReq;
//			break;
		default:
			sta = 0;
			break;
	}
#ifdef PREPAID
    MAPGETWORD(traMnuItm, mnuItem, lblKO);
	if(mnuItem == mnuPrepaidSettlement)
	{
		sta = prepaidSetSendReq;
	}
#endif
	goto lblEnd;
lblKO:
	sta = 0;
lblEnd:
	trcFN("tleSetFirstState: sta=%d\n", sta);
	return sta;
}

/** This function starts the state engine to perform the authorization.
 * \param    ses (I-) authorization state to process.
 *
 * \header log\\log.h
 * \source log\\tle.c
*/
int tleSession(char ses)
{
	int ret;
	tTleParam par;
	byte sta= 0;

	trcS("tleSession: Beg\n");

	reqtyp = ses;

	ret = tleInit(&par);
	CHECK(ret >= 0, lblKO);

	ret = kbdStart(1);
	CHECK(ret >= 0, lblKO);

	//to reset tmr 3
	tmrStop(3);
	ret = tmrStart(3, (par.tmrG) * 100);
	CHECK(ret >= 0, lblKO);
	trcFN("(par.tmrG) * 100 = %d\n", (par.tmrG) * 100);
	trcFN("delay = %d\n", ret);

	sta = tleSetFirstState(ses);

	ret = tleAut(&par, sta);
	CHECK(ret >= 0, lblKO);
	ret = 1;
	goto lblEnd;
lblKO:
//++ @agmr - BRI - auto reversal
	if(ret != -3 && ret != -100) //BRI3
//-- @agmr - BRI - auto reversal
		ret = -1;
lblEnd:
	trcFN("tleSession: ret=%d\n", ret);
	return ret;
}


//++ @agmr - BRI - auto reversal

int doBRIReversal()
{
	byte flag;
	int ret;
	byte i;
	tTleParam par;

    //++ @agmr - BRI3
#ifndef REVERSAL_TERPISAH
    ret = getBriReversalIdx();
    if(ret <=0)
        goto lblKO;
#endif
    //-- @agmr - BRI3
//    MAPGETBYTE(traAcqIdx,i,lblKO);
//	mapMove(revBeg, i);
	MAPGETBYTE(revReversalFlag, flag, lblNoReversal);
	if(flag == 1)
	{
		dspClear();
		dspLS(1," AUTO REVERSAL");

		MAPPUTWORD(traMnuItm, mnuReversal, lblKO);
		MAPPUTWORD(traMnuItmContext,0,lblKO); //agar supaya tidak kirim msg 200 lagi.
		reqtyp = 'R';

		for(i=0;i<1;i++)
		{
        	ret = tleInit(&par);
        	CHECK(ret >= 0, lblKO);

        	ret = kbdStart(1);
        	CHECK(ret >= 0, lblKO);

        	//to reset tmr 3
        	tmrStop(3);
        	ret = tmrStart(3, (par.tmrG) * 100);
        	CHECK(ret >= 0, lblKO);
        	trcFN("(par.tmrG) * 100 = %d\n", (par.tmrG) * 100);
        	trcFN("delay = %d\n", ret);

    		MAPPUTBYTE(regLocType, 'R', lblKO);
        	ret = tleAut(&par, revAutSendReq);
        	if(ret >= 0)
        	{
        	    MAPPUTBYTE(revReversalFlag, 0, lblKO);
        	    break;
        	}
//        	CHECK(ret >= 0, lblKO);
        }
	}
	ret = 1;
	goto lblEnd;

lblNoReversal:
	ret = 0;
	goto lblEnd;
lblKO:
	ret = -1;
	goto lblEnd;
lblEnd:
	trcFN("End saveReversalData ret=%d\n", ret);
	return ret;
}
//-- @agmr - BRI - auto reversal

//++ @agmr
int checkTid()
{
    int ret;
    char reqTid[20],respTid[20];
    word key;

    memset(reqTid,0,sizeof(reqTid));
    memset(respTid,0,sizeof(respTid));
    ret = mapGet(traRspBit41,respTid,8);
    if(ret < 0)
        return -1;
    if(respTid[0] == 0) //@agmr - jika tidak dikembalikan TID, dianggap TID sama
        return 1;

	key = ((reqtyp == 'P') ? appTid : acqTID);
    MAPGET(key, reqTid, lblKO);

    if(memcmp(reqTid,respTid,8) != 0)
        return -1;

    return 1;

lblKO:
	return -1;
}

int checkMid()
{
    int ret;
    char reqMid[20],respMid[20];
    word key;

    memset(reqMid,0,sizeof(reqMid));
    memset(respMid,0,sizeof(respMid));
    ret = mapGet(traRspBit42,respMid,15);
    if(ret < 0)
        return -1;
    if(respMid[0] == 0) //@agmr - jika tidak dikembalikan MID, dianggap TID sama
        return 1;
	key = ((reqtyp == 'P') ? regTmsRefNum : acqMID);
    MAPGET(key, reqMid, lblKO);

    if(memcmp(reqMid,respMid,15) != 0)
        return -1;
    return 1;

lblKO:
	return -1;
}

int checkStan()
{
    int ret;
    char reqStan[20],respStan[20];
    word key;
	card STAN;
	char dec[2 * lenSTAN + 1];
//	byte bcd[lenSTAN];
	byte LocationType;
	byte txnType;

    memset(reqStan,0,sizeof(reqStan));
    memset(respStan,0,sizeof(respStan));
	ret = mapGetByte(regLocType, LocationType);
	CHK;

	switch (LocationType)
	{
		case 'R':
			key = revSTAN;
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

	MAPGETBYTE(traTxnType,txnType,lblKO);
	if(txnType == trtSettlement)
		key = regSTAN;

	ret = mapGetCard(key, STAN);
	CHK;

	num2dec(dec, STAN, 2 * lenSTAN);
	hex2bin(reqStan, dec, 0);

    ret = mapGet(traRspBit11,respStan,3);
    if(ret < 0)
        return -1;

    if(memcmp(reqStan,respStan,3) != 0)
        return -1;
lblKO:
    return 1;
}

int checkMti()
{
    int ret;
    char reqMti[2],respMti[2];

    ret = mapGet(traMti,reqMti,2);
    if(ret < 0)
        return -1;
        
    ret = mapGet(traMtiRsp,respMti,2);
    if(ret < 0)
        return -1;
        
    if(reqMti[1]+0x10 != respMti[1])
        return -1; 
        
    return 1;
}

int procPrepaidUpload(void)
{
	int ret;
	word idxCnt, trxNum;
	byte settlementAcq, currAcq;
	byte temp;
	word mnuItem;
	int status;

	MAPGETWORD(regTrxNo, trxNum, lblKO);

	do
	{
		status=0;
    	MAPGETWORD(traIdx, idxCnt, lblKO);
    	if(idxCnt <= trxNum)
    	{
    		MAPPUTBYTE(traTxnType, trtPrepaidSettlement, lblKO);
    		mapMove(logBeg, (word) idxCnt-1);
    		idxCnt++;
    		MAPPUTWORD(traIdx, idxCnt, lblKO);

            MAPGETBYTE(appAcqSettlementIdx,settlementAcq,lblKO);
            MAPGETBYTE(logAcqIdx,currAcq,lblKO);
            //periksa Acquirer
            if(settlementAcq != currAcq)
            {
                continue;
            }
            //periksa sudah diupload
            MAPGETBYTE(logIsUploaded,temp,lblKO);
            if(temp == 1)
            {
                continue;
            }

            MAPGETWORD(logMnuItm,mnuItem,lblKO);
            if((mnuItem != mnuPrepaidPayment)&& (mnuItem != mnuPrepaidPaymentDisc) )
            {
                continue;
            }
            status = idxCnt-1;
    		break;
    	}
    	else
    	{
    		break;
    	}

    }while(idxCnt <= trxNum);
	return status;
lblKO:
	return -1;
}

//-- @agmr

#ifdef DUMP_ISO_MESSAGE
static int dumpField(const byte * rsp, byte bit) {
//    int ret;
    int fmt;
    card len;                   // length of Data Element
    byte cnt;
    byte lenhex[5];             //mapp: to check
 //   byte emvRespCode[3];
    char buf[1024];

    VERIFY(rsp);
    fmt = isoFmt(bit);
    cnt = 0;

    if(fmt < 0) {               //LLVAR or LLLVAR
        switch (-fmt) {
          case 4:
          case 2:
              cnt = 1;
              break;
          case 5:
          case 3:
              cnt = 2;
              break;
          default:
              cnt = 0;
              break;
        }
        VERIFY(cnt);
        memset(lenhex, 0x00, sizeof(lenhex));
        bin2hex((char *) lenhex, rsp, cnt);
        dec2num(&len, (char *) lenhex, sizeof(lenhex));
        trcFN("Len LLLVAR=%d\n", len);

        //handling of LLVAR & LLLVAR
        switch (-fmt) {
          case 2:
          case 3:
              len = (len + 1) / 2;
              break;
          default:
              break;
        }

    }
    else
    {
        if(fmt % 2 != 0)
            fmt++;

        len = fmt / 2;
    }

	memcpy(buf,"\x1B\x1A",2);
	sprintf(buf+2,"FIELD %i Len %i :",bit, (int)len+cnt);
	prtS(buf);

	ShowData((byte*)rsp,len+cnt,0, 1, 15);
    rsp += cnt;
    return cnt + len;
}

#define MOV(N) if(len<N) return -1; len-= N; rsp+= N

int dumpIsoMessage(const byte * rsp, word len)
{
    int ret;
    byte BitMap[8];
    byte bit;
    byte buf[50];
    byte res[5];

	MAPGETBYTE(appDumpIsoMessageFlag,res,lblDBA);
	if(res[0] == 0)
		return 0;

    VERIFY(rsp);

    trcS("rspParse rsp:");
    trcBN(rsp, len);
    trcS("\n");

//	MOV(2); //move length

	sprintf(buf,"TPDU: %02x %02x %02x %02x %02x",*(rsp),*(rsp+1),*(rsp+2),*(rsp+3),*(rsp+4));
	prtS(buf);
//	ShowData(rsp,5,0);
    MOV(5);                     //move TPDU

    sprintf(buf,"MSG TYPE:%02x%02x LEN:%i",*(rsp),*(rsp+1), len);
    prtS(buf);

    MOV(2);						//move Message type

    memset(BitMap, 0, 8);
    memcpy(BitMap, rsp, 8);
    trcS("bmp:");
    trcBN(BitMap, 8);
    trcS("\n");
    MOV(8);                     //move bitmap

    for (bit = 2; bit <= 64; bit++) {
        if(!bitTest(BitMap, bit))
            continue;
        ret = dumpField(rsp, bit);
        CHK;
        MOV(ret);
    }

    prtS("\n\n");

    trcFN("rspParse: ret=%d\n", len);
    return len;                 //return the length of the non-parsed tail; non-negative is OK; normally 1 (' ')
/*
    lblKO:
    return -1;
*/
  lblDBA:
    usrInfo(infDataAccessError);
    return -1;
}
#endif //DUMP_ISO_MESSAGE
