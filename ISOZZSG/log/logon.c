//#include <sdk30.h>
//#include <Libgr.h>
//#include "sys.h"
#include "log.h"
#include "bri.h"
#include "keyinject.h"
#include <string.h>
#include <stdlib.h>
#include "stdio.h"

unsigned long DateToInt(byte d,byte m,word y){
	byte i;
	unsigned long xy;

	xy = (y-1) / 4;
	xy = (xy * 366) + ((y-xy-1) * 365);

	for (i=2; i<=m; i++) {
		switch (i) {
			case 2:
			case 4:
			case 6:
			case 8:
			case 9:
			case 11:
				xy += 31;
				break;
			case 5:
			case 7:
			case 10:
			case 12:
				xy += 30;
				break;
			case 3:
				if ((y % 4) == 0)
					xy += 29;
				else
					xy += 28;
				break;
		}
	}

	xy += d;

 	return (xy);
}

unsigned long TimeToInt(byte h, byte m, byte s){
	unsigned long x;
	x = h*60;
	x = (x+m)*60;
	x += s;
	return(x);
}

int debitLogon(byte previousTrnType,byte forceLogon) {
	int ret;
	byte AcqId;
	char traName[dspW + 1];
	byte WK[lenMKey + 1];
	char RspCod[lenRspCod + 1];
	byte firstPowerOn;
	byte tak[16+1]; //@@AS-SIMAS
	byte MK[lenMKey + 1];// @@AS-SIMAS
	byte encResult[100];// @@AS-SIMAS

	byte regLocType_orig;
	word traMnuItm_orig;
	char traTransType_orig[dspW + 1], traTypeName_orig[dspW + 1];
	unsigned long ulDateSys,ulTimeSys,ulDate,ulTime,deltaDate,deltaTime;
	//char datetime[lenDatTim + 1];
	char buf[30],buf1[30],strCurrentDT[20];
	byte logonRequired=0;
//	DATE Date;
	int yy, mm, dd;
	int hh, min, ss;
	char	tid[9];


	memset(RspCod,0x00,sizeof(RspCod)); //@@AS0.0.33

	trcS("debitLogon Beg\n");

	trcS("get the original value\n");
	MAPGETBYTE(regLocType,regLocType_orig,lblKO);
	MAPGETWORD(traMnuItm, traMnuItm_orig, lblKO);
	MAPGET(traTransType, traTransType_orig, lblKO);
	MAPGET(traTypeName, traTypeName_orig, lblKO);

	trcS("overwrite value\n");

	mapMove(rqsBeg, (word) (trtDebitLogon - 1));
	MAPGET(rqsDisp, traName, lblKO);
	trcFS("traName: %s\n", traName);

	MAPPUTBYTE(regLocType,'T',lblKO);
	MAPPUTWORD(traMnuItm, mnuDebitLogon, lblKO);
	MAPPUTSTR(traTransType, traName, lblKO);
	MAPPUTSTR(traTypeName, traName, lblKO);
	MAPPUTBYTE(traTxnType, trtDebitLogon, lblKO);

	//=========
	ret = briGetAcq(1, &AcqId);
	if (ret < 0)
		goto lblKO;
	if (ret == 0)
		goto lblAcqNotFound;

	trcFN("AcqId: %d\n", AcqId);
	trcFN("IssId: %d\n", IssId);
	AcqId--; //Acquirer Id is 1 higher than the actual data base index for Acquirer

	CHECK(AcqId < dimAcq, lblKO);
	MAPPUTBYTE(traAcqIdx, AcqId, lblKO);
	mapMove(acqBeg, (word) AcqId);


	//@@AS-SIMAS - start : check key is loaded yet or not
	ret=mapGet(acqTAKLogon,tak,lenKEY);
	if(ret<0) return ret;
	if (memcmp(tak,__DFTTLE__,16)==0) {
		usrInfo(infKeyNotLoaded);
		goto lblKO;
	}
	//@@AS-SIMAS - end : check key is loaded yet or not

	//=========
	memset(tid,0x00,sizeof(tid));
	memset(buf,0x00,sizeof(buf));
	strcpy(buf, "20");     //CC
	ret = getDateTime(buf + 2);    //CC+YYMMDDhhmmss
	CHECK(ret > 0, lblKO);

	memcpy(buf1,buf,4);
	buf1[4]=0;
	yy = atoi(buf1);
	memcpy(buf1,&buf[4],2);
	buf1[2]=0;
	mm = atoi(buf1);
	memcpy(buf1,&buf[6],2);
	buf1[2]=0;
	dd = atoi(buf1);
	memcpy(buf1,&buf[8],2);
	buf1[2]=0;
	hh = atoi(buf1);
	memcpy(buf1,&buf[10],2);
	buf1[2]=0;
	min = atoi(buf1);
	memcpy(buf1,&buf[12],2);
	buf1[2]=0;
	ss = atoi(buf1);

	ulDateSys=DateToInt(dd,mm,yy); //date from system
	ulTimeSys=TimeToInt(hh,min,ss); //time from system
	sprintf(strCurrentDT, "%04d%02d%02d%02d%02d%02d", yy, mm, dd, hh, min,ss);

	MAPGET(appLogonDT, buf, lblKO); //Get the last logon date time
	MAPGETBYTE(appIsFirstPowerOn,firstPowerOn,lblKO);
	MAPGET(acqTID,tid,lblKO);

	if (forceLogon|firstPowerOn) {
		//if forceLogon, no need to check the deltaTime
		logonRequired =1; //ganti ini ya
		//logonRequired =0;
	}
	else {
		if (memcmp(buf,"00000000000000",lenDatTim)==0){
			logonRequired =1; //ganti ini ya
			//logonRequired =0;
		}
		else {
			memcpy(buf1,buf,4);
			buf1[4]=0;
			yy = atoi(buf1);
			memcpy(buf1,&buf[4],2);
			buf1[2]=0;
			mm = atoi(buf1);
			memcpy(buf1,&buf[6],2);
			buf1[2]=0;
			dd = atoi(buf1);
			memcpy(buf1,&buf[8],2);
			buf1[2]=0;
			hh = atoi(buf1);
			memcpy(buf1,&buf[10],2);
			buf1[2]=0;
			min = atoi(buf1);
			memcpy(buf1,&buf[12],2);
			buf1[2]=0;
			ss = atoi(buf1);

			ulDate=DateToInt(dd,mm,yy); //date from acq table
			ulTime=TimeToInt(hh,min,ss); //time from acq table

			if (ulDateSys>ulDate) {
				deltaDate = ulDateSys-ulDate;
				if (deltaDate==1) {
					deltaTime = 86400 + ulTimeSys - ulTime;
					if (deltaTime >= 86400)
						logonRequired=1;
				}else if (deltaDate>1)
					logonRequired =1;
			}
			else if (ulDateSys<ulDate) // the time goes back
				logonRequired = 1;
		}
	}

	if (logonRequired) {
		ret = onlSession();
		CHECK(ret > 0, lblKO);

		MAPGET(traRspCod, RspCod, lblKO);
		CHECK(ret >= 0, lblKO);

		if (memcmp(RspCod,"00",2)==0) {
			// @@AS-SIMAS - start
			ret=mapGet(acqTMK,MK,lenKEY);
			CHECK(ret > 0, lblKO);
#ifdef _DEBUG_TLE
			hex_dump_char("--acqMK--",MK,16);
#endif


			ret = mapGet(traTPKenc, WK, sizeof(WK));
			CHECK(ret > 0, lblKO);
#ifdef _DEBUG_TLE
			hex_dump_char("--TPK-ENC--",WK,16);
#endif
			ret = storeWorkingKey(1, WK);
			CHECK(ret > 0, lblKO);


			ret = mapGet(traTAKenc, WK, sizeof(WK));
			CHECK(ret > 0, lblKO);
#ifdef _DEBUG_TLE
			hex_dump_char("--TAK-ENC--",WK,16);
#endif
			stdDED(encResult,WK, MK);
			stdDED(&encResult[8],&WK[8], MK);
#ifdef _DEBUG_TLE
			hex_dump_char("--TAK--",encResult,16);
#endif

			ret = mapGet(traTLEenc, WK, sizeof(WK));
			CHECK(ret > 0, lblKO);
#ifdef _DEBUG_TLE
			hex_dump_char("--TLE-ENC--",WK,16);
#endif
			stdDED(&encResult[16],WK, MK);
			stdDED(&encResult[24],&WK[8], MK);
#ifdef _DEBUG_TLE
			hex_dump_char("--TLE--",&encResult[16],16);
#endif
			store_acqTAKnTLE(encResult,&encResult[16]);
			// @@AS-SIMAS - end


			MAPPUTSTR(appLogonDT,strCurrentDT,lblKO);
		}
		else goto lblKO;


		//=============
	}

	trcS("restore the original value\n");
	mapMove(rqsBeg, (word) (previousTrnType - 1));
	MAPPUTBYTE(regLocType,regLocType_orig,lblKO);
	MAPPUTWORD(traMnuItm, traMnuItm_orig, lblKO);
	MAPPUTSTR(traTransType, traTransType_orig, lblKO);
	MAPPUTSTR(traTypeName, traTypeName_orig, lblKO);
	MAPPUTBYTE(traTxnType, previousTrnType, lblKO);

	//@@AS0.0.27 : Fix Logon's bug, restore the acquirer multirecords to its origin - start
	if (!forceLogon) {
		ret=valBin();
		CHECK(ret > 0, lblAcqNotFound);
	}
	//@@AS0.0.27 : Fix Logon's bug, restore the acquirer multirecords to its origin - end


	//===============

	if (forceLogon && !firstPowerOn) {
		usrInfo(infLogonSuccess);
	}

	goto lblEnd;

lblKO:
	resetLogonDateTime(); //reset the last logon date time
    ret = -1;
	usrInfo(infLogonFail);
lblAcqNotFound:
lblEnd:
	if (firstPowerOn) {
		setFirstPowerOnFlag(0);
	}
	return ret;
	trcS("tleKeyDownload End\n");
}

