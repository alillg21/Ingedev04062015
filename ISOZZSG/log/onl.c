//Subversion reference
//$HeadURL: https://svn.ingenico.com/UTAPROJECTS/sea-partner/5W/trunk/ISO5WPH/log/onl.c $
//$Id: onl.c 2047 2009-05-22 03:58:55Z ajloreto $

#include "string.h"
#include "log.h"
#include "bri.h"

static struct
{
	char chn[lenChn + 1];       //chn[0]: 'M': modem; 'T': TCP/IP; 'G': GPRS; 'R': RS232 ...
	char srv;                   //'A': authorization; 'S': settlement; 'P': parameters download(TMS)
}
ctx;

#define CHK CHECK(ret>=0,lblKO)

static int onlGetDialSrv(char *srv, word key)
{
	int ret;
	char Pabx[lenPabx + 1];
	char Prefix[lenPrefix + 1];
	char newsrv[lenSrv + 1];
	byte dialOpt = 0;

	switch (ctx.srv)
	{
		case 'A':
			ret = mapGetByte(acqTxnDialOpt, dialOpt);
			CHECK(ret >= 0, lblKO);
			memcpy(&ctx.chn[3], (dialOpt & 0x02) ? "2400" : "1200", 4);
			break;
		case 'S':
			ret = mapGetByte(acqStlDialOpt, dialOpt);
			CHECK(ret >= 0, lblKO);
			memcpy(&ctx.chn[3], (dialOpt & 0x02) ? "2400" : "1200", 4);
			break;
		default:
			VERIFY(ctx.srv == 'P');
			break;
	}

	memset(newsrv, 0x00, sizeof(newsrv));

	ret = mapGet(appPabx, Pabx, lenPabx + 1);
	CHECK(ret >= 0, lblKO);

	ret = mapGet(appPrefix, Prefix, lenPrefix + 1);
	CHECK(ret >= 0, lblKO);

	if(*Pabx)
	{
		strcat(newsrv, Pabx);
		strcat(newsrv, "-");
	}
	if(*Prefix)
	{
		strcat(newsrv, Prefix);
	}
	strcat(newsrv, srv);
	memset(srv, 0x00, lenSrv + 1);
	memcpy(srv, newsrv, sizeof(newsrv));
	return strlen(newsrv);
lblKO:
	return -1;
}

static int onlDial(const char *chn, char *srv)
{
	int ret = 0;
	word inf, key;
	char GprsApn[lenApn + 1];
	byte comNum = 0;

	VERIFY(chn);
	VERIFY(srv);

	trcFS("srv: %s\n", srv);
	trcFS("chn: %s\n", chn);
	switch (chn[0])
	{
		case 'M':
			ret = comStart(chnHdlc);
			CHECK(ret >= 0, lblKO);
			break;
		case 'T':
			ret = comStart(chnTcp5100);
			CHECK(ret >= 0, lblKO);
			break;
		case 'R':
			MAPGETBYTE(appComType, comNum, lblKO);
			switch (comNum)
			{
				case 1:
					key = chnCom1;
					break;
				case 2:
					key = chnUSB;//chnCom2;
					break;
				default:
					key = 0;
					break;
			}
			ret = comStart(key);
			CHECK(ret >= 0, lblKO);
			break;
		case 'G':
			ret = comStart(chnGprs);
			CHECK(ret >= 0, lblKO);
			break;
	}
	switch (chn[0])
	{
//	    case 'T':
		case 'M':
#ifdef __UNICAPT__

			ret = comSet(&chn[1]);
#else

			ret = comSet(srv);
#endif

			break;
		case 'T':
		case 'R':
			MAPGETBYTE(appComType, comNum, lblKO);
			switch (comNum)
			{
				case 1:
					ret = comSet(&chn[1]);
					break;
				case 2:
					ret = comSet("");
//					key = chnUSB;//chnCom2;
					break;
				default:
					key = 0;
					break;
			}
			break;
		case 'G':
			MAPGET(appGprsApn, GprsApn, lblKO);
			ret = comSet(GprsApn);
			break;
	}

	CHECK(ret >= 0, lblKO);

	switch (chn[0])
	{
		case 'M':
		case 'G':
		case 'T':
			ret = comDial(srv);
			break;
		case 'R':
#ifdef __APR__

			ret = comDial(srv);
#else

			ret = comDial("");
#endif

			break;
	}

	if(ret < 0)
	{
		switch (-ret)
		{
			case 0:
				inf = infConnConnected;
				break;
			case comBusy:
				inf = infConnBusy;
				break;
			case comNoDialTone:
				inf = infConnNoDialTone;
				break;
			case comNoCarrier:
				inf = infConnNoCarrier;
				break;
			case comTimeout:
				inf = infConnTimeout;
				break;
			default:
				inf = infBlank;
				break;
		}
		usrInfo(inf);
		CHECK(ret >= 0, lblKO);
	}

	ret = 1;
	goto lblEnd;
lblKO:
	//Display error message
	comStop();
	ret = -1;
lblEnd:
	tmrStop(0);
	tmrStop(3);

	return ret;
}

static int onlProcessConnect(word key)
{
	int ret;
	byte idx, att;
	char srv[lenSrv + 1];

	ret = mapGetByte(traTxnType, idx);
	CHECK(ret >= 0, lblKO);
	ret = mapGet(key, srv, lenSrv + 1);
	CHECK(ret >= 0, lblKO);
	trcFS("onlProcessConnect srv=%s\n", srv);

	switch (ctx.chn[0])
	{
		case 'M':                //modem
			ret = onlGetDialSrv(srv, key);
			CHECK(ret >= 0, lblKO);
			break;
		case 'R':                //RS232
		case 'T':                //TCP/IP
		case 'G':                //GPRS
		default:
			break;
	}
	trcFS("srv: %s\n", srv);
	trcFS("chn: %s\n", ctx.chn);

	key++;                      //now it is the key corresponding to the number of attempts
	ret = mapGetByte(key, att);
	CHECK(ret >= 0, lblKO);
	
//++ @agmr - auto logon pada saat power on, connect cukup 1 kali saja	
    {
        byte firstPowerOn;
        byte txnType;
        
        MAPGETBYTE(appIsFirstPowerOn,firstPowerOn,lblKO);
        MAPGETBYTE(traTxnType, txnType,lblKO);

        if((firstPowerOn == 1) && (txnType == trtDebitLogon))
            att = 1;
    }
//-- @agmr	
	
	for (idx = 0; idx < att; idx++)
	{
		ret = onlDial(ctx.chn, srv);
		if(ret >= 0)
		{
			return ret;
		}
		usrInfo(infConnFail);
		if(idx < (att - 1))
		{
			if(ctx.chn[0] == 'M')
				usrInfo(infRedialing);
			else
				usrInfo(infReConnecting);
		}
	}
	CHECK(ret >= 0, lblKO);
	return ret;
lblKO:
	return -1;
	//lblEnd:
	//return ret;
}

static int onlConnect(void)
{
	int ret;
	word key;
	byte num;
//++ @agmr	
	char srv1[lenSrv + 1];
	char srv2[lenSrv + 1];
//-- @agmr
	key = 0;
	switch (ctx.srv)
	{
		case 'A':    
			key = acqPriTxnSrv;
			break;
		case 'S':		    
			key = acqPriStlSrv;
			break;
		default:	    
			VERIFY(ctx.srv == 'P');
			key = appPriSrv;
			break;
	}
	for (num = 0; num < 2; num++)
	{  
		ret = onlProcessConnect(key);
		if(ret >= 0)
			break;			
//++ @agmr - primary dan secondary sama maka keluar
        memset(srv1,0,sizeof(srv1));
	    ret = mapGet(key, srv1, lenSrv + 1);
	    CHECK(ret >= 0, lblKO);
	    memset(srv2,0,sizeof(srv2));
	    ret = mapGet(key+2, srv2, lenSrv + 1);
	    CHECK(ret >= 0, lblKO);	 
	    if(memcmp(srv1,srv2,lenSrv) == 0)
	    {
	    	ret = -1;
	        break;
	    }
//++ @agmr 
		key += 2;
	}
	CHECK(ret >= 0, lblKO); 
	goto lblEnd;
lblKO:
	return -1;
lblEnd:
	return ret;
}

void DisconnectFromHost()
{
	
	if(ctx.chn[0] != 'R'){
		comHangStart();
		comHangWait();
	}

	  if(ctx.chn[0] != 'G')
	 	comStop();
}

int onlSession(void)
{
	int ret;
	word key, inf = 0;
	char rspCod[2+1];       
   	byte TcFlag;

	trcS("onlSession: Beg\n");
	dspClear();

	MAPGETBYTE(traTcFlag, TcFlag, lblKO);

	ret = mapGet(appChn, ctx.chn, sizeof(ctx.chn));
	CHECK(ret >= 0, lblKO);

	if(ctx.chn[0] == 'M')
		inf = infDialing;
	else
		inf = infConnecting;

	ret = usrInfo(inf);
	CHECK(ret >= 0, lblKO);

	ret = mapGetWord(traMnuItm, key);
	CHECK(ret >= 0, lblKO);
	switch (key)
	{
		case mnuSettlement:
			ctx.srv = 'S';
			break;
		case mnuTMSLP:
			ctx.srv = 'P';
			break;
		case mnuReversal:
			ctx.srv = 'R';
			break;
		case mnuSaleRedeem:
		case mnuSale:
			if(TcFlag == 1)
				ctx.srv = 'U';	
			else
 		 		ctx.srv = 'A';
	  		break;									
		case mnuPreaut:
		case mnuAut: //@agmr - BRI5
		case mnuPreCancel:
		case mnuPreAuth:
		case mnuPreCom:
		case mnuRefund:
		case mnuVoid:
		default:
			ctx.srv = 'A';
			break;
	}
	VERIFY(key);
	ret = onlConnect();
	CHECK(ret >= 0, lblKO);
	ret = tleSession(ctx.srv);
	CHECK(ret >= 0, lblKO);
	if(ctx.chn[0] != 'R')
	{
//@agmr - masih ada kemungkinan kirim autoreversal bila emv decline nantinya
//      - tidak di disconnect dulu	    
        if(emvFullProcessCheck()!= 1) 
        {    
		    ret = comHangStart();
		    CHECK(ret >= 0, lblKO);
		    ret = comHangWait();
		    CHECK(ret >= 0, lblKO);
		}
	}
//#ifdef PREPAID //@agmr - dibuka jika memang dianggap settlement tidak periksa field 39
//    if(key == mnuPrepaidSettlement)
//    {
//        word trxNum,idxCnt;
//        
//	    MAPGETWORD(regTrxNo, trxNum, lblKO);
//        MAPGETWORD(traIdx, idxCnt, lblKO);    	
//    	if((idxCnt) > trxNum)
//    	{
//    	    usrInfo(infCompleted);
//    	    ret = 1;
//    	    goto lblEnd;
//    	}
//    }
//#endif

//	ret = usrInfo(infOnlineResult);
	ret = displayResponse();
	if(ret < 0)
	{
//       if(ret <= 0){
		ret = -1;
		goto lblEnd;
	}

//++ @agmr - BRI	
	else if(ret == 9999) //ret = 9999 ada data request lagi
	{
	    goto lblEndWithoutDc;
	}
//-- @agmr - BRI		   	
	
	switch (key)
	{
#ifdef PREPAID
        case mnuPrepaidSettlement:
            MAPGET(traRspCod, rspCod, lblKO);
			if(strcmp(rspCod, "00") == 0)
			       usrInfo(infCompleted);
			else{
				ret = -100;
				goto lblKO;
			}
#endif	    
		case mnuSettlement:
			MAPGET(traRspCod, rspCod, lblKO);
			if(strcmp(rspCod, "00") == 0)
			       usrInfo(infCompleted);
			else{
				ret = -5;
				goto lblKO;
			}
			break;
		case mnuTMSLP:
			usrInfo(infCompleted);
			break;
		case mnuReversal:
		case mnuSale: 		
		case mnuSaleRedeem:
		case mnuPreaut:
		case mnuAut: //@agmr - BRI5
		case mnuPreAuth:
		case mnuPreCom:
		case mnuPreCancel:
		case mnuRefund:
		case mnuVoid:
		default:
			break;
	}
	ret = 1;
	goto lblEnd;
lblKO:
	if(ret == -2)                                    
        usrInfo(infDeclinedEMV);
    else if (ret==-3)
    	{
    	    usrInfo(infConnTimeout);
    		doBRIReversal();
    		ret = -100; //BRI3 - sudah menampilkan error
    	}			
    else if(ret != -100) //BRI3			
    {    
		usrInfo(infConnFail);
	    ret = -100; //BUKAREVERSAL
	}//BRI3	
	goto lblEnd;
lblEnd:
//++ @agmr - masih ada kemungkinan kirim autoreversal bila emv decline nantinya
//         - tidak di disconnect dulu
    if(emvFullProcessCheck() == 1 && ret == 1)
    {
        goto lblEndWithoutDc;
    }
//-- @agmr    
	DisconnectFromHost();
lblEndWithoutDc:	 // @agmr - BRI	
	trcFN("onlSession: ret=%d\n", ret);
	return ret;
}

int tleSecond()
{
	int ret;

	if(ctx.chn[0] == 'R' || ctx.chn[0] == 'T'){

		ret = onlSession();
		CHECK(ret >= 0, lblKO);
		
	} else {	
	
		ret = tleSession('U');
	    	CHECK(ret >= 0, lblKO);

		DisconnectFromHost();
		
	}	
	return ret;
	lblKO:
		DisconnectFromHost();
		return -1;
	
}

//++ @agmr 
int onlConnect2()
{
    return onlConnect();
}
//
