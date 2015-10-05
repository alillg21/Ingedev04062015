//$HeadURL: https://svn.ingenico.com/UTAPROJECTS/sea-partner/5W/trunk/ISO5WPH/log/rsp.c $
//$Id: rsp.c 1917 2009-04-22 13:24:29Z abarantsev $

#include "string.h"
#include "stdio.h"
#include "log.h"
#include "iso.h"
#include "emv.h"
#include "str.h"
#include "BRI.h"

#define CHK CHECK(ret>=0,lblKO)
#define MOV(N) if(len<N) return -1; len-= N; rsp+= N
#define PARSE(F) ret = F(rsp, length); CHK; MOV(ret);
#ifndef MIN
#define MIN(x,y)  ( ((x) < (y)) ? (x) : (y) )
#endif

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

    setBeg,                     //engine states: Settlement
    setSendReq,
    setRecvRsp,
    setBatSendReq,
    setBatRecvRsp,
    setEnd
};

//TMS EMV Tags
#define EMV_KEY_EXPONENT 0xDFE91B
#define EMV_KEY_ALGORITHIM 0xDFE91C
#define EMV_DEFAULT_PARAM 0xFFE913
#define EMV_DEF_APPROVAL_LEVEL 0xDFE91D
#define EMV_DEF_TAGS 0xFFE91E
#define EMV_KEY 0xFFE911
#define EMV_KEY_RID 0xDFE918
#define EMV_KEY_INDEX 0xDFE919
#define EMV_KEY_DATA 0xDFE91A
#define EMV_AID_PARAM 0xFFE910
#define EMV_AID 0xDFE914
#define EMV_AID_PRIORITY 0xDFE91F
#define EMV_AID_APP_SEL_INDIC 0xDFE920
#define EMV_AID_TAGS 0xFFE915
#define EMV_TXN_TYPE 0xDFE916
#define EMV_AID_TXN_TAGS 0xFFE917

#define tmsTACDft 0x1f03
#define tmsTACDen 0x1f04
#define tmsTACOnl 0x1f05
#define tmsThrVal 0x1f07
#define tmsTarPer 0x1f08
#define tmsMaxTarPer 0x1f09
#define tmsDftValDDOL 0x1f15
#define tmsDftValTDOL 0x1f18

#define mask8 ((byte)0x80)
#define mask54321 ((byte)0x1F)
#define mask7654321 ((byte)0x7F)

static byte emvFile[65535];
static int emvFileIdx = 0;

static byte aidRecNum;
static byte pubRecNum;

static int ConvIpPrtSrv(const byte * rsp, char *buffer, word len)
{
	char 	tmpbuf[10];
	card 	val;                  
	byte 	cnt = 0;
	int 		ret = 0;

	memset(tmpbuf, 0x00, sizeof(tmpbuf));
	for (cnt = 0; cnt < 4; cnt++)   
	{
		val = (byte) * rsp;
		num2dec(tmpbuf, val, 3);
		strcat(buffer, tmpbuf);
		if(cnt < 3)
			strcat(buffer, ".");
		memset(tmpbuf, 0x00, sizeof(tmpbuf));
		MOV(1);
	}
	ret = cnt;                  
	strcat(buffer, "|");
	ret = ret + bin2num(&val, rsp, 2); 
	num2dec(tmpbuf, val, 4);
	strcat(buffer, tmpbuf);
	MOV(2);

	return ret;
}

static int putVal(const byte * val, word key, word len)
{
	int ret;

	VERIFY(val);
	ret = mapPut(key, val, len);
	CHK;
	return ret;
	
	lblKO:
		return -1;
}

static int emvGetT(byte * tag, const byte * src)
{ 
	byte ret;

	*tag = *src;
	ret = 1;
	if(((*tag) & mask54321) != mask54321)
	return 1;             
	do
	{                       
		ret++;
		tag++;
		src++;
		*tag = *src;
	}
	while((*tag) & mask8);
	VERIFY(ret <= 2);           
	return ret;
}

static int emvGetL(card * len, const byte * src)
{  
	byte ret;

	if(((*src) & mask8) != mask8)
	{ 
		*len = *src;
		ret = 1;
		goto lblOK;
	}
	ret = (*src) & mask7654321;
	src++;
	*len = *src++;
	ret--;

	if(ret == 0)
	{              
		ret = 2;
		goto lblOK;
	}
	*len *= 0x100;
	*len += *src++;
	ret--;

	if(ret == 0)
	{              
		ret = 3;
		goto lblOK;
	}
	*len *= 0x100;
	*len += *src++;
	ret--;

	if(ret == 0)
	{              
		ret = 4;
		goto lblOK;
	}
	*len *= 0x100;
	*len += *src++;
	ret--;
	
	if(ret == 0)
	{              
		ret = 5;
		goto lblOK;
	}

	return -1;                  
	lblOK:
		return ret;
}

static word mapKeyTmsTag(const byte * rsp, word len)
{  
	card respDta;

	bin2num(&respDta, rsp, len);

	switch (respDta)
	{
		case tmsTACDft:
			return emvTACDft;
		case tmsTACDen:
			return emvTACDen;
		case tmsTACOnl:
			return emvTACOnl;
		case tmsThrVal:
			return emvThrVal;
		case tmsTarPer:
			return emvTarPer;
		case tmsMaxTarPer:
			return emvMaxTarPer;
		case tmsDftValDDOL:
			return emvDftValDDOL;
		case tmsDftValTDOL:
			return emvDftValTDOL;
		case tagTrnTyp:
			return emvTrnTyp;
		case tagTrmAvn:
			return emvTrmAvn;
		case tagTrmFlrLim:
			return emvTrmFlrLim;
		case tagTCC:
			return emvTCC;
		default:
			return 0;
	}
}

static int parseTerDta(const byte * rsp, word lenDta)
{ 
	int 		ret = 0, val = 0;
	card 	len = 0;
	byte 	tag[2];
	byte 	tarPer[2 + 1];
	card 	respDta;
	word 	keyTag;

	while(lenDta > 0)
	{
		memset(tag, 0, sizeof tag);
		ret = emvGetT(tag, rsp);
		CHK;

		trcS("Tag:\n");
		trcBN(tag, ret);
		trcS("\n");
		rsp += ret;
		lenDta -= ret;

		len = 0;
		val = 0;
		val = emvGetL(&len, rsp);
		CHECK(val >= 0, lblKO);

		trcS("Parsed Data:\n");
		trcBN(rsp, len + val);
		trcS("\n");

		bin2num(&respDta, tag, ret);

		switch (respDta)
		{
			case tmsTarPer:      
			case tmsMaxTarPer:
				if(ret == 2)
				{
					memcpy(tarPer, "\x01", 1);
					rsp += 2;
					memcpy(&tarPer[1], rsp, 1);
					keyTag = mapKeyTmsTag(tag, ret);
					ret = mapPut(keyTag, tarPer, 2);
					rsp += 1;
				}
				else
				{
					keyTag = mapKeyTmsTag(tag, ret);
					ret = mapPut(keyTag, rsp, len + val);
					rsp += (len + val);
				}
				break;
				
			case tagTrmAvn:
				if(len != 0)
					ret = mapPut(mapKeyTmsTag(tag, ret), rsp, len + val);
				rsp += (len + val);
				break;
				
			default:
				ret = mapPut(mapKeyTmsTag(tag, ret), rsp, len + val);
				rsp += (len + val);
				break;
		}
		CHK;

		lenDta -= (len + val);
	}

	return ret;
	lblKO:
		return -1;
}


static word saveTmsEmvDta(const byte * rsp, word lenDta, int tag) {
	int 		ret = 1;
	word 	retLen = lenDta;
	char 	buf[256];

	switch (tag) {              
		case tetEmvKey:
			trcS("tetEmvKey");
			ret = mapMove(pubBeg, pubRecNum);
			break;
			
		case tetEmvKeyRid:
			trcS("tetEmvKeyRid");
			retLen = lenDta + 1;
			ret = mapPut(pubRid, rsp, retLen);
			trcS("EMV_KEY_RID:\n");
			CHK;
			break;
			
		case tetEmvKeyIdx:
			trcS("tetEmvKeyIdx");
			CHECK(lenDta == 1, lblKO);
			ret = mapPutByte(pubIdx, *rsp);
			trcS("EMV_KEY_INDEX:\n");
			CHK;
			break;
			
		case tetEmvKeyDta:
			trcS("tetEmvKeyDta");
			retLen = lenDta + 1;
			ret = mapPut(pubkeyData, rsp, retLen);
			trcS("EMV_KEY_DATA:\n");
			CHK;
			break;
			
		case tetEmvKeyExp:
			trcS("tetEmvKeyExp");
			CHECK(lenDta == 1, lblKO);
			ret = mapPutByte(pubExp, *rsp);
			trcS("EMV_KEY_EXPONENT:\n");
			CHK;
			break;
			
		case tetEmvKeyAlg:
			trcS("tetEmvKeyAlg");
			CHECK(lenDta == 1, lblKO);
			ret = mapPutByte(pubAlgo, *rsp);
			CHK;
			pubRecNum += 1;
			ret = mapPutByte(regPubCnt, pubRecNum);
			CHK;
			trcS("EMV_KEY_ALGORITHIM:\n");
			break;

		case tetEmvDefPar:
		case tetEmvDefAprLvl:    
		case tetEmvDefTag:       
			break;

		case tetEmvAidPar:
			trcS("tetEmvAidPar");
			ret = mapMove(aidBeg, aidRecNum);
			break;
			case tetEmvAid:
			trcS("tetEmvAid");
			retLen = lenDta + 1;
			ret = mapPut(emvAid, rsp, retLen);
			trcS("EMV_AID:\n");
			CHK;
			break;
			
		case tetEmvAidPri:   
			break;
			
		case tetEmvAppSelIdx:   
			break;

		case tetEmvAidTag:       
			trcS("tetEmvAidTag");
			mapGet(binHi, buf, 1 + lenBinPrefix);
			ret = mapPut(emvAcqId, buf, 1 + lenBinPrefix);
			CHK;
			break;
			
		case tetEmvTxnType:
			trcS("tetEmvTxnType");
			ret = mapPut(emvAidTxnType, rsp, lenDta);
			trcS("EMV_TXN_TYPE:\n");
			CHK;
			break;
			
		case tetEmvAidTxnTag:
			trcS("tetEmvAidTxnTag");
			parseTerDta(rsp, lenDta);
			aidRecNum += 1;
			MAPPUTBYTE(regAidCnt, aidRecNum, lblKO);
			trcS("EMV_AID_TXN_TAGS:\n");
			break;
			
		default:
			break;
	}

	return retLen;
	lblKO:
		return 0;
}

static int parseTmsTag(const byte * rsp) { 
	int 	ret = 0;
	card respDta;

	bin2num(&respDta, rsp, 3);
	switch (respDta) {
		case EMV_KEY:
			ret = tetEmvKey;
			break;
			
		case EMV_KEY_RID:        
			ret = tetEmvKeyRid;
			break;
			
		case EMV_KEY_INDEX:      
			ret = tetEmvKeyIdx;
			break;
			
		case EMV_KEY_DATA:       
			ret = tetEmvKeyDta;
			break;
			
		case EMV_KEY_EXPONENT:   
			ret = tetEmvKeyExp;
			break;
			
		case EMV_KEY_ALGORITHIM: 
			ret = tetEmvKeyAlg;
			break;
			
		case EMV_DEFAULT_PARAM:
			ret = tetEmvDefPar;
			break;
			
		case EMV_DEF_APPROVAL_LEVEL:
			ret = tetEmvDefAprLvl;
			break;
			
		case EMV_DEF_TAGS:
			ret = tetEmvDefTag;
			break;
			
		case EMV_AID_PARAM:
			ret = tetEmvAidPar;
			break;
			
		case EMV_AID:           
			ret = tetEmvAid;
			break;
			
		case EMV_AID_PRIORITY:  
			ret = tetEmvAidPri;
			break;
			
		case EMV_AID_APP_SEL_INDIC:  
			ret = tetEmvAppSelIdx;
			break;
			
		case EMV_AID_TAGS:
			ret = tetEmvAidTag;
			break;
			
		case EMV_TXN_TYPE:       
			ret = tetEmvTxnType;
			break;
			
		case EMV_AID_TXN_TAGS:
			ret = tetEmvAidTxnTag;
			break;
			
		default:
			ret = 0;
			break;
	}
	return ret;
}

static int parseEmvFile(void) {
	int 		ret, idx = 0,  tmp = 0, lenTmp = 0;;
	card 	lenDta = 0;

	while(idx  <  emvFileIdx) {
		lenTmp = 1;
		tmp = parseTmsTag(&emvFile[idx]);
		CHECK(tmp > 0, lblKO);
		idx += 3;                 

		if(memcmp(&emvFile[idx], "\x81", 1) == 0) {   
			idx += 1;                 
			bin2num(&lenDta, &emvFile[idx], 1);
		} else if (memcmp(&emvFile[idx], "\x82", 1) == 0) {   
			idx += 1;                
			bin2num(&lenDta, &emvFile[idx], 2);
			lenTmp = 2;
		} else {
			bin2num(&lenDta, &emvFile[idx], 1);
		}

		switch (tmp) {          
			case tetEmvKeyRid:
			case tetEmvAid:
			case tetEmvKeyDta:
				break;
				
			default:
				idx += lenTmp;           
				break;
		}

		lenDta = saveTmsEmvDta(&emvFile[idx], lenDta, tmp);
		CHECK(lenDta > 0, lblKO);

		switch (tmp) {         
			case tetEmvKey:
			case tetEmvAidPar:
			case tetEmvDefPar:
			case tetEmvAidTag:
				break;
				
			default:
				trcBN(&emvFile[idx], lenDta);
				trcS("\n");
				idx += lenDta;
				break;
		}
	}

	memset(emvFile, 0, sizeof(emvFile));
	emvFileIdx = 0;

	return ret;
	lblKO:
		return -1;
}

static int parseEmvFleRed(const byte * rsp, word len) {
	int 		ret = 0;
	card 	offset = 0;

	VERIFY(rsp);
	trcS("parseEmvFleRed:\n");
	trcBN(rsp, len);
	trcS("\n");

	CHECK(len <= lenTmsFleChkSze, lblKO);
	MAPGETCARD(traTmsEmvFleOff, offset, lblKO);
	offset += (card) len - 1;
	MAPPUTCARD(traTmsEmvFleOff, offset, lblKO);

	MAPGETBYTE(regAidCnt, aidRecNum, lblKO);
	CHECK(aidRecNum < dimAID, lblKO);
	MAPGETBYTE(regPubCnt, pubRecNum, lblKO);
	CHECK(pubRecNum < dimPub, lblKO);

	MOV(1);                     

	memcpy(&emvFile[emvFileIdx], rsp, len);
	emvFileIdx += len;

	return ret;
	lblKO:
		return -1;
}

static int parseEmvFleOpn(const byte * rsp, word len)
{
	int ret = 0;
	card fleSze = 0;

	VERIFY(rsp);
	trcS("parseEmvFleDnl:\n");
	trcBN(rsp, len);
	trcS("\n");

	MOV(1);                     
	MOV(2);                     

	trcS("File size:\n");
	trcBN(rsp, len);
	trcS("\n");
	ret = bin2num(&fleSze, rsp, len);
	CHK;
	MAPPUTCARD(traTmsEmvFleSze, fleSze, lblKO);
	MOV(len);
	CHECK(len == 0, lblKO);

	ret = mapReset(aidBeg);
	CHECK(ret > 0, lblKO);
	ret = mapReset(pubBeg);
	CHECK(ret > 0, lblKO);

	return ret;
	lblKO:
		return -1;
}

static int parseEmvFleDnl(const byte * rsp, word len)
{
	int ret = 0;

	VERIFY(rsp);
	trcS("parseEmvFleDnl:\n");
	trcBN(rsp, len);
	trcS("\n");

	MOV(1);                     

	trcS("File Handle:\n");
	trcBN(rsp, len);
	trcS("\n");
	ret = mapPut(traTmsEmvFleHdl, rsp, lenFleHdl);
	CHK;
	MOV(len);

	CHECK(len == 0, lblKO);

	return ret;
	lblKO:
		return -1;
}

static int parseEmvReq(const byte * rsp, word len)
{
	int 	ret = 0;

	VERIFY(rsp);
	trcS("parseEmvReq:\n");
	trcBN(rsp, len);
	trcS("\n");

	MOV(2);                     

	trcS("File Name:\n");
	trcBN(rsp, len);
	trcS("\n");
	if(*rsp == 0)               
		return -80;             

	ret = mapPut(traTmsEmvFleNme, rsp, len);
	CHK;
	MOV(len);

	CHECK(len == 0, lblKO);

	return ret;
	lblKO:
		return -1;
}

static int saveParseByte(const byte * rsp, word key, const char *trc)
{
	int 	ret;

	ret = sizeof(byte);
	trcFS("saveParseByte %s: ", trc);
	trcBN(rsp, ret);
	trcS("\n");
	MAPPUTBYTE(key, *rsp, lblKO);
	return sizeof(byte);

	lblKO:
		return -1;
}

static int saveParseWord(const card * rsp, word key, const char *trc)
{
	int 	ret;

	ret = sizeof(word);
	trcFS("saveParseWord %s: ", trc);
	trcFN("%d", *rsp);
	trcS("\n");
	MAPPUTWORD(key, *rsp, lblKO);
	return sizeof(word);

	lblKO:
		return -1;
}
static int saveParseStr(const byte * rsp, word key, word n, const char *trc)
{
	int 	ret;

	trcFS("saveParseStr %s: ", trc);
	trcBN(rsp, n);
	trcS("\n");
	ret = mapPut(key, rsp, n);
	CHK;
	return n;
	
	lblKO:
		return -1;
}
static int parseSkip(const byte * rsp, word n, const char *trc)
{
	trcFS("parseSkip  %s: ", trc);
	trcBN(rsp, n);
	trcS("\n");
	return n;
}

static int parsePos(const byte * rsp, word length)
{
	return parseSkip(rsp, sizeof(byte), "parsePos");
}

static int parseLength(const byte * rsp, word length)
{
	return parseSkip(rsp, sizeof(word), "parseLength");
}

static int parseAcqId(const byte * rsp, word length)
{
	return saveParseByte(rsp, acqID, "parseAcqId");
}

static int parseAcqName(const byte * rsp, word length)
{
	return saveParseStr(rsp, acqName, lenName, "parseAcqName");
}

static int parseTrans_Device(const byte * rsp, word length)
{
	return parseSkip(rsp, sizeof(byte), "parseTrans_Device");
}

static int parseSettle_Device(const byte * rsp, word length)
{
	return parseSkip(rsp, sizeof(byte), "parseSettle_Device");
}

static int parseAcqPriTxnPhnSrv(const byte * rsp, word length)
{

	return saveParseStr(rsp, acqPriTxnPhnSrv, lenPhone, "parseAcqPriTxnPhnSrv");

}

static int parseAcqPriStlPhnSrv(const byte * rsp, word length)
{
	return saveParseStr(rsp, acqPriStlPhnSrv, lenPhone, "parseAcqPriStlPhnSrv");
}

static int parseAcqPriTxnSrv(const byte * rsp, word length)
{
	return saveParseStr(rsp, acqPriTxnSrv, length, "parseAcqPriTxnSrv");

}

static int parseAcqPriStlSrv(const byte * rsp, word length)
{
	return saveParseStr(rsp, acqPriStlSrv, length, "parseAcqPriStlSrv");
}

static int parseAcqPriTxnAtt(const byte * rsp, word length)
{
	return saveParseByte(rsp, acqPriTxnAtt, "parseacqPriTxnAtt");
}

static int parseAcqPriStlAtt(const byte * rsp, word length)
{
	return saveParseByte(rsp, acqPriStlAtt, "parseacqPriStlAtt");
}

static int parseAcqSecTxnPhnSrv(const byte * rsp, word length)
{
	return saveParseStr(rsp, acqSecTxnPhnSrv, lenPhone, "parseAcqSecTxnPhnSrv");
}

static int parseAcqSecStlPhnSrv(const byte * rsp, word length)
{
	return saveParseStr(rsp, acqSecStlPhnSrv, lenPhone, "parseAcqSecStlPhnSrv");
}

static int parseAcqSecTxnSrv(const byte * rsp, word length)
{
	return saveParseStr(rsp, acqSecTxnSrv, length, "parseAcqSecTxnSrv");
}

static int parseAcqSecStlSrv(const byte * rsp, word length)
{
	return saveParseStr(rsp, acqSecStlSrv, length, "parseAcqSecStlSrv");
}

static int parseAcqSecTxnAtt(const byte * rsp, word length)
{
	return saveParseByte(rsp, acqSecTxnAtt, "parseacqSecTxnAtt");
}

static int parseAcqSecStlAtt(const byte * rsp, word length)
{
	return saveParseByte(rsp, acqSecStlAtt, "parseacqSecStlAtt");
}

static int parseAcqTxnMdmMod(const byte * rsp, word length)
{
	return parseSkip(rsp, sizeof(byte), "parseAcqTxnMdmMod");
}

static int parseAcqStlMdmMod(const byte * rsp, word length)
{
	return parseSkip(rsp, sizeof(byte), "parseAcqStlMdmMod");
}

static int parseAcqTxnProt(const byte * rsp, word length)
{
	return parseSkip(rsp, sizeof(byte), "parseAcqStlProt");
}

static int parseAcqStlProt(const byte * rsp, word length)
{
	return parseSkip(rsp, sizeof(byte), "parseAcqStlProt");
}

static int parseAcqTxnAddOfLargeRxBuffer(const byte * rsp, word length)
{
	return parseSkip(rsp, sizeof(word), "parseAcqTxnAddOfLargeRxBuffer");
}

static int parseAcqStlAddOfLargeRxBuffer(const byte * rsp, word length)
{
	return parseSkip(rsp, sizeof(word), "parseAcqStlAddOfLargeRxBuffer");
}

static int parseAcqTxnAddOfLargeTxBuffer(const byte * rsp, word length)
{
	return parseSkip(rsp, sizeof(word), "parseAcqTxnAddOfLargeTxBuffer");
}

static int parseAcqStlAddOfLargeTxBuffer(const byte * rsp, word length)
{
	return parseSkip(rsp, sizeof(word), "parseAcqStlAddOfLargeTxBuffer");
}

static int parseAcqTxnRxBufferSize(const byte * rsp, word length)
{
	return parseSkip(rsp, sizeof(word), "parseAcqTxnRxBufferSize");
}

static int parseAcqStlRxBufferSize(const byte * rsp, word length)
{
	return parseSkip(rsp, sizeof(word), "parseAcqStlRxBufferSize");
}

static int parseAcqTxnTxBufferSize(const byte * rsp, word length)
{
	return parseSkip(rsp, sizeof(word), "parseAcqTxnTxBufferSize");
}

static int parseAcqStlTxBufferSize(const byte * rsp, word length)
{
	return parseSkip(rsp, sizeof(word), "parseAcqStlTxBufferSize");
}

static int parseAcqTxnDialOpt(const byte * rsp, word length)
{
	return saveParseByte(rsp, acqTxnDialOpt, "parseAcqTxnDialOpt");
}

static int parseAcqStlDialOpt(const byte * rsp, word length)
{
	return saveParseByte(rsp, acqStlDialOpt, "parseAcqStlDialOpt");
}

static int parseTxnReserved(const byte * rsp, word length)
{
	return parseSkip(rsp, 7, "parseTxnReserved");
}

static int parseStlReserved(const byte * rsp, word length)
{
	return parseSkip(rsp, 7, "parseStlReserved");
}

static int parseAcqTxnRxTO(const byte * rsp, word length)
{
	card 	val;

	bin2num(&val, rsp, 2);
	return saveParseWord(&val, acqTxnRxTO, "parseAcqTxnRxTO");
}

static int parseAcqStlRxTO(const byte * rsp, word length)
{
	card 	val;

	bin2num(&val, rsp, 2);
	return saveParseWord(&val, acqStlRxTO, "parseAcqStlRxTO");
}

static int parseAcqTxnICTO(const byte * rsp, word length)
{
	card 	val;

	bin2num(&val, rsp, 2);
	return saveParseWord(&val, acqTxnICTO, "parseAcqTxnICTO");
}

static int parseAcqStlICTO(const byte * rsp, word length)
{
	card 	val;

	bin2num(&val, rsp, 2);
	return saveParseWord(&val, acqStlICTO, "parseAcqStlICTO");
}

static int parseAcqTxnOpt(const byte * rsp, word length)
{
	return saveParseByte(rsp, acqTxnOpt, "parseAcqTxnOpt");
}

static int parseAcqStlOpt(const byte * rsp, word length)
{
	return saveParseByte(rsp, acqStlOpt, "parseAcqStlOpt");
}

static int parseAcqPriTxnIPSrv(const byte * rsp, word length)
{
	char 	buf[lenSrv + 1];
	int 		ret;

	memset(buf, 0, sizeof(buf));
	ret = ConvIpPrtSrv(rsp, buf, length);
	CHK;

	saveParseStr((byte *) buf, acqPriTxnIPSrv, lenSrv, "parseAcqPriTxnIPSrv");

	return ret;

	lblKO:
		return -1;
}

static int parseAcqPriStlIPSv(const byte * rsp, word length)
{
	char 	buf[lenSrv + 1];
	int 		ret;

	memset(buf, 0, sizeof(buf));
	ret = ConvIpPrtSrv(rsp, buf, length);
	CHK;

	saveParseStr((byte *) buf, acqPriStlIPSrv, lenSrv, "parseAcqPriStlIPSrv");

	return ret;

	lblKO:
		return -1;
}

static int parseAcqSecTxnIPSrv(const byte * rsp, word length)
{
	char 	buf[lenSrv + 1];
	int 		ret;

	memset(buf, 0, sizeof(buf));
	ret = ConvIpPrtSrv(rsp, buf, length);
	CHK;

	saveParseStr((byte *) buf, acqSecTxnIPSrv, lenSrv, "parseAcqSecTxnIPSrv");

	return ret;

	lblKO:
		return -1;
}

static int parseAcqSecStlIPSrv(const byte * rsp, word length)
{
	char 	buf[lenSrv + 1];
	int 		ret;

	memset(buf, 0, sizeof(buf));
	ret = ConvIpPrtSrv(rsp, buf, length);
	CHK;

	saveParseStr((byte *) buf, acqSecStlIPSrv, lenSrv, "parseAcqSecStlIPSrv");

	return ret;

	lblKO:
		return -1;
}

static int parseAcqTxnConTO(const byte * rsp, word length)
{
	card 	val;

	bin2num(&val, rsp, 2);

	return saveParseWord(&val, acqTxnConTO, "parseAcqTxnConTO");
}

static int parseAcqStlConTO(const byte * rsp, word length)
{
	card 	val;

	bin2num(&val, rsp, 2);

	return saveParseWord(&val, acqStlConTO, "parseAcqStlConTO");
}

static int parseAcqOpt1(const byte * rsp, word length)
{
	return parseSkip(rsp, sizeof(byte), "parseAcqOpt1");
}

static int parseAcqOpt2(const byte * rsp, word length)
{
	return saveParseByte(rsp, acqOpt2, "parseAcqOpt2");
}

static int parseAcqOpt3(const byte * rsp, word length)
{
	return parseSkip(rsp, sizeof(byte), "parseAcqOpt3");
}

static int parseAcqOpt4(const byte * rsp, word length)
{
	return parseSkip(rsp, sizeof(byte), "parseAcqOpt4");
}

static int parseAcqNii(const byte * rsp, word length)
{
	card 	numNii;
	char 	buf[lenNII + 1];
	byte 	nii[lenNII + 1];
	int 		ret;

	memset(nii, 0x00, sizeof(nii));
	memset(buf, 0x00, sizeof(buf));
	memcpy(nii, rsp, lenNii);
	bin2hex(buf, nii, lenNii);
	dec2num(&numNii, buf, 0);
	num2dec(buf, numNii, lenNII);

	ret = saveParseStr((byte *) buf, acqNII, lenNII, "parseAcqNii");
	CHK;
	ret = sizeof(word);         
	return ret;

	lblKO:
		return -1;
}

static int parseAcqTID(const byte * rsp, word length)
{
	return saveParseStr(rsp, acqTID, lenTID, "parseAcqTID");
}

static int parseAcqMID(const byte * rsp, word length)
{
	return saveParseStr(rsp, acqMID, lenMid, "parseAcqTID");
}

static int parseAcqCurBat(const byte * rsp, word length)
{
	char bat[lenInvNum + 1];    
	int ret;

	memset(bat, 0x00, sizeof(bat));
	bin2hex(bat, rsp, 3);

	//++ @agmr
	if(memcmp(bat,"000000",6)==0)
	memcpy(bat,"000001",6);
	//-- @agmr

	ret = saveParseStr((byte *) bat, acqCurBat, sizeof(bat) - 1,	 "parseAcqCurBat");
	CHK;
	ret = 3;                    
	return ret;

	lblKO:
		return -1;
}

static int parseAcqNxtBat(const byte * rsp, word length)
{
	return parseSkip(rsp, 3, "parseAcqNxtBat");
}

static int parseAcqStlTme(const byte * rsp, word length)
{
	return parseSkip(rsp, 6, "parseAcqStlTme");
}

static int parseAcqStlDte(const byte * rsp, word length)
{
	return parseSkip(rsp, sizeof(word), "parseAcqStlDte");
}

static int parseAcqEncPinKey(const byte * rsp, word length)
{   
	#ifdef SIMAS_TMS
	char 	buf[lenKEY_hex + 1];
	char AcqName[lenName + 1];
	int ret;

	memset(buf,0x00,sizeof(buf));
	bin2hex(buf, rsp, lenKEY*2);

	MAPGET(acqName, AcqName, lblKO);
	if (memcmp(AcqName,"ACQDUMMY",8)!=0) {
		mapPut((word)(regTMK),buf,lenKEY_hex);
	}
	else {
		mapPut((word)(regTAKLogon),buf,lenKEY_hex);
	}
lblKO:
	return parseSkip(rsp, lenKEY*2, "parseAcqEncPinKey");
#else
	return parseSkip(rsp, 8, "parseAcqEncPinKey");
#endif
}

static int parseAcqEncMacKey(const byte * rsp, word length)
{  
	#ifdef SIMAS_TMS
		return 0;
	#else
		return parseSkip(rsp, 8, "parseAcqEncMacKey");
	#endif
}

static int parseAcqDunno(const byte * rsp, word length)
{
	return parseSkip(rsp, 23, "parseAcqDunno");
}

static int parseAcqEMVRespCodeList(const byte * rsp, word length)
{
	return parseSkip(rsp, 18, "parseAcqEMVRespCodeList");
}

static int parseAcqTbl(const byte * rsp, word len)
{
	int 		ret = 0;
	char 	buf[lenSrv + 1];
	byte 	recNo;
	char 	chn[lenChn + 1];
	word 	length = 1;

#ifdef SIMAS_TMS
	char AcqName[lenName + 1];
#endif //SIMAS_TMS


	VERIFY(rsp);
	trcS("parseAcqTbl:\n");
	trcBN(rsp, len);
	trcS("\n");

	MAPGET(appChn, chn, lblKO);
	switch (chn[0])
	{
		case 'T': //@@AS0.0.20
			MAPPUTBYTE(appConnType, 2, lblKO);
			break;
			
		case 'G':
			MAPPUTBYTE(appConnType, 4, lblKO); //@@AS0.0.20
			break;
			
		case 'R':
			MAPPUTBYTE(appConnType, 3, lblKO);
			break;
			
		case 'M':
		default:
			MAPPUTBYTE(appConnType, 1, lblKO);
			break;
	}
	MAPGETBYTE(regAcqCnt, recNo, lblKO);
	CHECK(recNo < dimAcq, lblKO);

	if(recNo == 0)
	{           
		ret = mapReset(acqBeg); 
		CHECK(ret > 0, lblKO);
	}
	mapMove(acqBeg, recNo);

	PARSE(parseLength);
	PARSE(parsePos);
	PARSE(parseAcqId);
	PARSE(parseAcqName);
	PARSE(parseTrans_Device);
	PARSE(parseSettle_Device);

	if(chn[0] == 'M')
	{
		ret = parseAcqPriTxnSrv(rsp, lenPhone);
		CHK;
	}

	PARSE(parseAcqPriTxnPhnSrv);
	PARSE(parseAcqPriTxnAtt);

	if(chn[0] == 'M')
	{
		ret = parseAcqSecTxnSrv(rsp, lenPhone);
		CHK;
	}
	PARSE(parseAcqSecTxnPhnSrv);
	PARSE(parseAcqSecTxnAtt);

	PARSE(parseAcqTxnMdmMod);
	PARSE(parseAcqTxnProt);
	PARSE(parseAcqTxnAddOfLargeRxBuffer);
	PARSE(parseAcqTxnAddOfLargeTxBuffer);
	PARSE(parseAcqTxnRxBufferSize);
	PARSE(parseAcqTxnTxBufferSize);
	PARSE(parseAcqTxnDialOpt);
	PARSE(parseTxnReserved);
	PARSE(parseAcqTxnRxTO);
	PARSE(parseAcqTxnICTO);
	PARSE(parseAcqTxnOpt);

	if ((chn[0] == 'T')  || (chn[0] == 'G')) 
	{
		memset(buf, 0, sizeof(buf));
		ret = ConvIpPrtSrv(rsp, buf, len);
		CHK;

		ret = parseAcqPriTxnSrv((byte *) buf, lenSrv);
		CHK;
	}
	length = len;
	PARSE(parseAcqPriTxnIPSrv);

	if ((chn[0] == 'T')  || (chn[0] == 'G')) 
	{
		memset(buf, 0, sizeof(buf));
		ret = ConvIpPrtSrv(rsp, buf, len);
		CHK;

		ret = parseAcqSecTxnSrv((byte *) buf, lenSrv);
		CHK;
	}
	length = len;
	PARSE(parseAcqSecTxnIPSrv);
	PARSE(parseAcqTxnConTO);
	
	if(chn[0] == 'M')
	{
		ret = parseAcqPriStlSrv(rsp, lenPhone);
		CHK;
	}
	PARSE(parseAcqPriStlPhnSrv);
	PARSE(parseAcqPriStlAtt);

	if(chn[0] == 'M')
	{
		ret = parseAcqSecStlSrv(rsp, lenPhone);
		CHK;
	}
	PARSE(parseAcqSecStlPhnSrv);
	PARSE(parseAcqSecStlAtt);

	PARSE(parseAcqStlMdmMod);
	PARSE(parseAcqStlProt);
	PARSE(parseAcqStlAddOfLargeRxBuffer);
	PARSE(parseAcqStlAddOfLargeTxBuffer);
	PARSE(parseAcqStlRxBufferSize);
	PARSE(parseAcqStlTxBufferSize);
	PARSE(parseAcqStlDialOpt);
	PARSE(parseStlReserved);
	PARSE(parseAcqStlRxTO);
	PARSE(parseAcqStlICTO);
	PARSE(parseAcqStlOpt);

	if ((chn[0] == 'T')  || (chn[0] == 'G')) 
	{
		memset(buf, 0, sizeof(buf));
		ret = ConvIpPrtSrv(rsp, buf, len);
		CHK;

		ret = parseAcqPriStlSrv((byte *) buf, lenSrv);
		CHK;
	}
	PARSE(parseAcqPriStlIPSv);

	if ((chn[0] == 'T')  || (chn[0] == 'G')) 
	{
		memset(buf, 0, sizeof(buf));
		ret = ConvIpPrtSrv(rsp, buf, len);
		CHK;

		ret = parseAcqSecStlSrv((byte *) buf, lenSrv);
		CHK;
	}
	
	PARSE(parseAcqSecStlIPSrv);
	PARSE(parseAcqStlConTO);
	PARSE(parseAcqOpt1);
	PARSE(parseAcqOpt2);
	PARSE(parseAcqOpt3);
	PARSE(parseAcqOpt4);
	PARSE(parseAcqNii);
	PARSE(parseAcqTID);
	PARSE(parseAcqMID);
	PARSE(parseAcqCurBat);
	PARSE(parseAcqNxtBat);
	PARSE(parseAcqStlTme);
	PARSE(parseAcqStlDte);
	PARSE(parseAcqEncPinKey);
	PARSE(parseAcqEncMacKey);
	PARSE(parseAcqDunno);
	PARSE(parseAcqEMVRespCodeList);

	CHECK(len == 0, lblKO);

#ifdef SIMAS_TMS
	MAPGET(acqName, AcqName, lblKO);
	if (memcmp(AcqName,"ACQDUMMY",8)!=0) {
		recNo+=1;
	}
	else {
		memset(AcqName,0x00,sizeof(AcqName));
		mapPut((word)(acqName),AcqName,sizeof(acqName));
	}
#else //SIMAS_TMS
	recNo += 1;
#endif //SIMAS_TMS
	MAPPUTBYTE(regAcqCnt, recNo, lblKO);

	return ret;
	lblKO:
		return -1;
}

static int parseIssId(const byte * rsp, word length)
{
	return saveParseByte(rsp, issID, "parseIssId");
}

static int parseIssRefNum(const byte * rsp, word length)
{
	char 	buf[lenIssRef + 1];
	int 		ret, cnt = 0;

	memset(buf, 0x00, sizeof(buf));
	bin2hex(buf, rsp, 12);

	for (cnt = 0; cnt < lenIssRef; cnt++)
	{
		if(buf[cnt] == 'F')
			buf[cnt] = 0;
	}
	ret = saveParseStr((byte *) buf, issRef, lenIssRef, "parseIssRefNum");
	CHK;
	ret = 12;                   
	return ret;

	lblKO:
		return -1;
}

static int parseIssOpt1(const byte * rsp, word length)
{
	return saveParseByte(rsp, issOpt1, "parseIssOpt1");
}

static int parseIssOpt2(const byte * rsp, word length)
{
	return saveParseByte(rsp, issOpt2, "parseIssOpt2");
}

static int parseIssOpt3(const byte * rsp, word length)
{
	return saveParseByte(rsp, issOpt3, "parseIssOpt3");
}

static int parseIssOpt4(const byte * rsp, word length)
{
	return saveParseByte(rsp, issOpt4, "parseIssOpt4");
}

static int parseIssDefAcct(const byte * rsp, word length)
{
	return saveParseByte(rsp, issDefAcct, "parseIssDefAcct");
}

static int parseIssFlrLimit(const byte * rsp, word length)
{
	card 	flrlim;
	char 	buffer[5];
	int 		ret;

	memset(buffer, 0x00, sizeof(buffer));
	bin2hex(buffer, rsp, 2);
	dec2num(&flrlim, buffer, 4);
	ret = saveParseWord(&flrlim, issFloorLimit, "parseIssFlrLimit");
	CHK;
	ret = sizeof(word);         
	return ret;

	lblKO:
		return -1;
}

static int parseIssReAuthMargin(const byte * rsp, word length)
{
	return parseSkip(rsp, sizeof(byte), "parseIssReAuthMargin");
}

static int parseIssOpt5(const byte * rsp, word length)
{
	return saveParseByte(rsp, issOpt5, "parseIssOpt5");
}

static int parseIssPinOpt(const byte * rsp, word length)
{
	return saveParseByte(rsp, issPinOpt, "parseIssPinOpt");
}

static int parseIssuer(const byte * rsp, word * rsplen)
{
	int 		ret = 0;
	word 	len, length = 1;

	len = *rsplen;

	PARSE(parseLength);
	PARSE(parsePos);
	PARSE(parseIssId);
	PARSE(parseIssRefNum);
	PARSE(parseIssOpt1);
	PARSE(parseIssOpt2);
	PARSE(parseIssOpt3);
	PARSE(parseIssOpt4);
	PARSE(parseIssDefAcct);
	PARSE(parseIssFlrLimit);
	PARSE(parseIssReAuthMargin);
	PARSE(parseIssOpt5);
	PARSE(parseIssPinOpt);

	*rsplen = len;

	return ret;
	lblKO:
		return -1;
}

static int parseIssTbl(const byte * rsp, word len)
{
	int 		ret = 0;
	byte 	recNo;

	VERIFY(rsp);
	trcS("parseIssTbl:\n");
	trcBN(rsp, len);
	trcS("\n");

#ifdef OLD_CODE
	ret = mapReset(issBeg);     
	CHECK(ret > 0, lblKO);
#else 
	MAPGETBYTE(regIssCnt, recNo, lblKO);
	CHECK(recNo < dimIss, lblKO);

	if(recNo == 0)
	{            
	ret = mapReset(issBeg); 
	CHECK(ret > 0, lblKO);
	}
#endif
	while((len > 0) && (!(len % lenTmsIssDatSze)))
	{   
		MAPGETBYTE(regIssCnt, recNo, lblKO);
		CHECK(recNo < dimIss, lblKO);

		ret = mapMove(issBeg, recNo);
		CHK;
		ret = parseIssuer(rsp, &len);
		CHK;
		rsp += lenTmsIssDatSze;

		recNo++;
		MAPPUTBYTE(regIssCnt, recNo, lblKO);
	}
	
	CHECK(len == 0, lblKO);
	MAPPUTBYTE(regIssCnt, recNo, lblKO);

	return ret;
	lblKO:
		return -1;
}

static int parseBinRangeName(const byte * rsp, word length)
{
	return saveParseStr(rsp, binName, lenName, "parseBinRangeName");
}

static int parseBinRangeId(const byte * rsp, word length)
{
	return saveParseByte(rsp, binID, "parseBinRangeId");
}

static int parseBinRangeLowPan(const byte * rsp, word length)
{
	char 	buf[lenBinPrefix + 2];
	int 		ret;

	memset(buf, 0x00, sizeof(buf));
	buf[0] = lenBinPrefix;
	memcpy(&buf[1], rsp, lenBinPrefix);

	ret = saveParseStr((byte *) buf, binLo, 1 + lenBinPrefix, "parseBinRangeLowPan");
	CHK;
	ret = lenBinPrefix;
	return ret;

	lblKO:
		return -1;
}

static int parseBinRangeHighPan(const byte * rsp, word length)
{
	char 	buf[lenBinPrefix + 2];
	int 		ret;

	memset(buf, 0, sizeof(buf));
	buf[0] = lenBinPrefix;
	memcpy(&buf[1], rsp, lenBinPrefix);

	ret =
	saveParseStr((byte *) buf, binHi, 1 + lenBinPrefix, "parseBinRangeHighPan");
	CHK;
	ret = lenBinPrefix;
	return ret;

	lblKO:
		return -1;
}

static int parseBinRangeCrIssId(const byte * rsp, word length)
{
	return saveParseByte(rsp, binCrIssID, "parseBinRangeCrIssId");
}

static int parseBinRangeCrAcqId(const byte * rsp, word length)
{
	return saveParseByte(rsp, binCrAcqID, "parseBinRangeCrAcqId");
}

static int parseBinRangePanLength(const byte * rsp, word length)
{
	return saveParseByte(rsp, binPanLen, "parseBinRangePanLength");
}

static int parseBinRangeDbIssId(const byte * rsp, word length)
{
	return saveParseByte(rsp, binDbIssID, "parseBinRangeDbIssId");
}

static int parseBinRangeDbAcqId(const byte * rsp, word length)
{
	return saveParseByte(rsp, binDbAcqID, "parseBinRangeDbAcqId");
}

static int parseBin(const byte * rsp, word * rsplen)
{
	int 		ret = 0;
	char 	buffer[prtW + 1];
	word 	len, length = 1;
	byte 	binLogo;
	byte 	Bin[lenBinPrefix + 1];

	len = *rsplen;

	PARSE(parseLength);
	PARSE(parseBinRangeName);
	PARSE(parseBinRangeId);
	PARSE(parseBinRangeLowPan);

	memset(buffer, 0, sizeof(buffer));
	MAPGET(binLo, Bin, lblKO);
	bin2hex(buffer, &Bin[1], 5);

	binLogo = valBinLogoAssign(buffer);

	MAPPUTBYTE(binLogoID, binLogo, lblKO);  
	PARSE(parseBinRangeHighPan);
	PARSE(parseBinRangeCrIssId);
	PARSE(parseBinRangeCrAcqId);
	PARSE(parseBinRangePanLength);
	PARSE(parseBinRangeDbIssId);
	PARSE(parseBinRangeDbAcqId);

	*rsplen = len;

	return ret;
	lblKO:
		return -1;
}

static int parseBinTbl(const byte * rsp, word len)
{
	int 		ret = 0;
	byte 	recNo;

	VERIFY(rsp);
	trcS("parseBinTbl:\n");
	trcBN(rsp, len);
	trcS("\n");

#ifdef OLD_CODE
	ret = mapReset(binBeg);     
	CHECK(ret > 0, lblKO);
#else
	MAPGETBYTE(regBinCnt, recNo, lblKO);
	CHECK(recNo < dimBin, lblKO);

	if(recNo == 0)
	{           
		ret = mapReset(binBeg); 
		CHECK(ret > 0, lblKO);
	}
#endif
	while((len > 0) && (!(len % lenTmsBinDatSze)))
	{    
		MAPGETBYTE(regBinCnt, recNo, lblKO);
		CHECK(recNo < dimBin, lblKO);
		mapMove(binBeg, recNo);

		ret = parseBin(rsp, &len);
		CHK;
		rsp += lenTmsBinDatSze;

		recNo++;
		MAPPUTBYTE(regBinCnt, recNo, lblKO);
	}
	CHECK(len == 0, lblKO);
	MAPPUTBYTE(regBinCnt, recNo, lblKO);

	return ret;
	lblKO:
		return -1;
}

static int parseAppDefTra(const byte * rsp, word length)
{
	return saveParseByte(rsp, appDefTra, "parseAppDefTra");
}

static int parseAppAmtFmt(const byte * rsp, word length)
{
	return saveParseByte(rsp, appAmtFmt, "parseAppAmtFmt");
}

static int parseAppDateTime(const byte * rsp, word length)
{
	return parseSkip(rsp, 6, "parseAppDateTime");
}

static int parseAppAdmPwd(const byte * rsp, word length)
{
	char 	buf[lenPwd + 1];
	int 		ret;

	memset(buf, 0, sizeof(buf));
	bin2hex(buf, rsp, lenPwd / 2);
	ret = saveParseStr((byte *) buf, appAdmPwd, lenPwd, "parseAppAdmPwd");
	CHK;
	ret = lenPwd / 2;
	return ret;
	
	lblKO:
		return -1;
}

static int parseAppHelpDesk(const byte * rsp, word length)
{
	return parseSkip(rsp, 12, "parseAppHelpDesk");
}

static int parseAppTermOpt1(const byte * rsp, word length)
{
	return saveParseByte(rsp, appTermOpt1, "parseAppTermOpt1");
}

static int parseAppTermOpt2(const byte * rsp, word length)
{
	return saveParseByte(rsp, appTermOpt2, "parseAppTermOpt2");
}

static int parseAppTermOpt3(const byte * rsp, word length)
{
	return parseSkip(rsp, sizeof(byte), "parseAppTermOpt3");
}

static int parseAppTermOpt4(const byte * rsp, word length)
{
	return parseSkip(rsp, sizeof(byte), "parseAppTermOpt4");
}

static int parseRptHdrSiteName(const byte * rsp, word length)
{
	return saveParseStr(rsp, rptHdrSiteName, 24, "parseRptHdrSiteName");
}

static int parseRptHdrSiteAddr1(const byte * rsp, word length)
{
	return saveParseStr(rsp, rptHdrSiteAddr1, 24, "parseRptHdrSiteAddr1");
}

static int parseRptHdrSiteAddr2(const byte * rsp, word length)
{
	return saveParseStr(rsp, rptHdrSiteAddr2, 24, "parseRptHdrSiteAddr2");
}

static int parseRptHdrSiteAddr3(const byte * rsp, word length)
{
	return saveParseStr(rsp, rptHdrSiteAddr3, 24, "parseRptHdrSiteAddr3");
}

static int parseRptHdrSiteAddr4(const byte * rsp, word length)
{
	return saveParseStr(rsp, rptHdrSiteAddr4, 24, "parseRptHdrSiteAddr4");
}

static int parseAppPointPos(const byte * rsp, word length)
{
	return parseSkip(rsp, sizeof(byte), "parseRptHdrSitePos");
}

static int parseAppLang(const byte * rsp, word length)
{
	return parseSkip(rsp, sizeof(byte), "parseAppLang");
}

static int parseAppLocOpt(const byte * rsp, word length)
{
	return saveParseByte(rsp, appLocOpt, "parseAppLocOpt");
}

static int parseAppCurrSign(const byte * rsp, word length)
{
	return saveParseStr(rsp, appCurrSign, lenCurrSign, "parseAppCurrSign");
}

static int parseAppVoidPwd(const byte * rsp, word length)
{
	char 	buf[lenPwd + 1];
	int 		ret;

	memset(buf, 0, sizeof(buf));
	bin2hex(buf, rsp, lenPwd / 2);
	ret = saveParseStr((byte *) buf, appVoidPwd, lenPwd, "parseAppVoidPwd");
	CHK;
	ret = lenPwd / 2;
	return ret;
	
	lblKO:
		return -1;
}

static int parseAppRefPwd(const byte * rsp, word length)
{
	char 	buf[lenPwd + 1];
	int 		ret;

	memset(buf, 0, sizeof(buf));
	bin2hex(buf, rsp, lenPwd / 2);
	ret = saveParseStr((byte *) buf, appRefPwd, lenPwd, "parseAppRefPwd");
	CHK;
	ret = lenPwd / 2;
	return ret;
	
	lblKO:
		return -1;
}

static int parseAppStlPwd(const byte * rsp, word length)
{
	char 	buf[lenPwd + 1];
	int 		ret;

	memset(buf, 0x00, sizeof(buf));
	bin2hex(buf, rsp, lenPwd / 2);
	ret = saveParseStr((byte *) buf, appSetlPwd, lenPwd, "parseAppStlPwd");
	CHK;
	ret = lenPwd / 2;
	return ret;
	
	lblKO:
		return -1;
}

static int parseAppTermOpt5(const byte * rsp, word length)
{
	return saveParseByte(rsp, appTermOpt5, "parseAppTermOpt5");
}

static int parseAppDefTransaction(const byte * rsp, word length)
{
	return parseSkip(rsp, sizeof(byte), "parseAppDefTrans");
}

static int parseAppEMSale(const byte * rsp, word length)
{
	char 	contStr[7];
	int 		ret;

	memset(contStr, 0x00, sizeof(contStr));
	if(*rsp & 0x01)
		strcat(contStr, "c");
	if(*rsp & 0x02)
		strcat(contStr, "m");
	if(*rsp & 0x04)
		strcat(contStr, "k");
	if(*rsp & 0x08)
		MAPPUTBYTE(appServiceCode, 1, lblKO);

	trcFS("appEMSale: %s\n", contStr);

	mapMove(rqsBeg, (word) (trtSale - 1));
	MAPPUTSTR(rqsEntMod, contStr, lblKO);
	return saveParseByte(rsp, appEMSale, "parseAppEMSale");
	
	lblKO:
		return -1;
}

static int parseAppEMRefund(const byte * rsp, word length)
{
	char 	contStr[4];
	int 		ret;

	memset(contStr, 0x00, sizeof(contStr));
	if(*rsp & 0x01)
		strcat(contStr, "c");
	if(*rsp & 0x02)
		strcat(contStr, "m");
	if(*rsp & 0x04)
		strcat(contStr, "k");

	trcFS("appEMRefund: %s\n", contStr);

	mapMove(rqsBeg, (word) (trtRefund - 1));
	MAPPUTSTR(rqsEntMod, contStr, lblKO);
	return saveParseByte(rsp, appEMRefund, "parseAppEMRefund");
	
	lblKO:
		return -1;
}

static int parseAppEMOffline(const byte * rsp, word length)
{
	char 	contStr[4];
	int 		ret;

	memset(contStr, 0x00, sizeof(contStr));
	if(*rsp & 0x01)
		strcat(contStr, "c");
	if(*rsp & 0x02)
		strcat(contStr, "m");
	if(*rsp & 0x04)
		strcat(contStr, "k");

	trcFS("appEMOffline: %s\n", contStr);

	mapMove(rqsBeg, (word) (trtOffline - 1));
	MAPPUTSTR(rqsEntMod, contStr, lblKO);
	return saveParseByte(rsp, appEMOffline, "parseAppEMOffline");
	
	lblKO:
		return -1;
}

static int parseAppEMCash(const byte * rsp, word length)
{
	char 	contStr[4];
	int 		ret;

	memset(contStr, 0x00, sizeof(contStr));
	if(*rsp & 0x01)
		strcat(contStr, "c");
	if(*rsp & 0x02)
		strcat(contStr, "m");
	if(*rsp & 0x04)
		strcat(contStr, "k");

	trcFS("appEMCash: %s\n", contStr);

	mapMove(rqsBeg, (word) (trtCash - 1));
	MAPPUTSTR(rqsEntMod, contStr, lblKO);
	return saveParseByte(rsp, appEMCash, "parseAppEMCash");
	
	lblKO:
		return -1;
}

static int parseAppEMPreAuth(const byte * rsp, word length)
{
	char contStr[4];
	int ret;

	memset(contStr, 0x00, sizeof(contStr));
	if(*rsp & 0x01)
		strcat(contStr, "c");
	if(*rsp & 0x02)
		strcat(contStr, "m");
	if(*rsp & 0x04)
		strcat(contStr, "k");

	trcFS("appEMPreAuth: %s\n", contStr);

	mapMove(rqsBeg, (word) (trtPreAut - 1));
	MAPPUTSTR(rqsEntMod, contStr, lblKO);
	return saveParseByte(rsp, appEMPreAuth, "parseAppEMPreAuth");
	
	lblKO:
		return -1;
}

static int parseAppEMCrdVrfy(const byte * rsp, word length)
{
	char 	contStr[4];
	int 		ret;

	memset(contStr, 0x00, sizeof(contStr));
	if(*rsp & 0x01)
		strcat(contStr, "c");
	if(*rsp & 0x02)
		strcat(contStr, "m");
	if(*rsp & 0x04)
		strcat(contStr, "k");

	trcFS("appEMCrdVrfy: %s\n", contStr);

	mapMove(rqsBeg, (word) (trtVerify - 1));
	MAPPUTSTR(rqsEntMod, contStr, lblKO);
	return saveParseByte(rsp, appEMCrdVrfy, "parseAppEMCrdVrfy");
	
	lblKO:
		return -1;
}

static int parseAppEMBalance(const byte * rsp, word length)
{
	char 	contStr[4];
	int 		ret;

	memset(contStr, 0x00, sizeof(contStr));
	if(*rsp & 0x01)
		strcat(contStr, "c");
	if(*rsp & 0x02)
		strcat(contStr, "m");
	if(*rsp & 0x04)
		strcat(contStr, "k");

	trcFS("appEMBalance: %s\n", contStr);

	mapMove(rqsBeg, (word) (trtBalance - 1));
	MAPPUTSTR(rqsEntMod, contStr, lblKO);
	return saveParseByte(rsp, appEMBalance, "parseAppEMBalance");
	
	lblKO:
		return -1;
}

static int parseConTbl(const byte * rsp, word len)
{
	int ret = 0;
	word length = 1;

	VERIFY(rsp);
	trcS("parseCONTBL:\n");
	trcBN(rsp, len);
	trcS("\n");

	PARSE(parseLength);
	PARSE(parseAppAmtFmt);
	PARSE(parseAppDefTra);
	PARSE(parseAppDateTime);
	PARSE(parseAppAdmPwd);
	PARSE(parseAppHelpDesk);
	PARSE(parseAppTermOpt1);
	PARSE(parseAppTermOpt2);
	PARSE(parseAppTermOpt3);
	PARSE(parseAppTermOpt4);
	PARSE(parseRptHdrSiteName);
	PARSE(parseRptHdrSiteAddr1);
	PARSE(parseRptHdrSiteAddr2);
	PARSE(parseRptHdrSiteAddr3);
	PARSE(parseRptHdrSiteAddr4);
	PARSE(parseAppPointPos);
	PARSE(parseAppLang);
	PARSE(parseAppLocOpt);
	PARSE(parseAppCurrSign);
	PARSE(parseAppVoidPwd);
	PARSE(parseAppRefPwd);
	PARSE(parseAppStlPwd);
	PARSE(parseAppTermOpt5);
	PARSE(parseAppDefTransaction);
	PARSE(parseAppEMSale);
	PARSE(parseAppEMRefund);
	PARSE(parseAppEMOffline);
	PARSE(parseAppEMCash);
	PARSE(parseAppEMPreAuth);
	PARSE(parseAppEMCrdVrfy);
	PARSE(parseAppEMBalance);

	CHECK(len == 0, lblKO);

	return ret;
	lblKO:
		return -1;
}

static int parseTMSLP(const byte * val, word len)
{
	int 		ret = 0;
	byte 	TmsMsg;

	VERIFY(val);
	trcS("parseTMSLP:\n");
	trcBN(val, len);
	trcS("\n");

	MAPGETBYTE(traTmsMsg, TmsMsg, lblKO);
	switch (TmsMsg) {
		case tmsDnlConTbl:
			MAPPUTBYTE(traTmsMreRec, val[0], lblKO);
			ret = parseConTbl(val + 1, len - 1);
			break;
			
		case tmsDnlBinTbl:
			MAPPUTBYTE(traTmsMreRec, val[0], lblKO);
			ret = parseBinTbl(val + 1, len - 1);
			break;
			
		case tmsDnlIssTbl:
			MAPPUTBYTE(traTmsMreRec, val[0], lblKO);
			ret = parseIssTbl(val + 1, len - 1);
			break;
			
		case tmsDnlAcqTbl:
			MAPPUTBYTE(traTmsMreRec, val[0], lblKO);
			ret = parseAcqTbl(val + 1, len - 1);
			break;
			
		case tmsEmvReq:
			ret = parseEmvReq(val, len);
			break;
			
		case tmsEmvFileDnl:
			ret = parseEmvFleDnl(val, len);
			break;
			
		case tmsEmvFileOpn:
			ret = parseEmvFleOpn(val, len);
			break;
			
		case tmsEmvFileRed:
			ret = parseEmvFleRed(val, len);
			break;
			
		default:
			break;
	}
	return ret;
	lblKO:
		return -1;
}

static int parseICCData(const byte * val, word lenVal)
{
	int 		ret = 0;
	card 	ctl = 0;
	word 	key;
	byte 	tag[2];
	byte 	len[1];
	byte 	dat[256];
	card 	lenDat;
	byte 	datICC[128+1];
	card 	tag2;

	VERIFY(val);
	trcS("parseICCData: ");
	trcBN(val, lenVal);
	trcS("\n");

	while(ctl < lenVal)
	{
		trcS("Tag: \n");
		if(((*val + ctl) & 0x1F) == 0x1F)
		{
			tag2 = (*(val+ctl) << 8) + *(val+ctl+1);
			memcpy(tag, val + ctl, 2);  
			trcBN(tag, 2);
			ctl += 2;
		}
		else
		{
			tag2 = (card)*(val + ctl);
			memcpy(tag, val + ctl, 1);  		
			trcBN(tag, 1);
			ctl += 1;
		}
		trcS("\n");

		memcpy(len, val + ctl, 1);
		bin2num(&lenDat, val + ctl, 1);
		trcS("Tag Len: \n");
		trcBN(len, 1);
		trcS("\n");
		ctl += 1;

		memcpy(dat, val + ctl, lenDat);
		trcFN("Len %d \n", lenDat);
		trcS("Tag Len Val: \n");
		trcBN(dat, lenDat);
		trcS("\n");
		ctl += lenDat;

		key = mapKeyTag((card) tag2);
		// ++ @agm - BRI
		if(key == 0)
			continue;
		// -- @agm - BRI
		datICC[0] = lenDat;
		memcpy(&datICC[1], dat, lenDat);
		ret = mapPut(key, datICC, lenDat+1);
	}

	return ret;
}

static int rspWorkingKey(const byte * pucRsp, word usLen)
{
	enum 	{ SIZE = 16 };        
	char 	tcWKey[SIZE + 1];
	byte 	tucDoubleWKey[SIZE];  
	byte 	ucLocMK;               
	byte 	ucLocPK;               
	char 	tcSep[] = { cFS, cETX, 0 };    
	int 		ret;

	VERIFY(pucRsp);
	trcS("# BEG rspWorkingKey #\n");

	ret = fmtTok(0, (char *) pucRsp, tcSep);   
	CHECK(usLen >= ret, lblKO);
	CHECK(SIZE >= ret, lblKO);  
	ret = fmtTok(tcWKey, (char *) pucRsp, tcSep);  
	VERIFY(ret <= SIZE);
	VERIFY(ret == (int) strlen(tcWKey));

	usLen = (byte) ret;
	MAPGETBYTE(appMKeyLoc, ucLocMK, lblKO); 
	MAPGETBYTE(appPinKeyLoc, ucLocPK, lblKO);  

	ret = cryStart('m');
	CHECK(ret >= 0, lblCry);
	hex2bin(tucDoubleWKey, tcWKey, SIZE);   
	hex2bin(&tucDoubleWKey[8], tcWKey, SIZE);
	ret = cryLoadDKuDK(ucLocPK, ucLocMK, tucDoubleWKey, 'p');
	CHECK(ret >= 0, lblCry);    

	ret = usLen;               
	goto lblEnd;

	lblCry:
	lblKO:                       
		trcErr(ret);
		ret = -1;
		goto lblEnd;
	lblEnd:
		cryStop('m');               
		trcFN("# END rspWorkingKey: ret=%d #\n", ret);
		return ret;
}

static int getFld(const byte * rsp, byte bit) {
	int 	ret;
	int 	fmt;
	card len;                   
	byte cnt;
	byte lenhex[5];             
	byte emvRespCode[3];
	byte TmsMsg;
	byte emvAuthCode[6];
	char buf[20];
	char buf1[20];
	char buf2[20];
	char temp[20];
	byte txnType; 


	VERIFY(rsp);
	fmt = isoFmt(bit);
	cnt = 0;

	if(fmt < 0)
	{               
		switch (-fmt)
		{
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

		switch (-fmt)
		{
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

	trcFN("getFld bit=%d\n", bit);
	trcS("getFld rsp=");
	trcBAN(rsp, len + cnt);
	trcS("\n");

	rsp += cnt;
	switch (bit) {
		case isoRrn:
			trcS("RRN:");
			trcBN(rsp, len);
			trcS("\n");
			ret = putVal(rsp, traRrn, len);
			break;
			
		case isoAutCod:
			trcS("AutCod:");
			trcBN(rsp, len);
			trcS("\n");
			ret = putVal(rsp, traAutCod, len);
			CHK;
			emvAuthCode[0] = len;
			memcpy(&emvAuthCode[1], rsp, len);
			ret = putVal(emvAuthCode, emvAutCod, len+1);
			CHK;
			break;
			
		case isoRspCod:
			trcS("RspCod:");
			trcBN(rsp, len);
			trcS("\n");
			ret = putVal(rsp, traRspCod, len);
			CHK;
			emvRespCode[0] = len;
			memcpy(&emvRespCode[1], rsp, len);
			ret = putVal(emvRespCode, emvRspCod, len + 1);
			CHK;

			MAPGETBYTE(traTmsMsg, TmsMsg, lblKO);
			switch (TmsMsg) {
				case tmsEmvFileCls:
					ret = parseEmvFile();
					break;
					
				default:
					break;
			}
			break;
			
		case isoDat:
			trcS("isoDat:");
			trcBN(rsp, len);
			trcS("\n");
			ret = putVal(rsp, traRspBit13, len);
			CHK;
			break;
			
		case isoTim:
			trcS("isoTim: ");
			trcBN(rsp, len);
			trcS("\n");
			ret = putVal(rsp, traRspBit12, len);
			CHK;
			break;
			
		case isoSecCtl:
			ret = rspWorkingKey(rsp, len);
			CHK;
			break;
			
		case isoEmvPds:
			ret = parseICCData(rsp, len);
			CHK;
			break;

		case isoRoc:
			MAPGETBYTE(traTxnType,txnType,lblKO);
			if((txnType == trtDebitLogon) || (txnType == trtPreAuth) || (txnType == trtPreCom) || (txnType == trtPreCancel)){
				ret = parseField62(rsp,len);
				CHK;
			}
			break;

		//@@AS-SIMAS : Start
		case iso057:
			MAPGETBYTE(traTxnType,txnType,lblKO);
			if(txnType == trtDebitLogon) {
				ret = parseField57(rsp,len);
				CHK;
			}
			break;
		//@@AS-SIMAS : End
		
		case isoRsvPvt:
			MAPGETBYTE(traTxnType,txnType,lblKO);
			if(txnType == trtTMSLOGON || txnType == trtTMSLOGOFF || txnType == trtTMSDnlTbl ||txnType == trtTMSEmvReq || 	txnType == trtTMSEmvFleDnl)
				ret = parseTMSLP(rsp, len);
			else if(txnType == trtTLEKeyDownload)
				ret = parseField60(rsp,len);
			CHK;
			break;
			
		case isoPinMacKey:
			ret = parseField48(rsp,len);
			CHK;
			break;

		case isoAmt:
			memset(buf,0,sizeof(buf));
			bin2hex(buf,rsp,6);

			MAPGETBYTE(traTxnType,txnType,lblKO);
			if((txnType == trtPembayaranPLNPasca) || (txnType == trtPembayaranPLNPascaInquiry))
			{
				mapPut(traAmt,buf,lenInquiryAmt);
				CHK;
				break;
			}

			else if((txnType == trtPembayaranTVBerlangganan)||(txnType == trtPembayaranTVBerlanggananInquiry))
			{
				ret = mapPut(traTotalAmount,buf,lenInquiryAmt-2);
				CHK;
				mapPut(traAmt,buf,lenInquiryAmt-2);
				break;
			}
			else if((txnType == trtPembayaranTiketPesawatInquiry)||(txnType == trtPembayaranTiketPesawatInquiry))
			{
				ret = mapPut(traTotalAmount,buf,lenInquiryAmt-2);
				CHK;
				mapPut(traAmt,buf,lenInquiryAmt-2);
				break;
			}
			else if(txnType == trtPembayaranPdamInquiry)
			{
				ret = mapPut(traTotalAmount,buf,lenInquiryAmt);
				CHK;
				mapPut(traAmt,buf,lenInquiryAmt);
				break;
			} 
			else if(txnType == trtPembayaranPdam)
			{
				mapPut(traAmt,buf,lenInquiryAmt);
				break;
			} 
			else if(txnType == trtPembayaranSSBInquiry)
			{
				memset(buf1, 0, sizeof(buf));
				mapGet(traTotalOutstanding,buf1,lenTotalOutstanding);
				if(
					(strcmp(buf1, "81") == 0) ||
					(strcmp(buf1, "82") == 0) ||
					(strcmp(buf1, "91") == 0) ||
					(strcmp(buf1, "92") == 0) 

				)
				{
					memset(buf2, 0, sizeof(buf2));
					memset(temp, 0, sizeof(temp));
					mapGet(traBillStatus,buf2,lenBillStatus);
					sprintf(temp, "%d", ((atoi(buf))*(atoi(buf2))));
					mapPut(traAmt,temp,lenInquiryAmt);
				}
				else
				{
					mapPut(traAmt,buf,lenInquiryAmt);
				}
				break;
			
			} 
			if(txnType == trtPembayaranSSB)
			{
				mapPut(traAmt,buf,lenInquiryAmt);
				CHK;
				break;
			}
			else if((txnType == trtPembayaranSPP)||(txnType == trtPembayaranSPPInquiry))
			{
				ret = mapPut(traAmt,buf,lenInquiryAmt-2);
				CHK;
				break;
			}
			else if((txnType == trtPembayaranPLNToken))
			{
				ret = mapPut(traAmt,buf,lenInquiryAmt);
				CHK;
				break;
			}


			else if((txnType == trtPembayaranBriva)||(txnType == trtPembayaranBrivaInquiry))
			{
				if(txnType == trtPembayaranBrivaInquiry)
					ret = mapPut(traTotalAmount,buf,lenInquiryAmt);
				CHK;
				ret = mapPut(traAmt,buf,lenInquiryAmt);
				CHK;
				break;
			}
		
#ifdef PREPAID
			if(txnType == trtPrepaidAktivasi)
			{
				char amtAsli[20];

				memset(amtAsli,0,sizeof(amtAsli));
				memcpy(amtAsli,buf,lenAmt);
				amtAsli[lenAmt-2] = 0;
				fmtPad(amtAsli, -lenAmt, '0');

				ret = putVal(amtAsli, traAmt, lenAmt);
				CHK;
			}
#endif
			ret = putVal(buf, traInquiryAmt, lenInquiryAmt);
			CHK;
			break;
		
		case isoAmtOrg:
			ret = parseField61(rsp,len);
			CHK;
			break;

		case isoSTAN:
			ret = putVal(rsp, traRspBit11, len);
			CHK;
			break;

		case isoAddDat:
			ret = parseField63(rsp,len);
			CHK;
			break;

		case isoTid:
			ret = putVal(rsp, traRspBit41, len);
			CHK;
			break;

		case isoMid:
			ret = putVal(rsp, traRspBit42, len);
			CHK;
			break;
		default:
			break;
	}

	return cnt + len;
	lblKO:
		return ret;
}

int rspParse(const byte * rsp, word len)
{
	int 	ret;
	byte BitMap[8];
	byte bit;
	byte test55 = 0;
	byte EntMod = 'M';

	VERIFY(rsp);

	trcS("rspParse rsp:");
	trcBN(rsp, len);
	trcS("\n");

	mapPut(traMtiRsp,rsp,2);
	mapPut(traRspCod,"\x00\x00",2);

	MOV(2);                

	memset(BitMap, 0, 8);
	memcpy(BitMap, rsp, 8);
	trcS("bmp:");
	trcBN(BitMap, 8);
	trcS("\n");
	MOV(8);                    

	for (bit = 2; bit <= 64; bit++)
	{
		if(!bitTest(BitMap, bit))
			continue;
		if (bit == 55)
		{
			if (EntMod == 'c' || EntMod == 'C')
			test55 = 1;
		}
		ret = getFld(rsp, bit);
		CHK;
		MOV(ret);
	}

	trcFN("rspParse: ret=%d\n", len);
	return len;                
	lblKO:
		return ret;
}
