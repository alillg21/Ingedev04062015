#include <string.h>
#include <stdio.h>

#include "tst.h"
#ifdef __TEST__

#define RET_OK 0
#define RET_CANCEL -1
#define RPT_SEP 	ret= prtS("------------------------"); CHK;
#define HDR_SEP 	ret= prtS("========================"); CHK;
#define EMPTY_LINE 	ret= prtS("                        "); CHK;
#define SMALL_SEP	ret= prtS("----------------"); CHK;

static code const char *srcFile= __FILE__;
#define CHK CHECK(ret>=0,lblKO)

static int prepareData(void){
	int ret=RET_OK;CHK;

	mapMove(aidBeg,0);
	mapPutStr(traPan,"4015507800000041");
	mapPutStr(traAmt,"1000");
	mapPutStr(traExpDat,"0412");
	mapPutStr(traAppCod,"123456");
	datTrue(traExemplClient);

	goto lblEnd;
lblKO:
	trcErr(ret);
lblEnd:
	return ret;
}

// this version doesn't require to restore any data
static int restoreData(void){
	int ret=RET_OK;CHK;
	goto lblEnd;
lblKO:
    trcErr(ret);
lblEnd:
	return ret;
}
static int showReceipts(){
	int key;
	char tmpstr2[lenRpt+1];
	int ret=RET_OK;

	dspClear();
	dspLS(1," Printing the   ");
	dspLS(1," Receipt lines..");

	for (key=rloBeg+1;key<rloEnd;key++){
		ret=sprintf(tmpstr2,"Receipt template: %d",(int)(key-rloBeg)); ret=prtS(tmpstr2); CHK;
		HDR_SEP
		ret= rptReceipt(key);  CHK;
		//EMPTY_LINE
		HDR_SEP
	}

	goto lblEnd;
lblKO:
	trcErr(ret);
lblEnd:
	return ret;
}

void tcts0100(void){
	prepareData();
	showReceipts();
	restoreData();
	return;
}

void tcts0001(void){// testing hash functions
	byte hash1;
	byte hash2;
	byte hash3;
	byte cardno1[10];
	byte cardno2[10];
	char cardno3[20];
	char tmp[30];

	strcpy(cardno3,"67609999070005001");
	prtS("Card chars:");
	prtS(cardno3);
	hash3=panDecHash(cardno3);
	bin2hex(tmp,&hash3,1);
	tmp[2]=0;
	prtS(tmp);

	prtS(" ");
	prtS("Card binary:");
	prtS("67609999070005001F");
	memcpy(cardno2,"\x67\x60\x99\x99\x07\x00\x05\x00\x1F",9);
	hash2=panBinHash(cardno2);
	bin2hex(tmp,&hash2,1);
	tmp[2]=0;
	prtS(tmp);


	prtS(" ");
	prtS("Card binary:");
	prtS("67609999070005001FFF");
	memcpy(cardno1,"\x67\x60\x99\x99\x07\x00\x05\x00\x1F\xFF",10);
	hash1=panBinHash(cardno1);
	bin2hex(tmp,&hash1,1);
	tmp[2]=0;
	prtS(tmp);

	prtS(" ");
	prtS("16 byte card chars:");
	strcpy(cardno3,"4905150000070053");
	prtS(cardno3);
	hash1=panDecHash(cardno3);
	bin2hex(tmp,&hash1,1);
	tmp[2]=0;
	prtS(tmp);

	prtS("and binary:");
	prtS(cardno3);
	memset(cardno2,0,sizeof(cardno2));
	memcpy(cardno2,"\x49\x05\x15\x00\x00\x07\x00\x53",8);
	hash1=panDecHash(cardno3);
	bin2hex(tmp,&hash1,1);
	tmp[2]=0;
	prtS(tmp);

	hash2=panBinHash(cardno2);
	bin2hex(tmp,&hash2,1);
	tmp[2]=0;
	prtS(tmp);

	prtS("16 byte card chars:");
	strcpy(cardno3,"4905150000070064");
	prtS(cardno3);
	hash2=panDecHash(cardno3);
	bin2hex(tmp,&hash2,1);
	tmp[2]=0;
	prtS(tmp);
	strcpy(cardno3,"4905150000070075");
	prtS(cardno3);
	hash3=panDecHash(cardno3);
	bin2hex(tmp,&hash3,1);
	tmp[2]=0;
	prtS(tmp);

}
#endif
