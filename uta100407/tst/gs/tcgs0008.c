// tcgs0007
//
// author:      Gábor Sárospataki
// date:        01.02.2006
// description: test case to oprSpr combo

#include <string.h>
#include "pri.h"

//static code const char *srcFile= __FILE__;

#define CHK CHECK(ret>=0,lblKO)

void tcgs0008(void) {
/*
	enum {dim=8};
	int ret;
	tTable arg2,arg3;
	tRecord arg1;
	tContainer cntArg1,cntArg2,cntArg3;
	tLocator arg[3];
	tCombo cmbSpr;
	char bgd[prtW+1];
	char str[prtW+1];
	word number;
	byte sec;
	word ofs;
	int fld;
	word len;
	char dst[prtW+1];

	sec=0x0;
	ofs=0;

	nvmStart();
	//initialize the fields
	len=prtW;
	ret=tabInit(&arg1,sec,ofs,len,1); CHK;
	ofs+=ret;
	ret=tabInit(&arg2,sec,ofs,sizeof(number),1); CHK;
	ofs+=ret;
	ret=tabInit(&arg3,sec,ofs,len,1); CHK;
	ofs+=ret;

	//initialize the containers
	ret= cntInit(&cntArg1,'t',&arg1); CHK;
	ret= cntInit(&cntArg2,'t',&arg2); CHK;
	ret= cntInit(&cntArg3,'t',&arg3); CHK;

	//initialize the locators's pointer
	arg[0].cnt=&cntArg1;
	arg[1].cnt=&cntArg2;
	arg[2].cnt=&cntArg3;

	arg[0].key=0;
	arg[1].key=0;
	arg[2].key=0;

	ret= prtStart(); CHK;

	number=1935;
	ret= tabPut(&arg2,0,&number,prtW); CHK;
	strcpy(str,"1788");
	ret= tabPut(&arg3,0,str,prtW); CHK;

	memset(dst,0,sizeof(dst));
    ret= cmbInit(&cmbSpr,oprSpr,arg,prtW); CHK;
	strcpy(bgd,"Number:%d String:%s");
	ret= tabPut(&arg1,0,bgd,prtW); CHK;
	ret=cmbGet(&cmbSpr,fld,dst,prtW); CHK;
	CHECK(strcmp(dst,"Number:1935 String:1788")==0,lblKO);
	ret= prtS(dst); CHK;

	memset(dst,0,sizeof(dst));
    ret= cmbInit(&cmbSpr,oprSpr,arg,prtW); CHK;
	strcpy(bgd,"Number:%2d String:%s");
	ret= tabPut(&arg1,0,bgd,prtW); CHK;
	ret=cmbGet(&cmbSpr,fld,dst,prtW); CHK;
	CHECK(strcmp(dst,"Number:35 String:1788")==0,lblKO);
	ret= prtS(dst); CHK;

	memset(dst,0,sizeof(dst));
    ret= cmbInit(&cmbSpr,oprSpr,arg,prtW); CHK;
	strcpy(bgd,"Number:%d String:%-2s");
	ret= tabPut(&arg1,0,bgd,prtW); CHK;
	ret=cmbGet(&cmbSpr,fld,dst,prtW); CHK;
	CHECK(strcmp(dst,"Number:1935 String:17")==0,lblKO);
	ret= prtS(dst); CHK;

	memset(dst,0,sizeof(dst));
    ret= cmbInit(&cmbSpr,oprSpr,arg,prtW); CHK;
	strcpy(bgd,"Number:%D String:%2S");
	ret= tabPut(&arg1,0,bgd,prtW); CHK;
	ret=cmbGet(&cmbSpr,fld,dst,prtW); CHK;
	CHECK(strcmp(dst,"Number:1.935 String:88")==0,lblKO);
	ret= prtS(dst); CHK;

	memset(dst,0,sizeof(dst));
    ret= cmbInit(&cmbSpr,oprSpr,arg,prtW); CHK;
	strcpy(bgd,"Number:%5d String:%1S");
	ret= tabPut(&arg1,0,bgd,prtW); CHK;
	ret=cmbGet(&cmbSpr,fld,dst,prtW); CHK;
	CHECK(strcmp(dst,"Number: 1935 String:8")==0,lblKO);
	ret= prtS(dst); CHK;

	memset(dst,0,sizeof(dst));
    ret= cmbInit(&cmbSpr,oprSpr,arg,prtW); CHK;
	strcpy(bgd,"Number:%-6D String:%1S");
	ret= tabPut(&arg1,0,bgd,prtW); CHK;
	ret=cmbGet(&cmbSpr,fld,dst,prtW); CHK;
	CHECK(strcmp(dst,"Number:1.935  String:8")==0,lblKO);
	ret= prtS(dst); CHK;

	memset(dst,0,sizeof(dst));
    ret= cmbInit(&cmbSpr,oprSpr,arg,prtW); CHK;
	strcpy(bgd,"Number:%05d String:%1S");
	ret= tabPut(&arg1,0,bgd,prtW); CHK;
	ret=cmbGet(&cmbSpr,fld,dst,prtW); CHK;
	CHECK(strcmp(dst,"Number:01935 String:8")==0,lblKO);
	ret= prtS(dst); CHK;

	ret=1;
	goto lblEnd;

lblKO:
	trcErr(ret);
	ret=-1;
lblEnd:
	prtStop();
	return ret;
*/
}
