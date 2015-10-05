/**
 *	\brief	  unitary test case tctd0008
 *	\author   Tamas DANI
 *	\date	  09.10.2006
 *
 *	Combo processing. Testing operation OR (oprOr).
 */

#include <string.h>
#include "pri.h"

#define CHK CHECK(ret>0,lblKO)
#define CHECK_RESULT_TRUE  CHECK(result==1,lblResult)
#define CHECK_RESULT_FALSE CHECK(result==0,lblResult)

static int testOprOr_1(void);
static int testOprOr_2(void);

void tctd0011(void){
	int ret;

	ret= dspStart();       CHK;
	ret= testOprOr_1();    CHECK(ret>=0,lblKO)
	ret= testOprOr_2();    CHECK(ret>=0,lblKO)

	tmrPause(2);
	goto lblEnd;

lblKO:
	trcErr(ret);
	dspLS(0,"KO!");
	tmrPause(2);
lblEnd:
	dspStop();
}

static int testOprOr_1(void){
	int ret;
	tTable tab;
	tContainer cnt;
	tCombo cmb;
	tLocator loc[3];
	byte result;
	char volDba[4*4];
	card valTrue= 1;
	card valFalse= 0;

	memset(volDba,0,sizeof(volDba));
	dbaSetPtr(volDba);

	ret= tabInit(&tab,0xFF,0,sizeof(card),3);
	VERIFY(ret==(int)tabSize(&tab));
	ret= cntInit(&cnt,'t',&tab);                CHK;

	loc[0].cnt= &cnt;
	loc[0].key= 0;
	loc[1].cnt= &cnt;
	loc[1].key= 1;
	loc[2].cnt= 0;
	loc[2].key= 0;

	ret= cmbInit(&cmb,oprOr,loc,1);             CHK;

	ret= tabPut(&tab,0,&valTrue,sizeof(card));  CHK;
	ret= tabPut(&tab,1,&valFalse,sizeof(card)); CHK;
	ret= cmbGet(&cmb,0,&result,1);              CHK;

	// 1 OR 0 = TRUE
	CHECK_RESULT_TRUE;

	ret= tabPut(&tab,0,&valFalse,sizeof(card)); CHK;
	ret= tabPut(&tab,1,&valTrue,sizeof(card));  CHK;
	ret= cmbGet(&cmb,0,&result,1);              CHK;

	// 0 OR 1 = TRUE
	CHECK_RESULT_TRUE;

	ret= tabPut(&tab,0,&valTrue,sizeof(card));  CHK;
	ret= tabPut(&tab,1,&valTrue,sizeof(card));  CHK;
	ret= cmbGet(&cmb,0,&result,1);              CHK;

	// 1 OR 1 = TRUE
	CHECK_RESULT_TRUE;

	loc[2].cnt= &cnt;
	loc[2].key= 2;
	ret= cmbInit(&cmb,oprOr,loc,1);             CHK;
	ret= tabPut(&tab,2,&valTrue,sizeof(card));  CHK;
	ret= cmbGet(&cmb,0,&result,1);              CHK;

	// 1 OR 1 OR 1 = TRUE
	CHECK_RESULT_TRUE;

	ret= tabPut(&tab,0,&valFalse,sizeof(card)); CHK;
	ret= tabPut(&tab,1,&valFalse,sizeof(card)); CHK;
	ret= cmbGet(&cmb,0,&result,1);              CHK;

	// 0 OR 0 OR 1 = TRUE
	CHECK_RESULT_TRUE;

	ret= tabPut(&tab,2,&valFalse,sizeof(card)); CHK;
	ret= cmbGet(&cmb,0,&result,1);              CHK;

	// 0 OR 0 OR 0 = FALSE
	CHECK_RESULT_FALSE;

	dspLS(0,"TEST 1 OK");
	ret= 1;
	goto lblEnd;

lblKO:
	trcErr(ret);
	ret= -1;
	Click();
	goto lblEnd;
lblResult:
	dspLS(0,"Result not OK!");
	Click();
	ret= 0;
lblEnd:
	tmrPause(1);
	return ret;
}

static int testOprOr_2(void){
	int ret;
	tTable tab;
	tContainer cnt;
	tCombo cmb;
	tLocator loc[3];
	byte result;
	char volDba[prtW*4];

	memset(volDba,0,sizeof(volDba));
	dbaSetPtr(volDba);

	ret= tabInit(&tab,0xFF,0,prtW,3);
	VERIFY(ret==(int)tabSize(&tab));
	ret= cntInit(&cnt,'t',&tab);                CHK;

	loc[0].cnt= &cnt;
	loc[0].key= 0;
	loc[1].cnt= &cnt;
	loc[1].key= 1;
	loc[2].cnt= 0;
	loc[2].key= 0;

	ret= cmbInit(&cmb,oprOr,loc,1);             CHK;
	ret= tabPutStr(&tab,0,"first arg");         CHK;
	ret= cmbGet(&cmb,0,&result,1);              CHK;

	// "first arg" OR 0 = TRUE
	CHECK_RESULT_TRUE;

	ret= tabPutStr(&tab,1,"second arg");        CHK;
	ret= cmbGet(&cmb,0,&result,1);              CHK;

	// "first arg" OR "second arg" = TRUE
	CHECK_RESULT_TRUE;

	loc[2].cnt= &cnt;
	loc[2].key= 2;
	ret= cmbInit(&cmb,oprOr,loc,1);             CHK;
	ret= tabPutStr(&tab,2,"third arg");         CHK;
	ret= cmbGet(&cmb,0,&result,1);              CHK;

	// "first arg" OR "second arg" OR "third arg" = TRUE
	CHECK_RESULT_TRUE;

	ret= tabPutStr(&tab,0,"");                  CHK;
	ret= tabPutStr(&tab,1,"");                  CHK;
	ret= cmbGet(&cmb,0,&result,1);              CHK;

	// "" OR "" OR "third arg" = TRUE
	CHECK_RESULT_TRUE;

	memset(volDba,0,sizeof(volDba));
	ret= cmbGet(&cmb,0,&result,1);              CHK;

	// 0 OR 0 OR 0 = FALSE
	CHECK_RESULT_FALSE;

	dspLS(1,"TEST 2 OK");
	ret= 1;
	goto lblEnd;

lblKO:
	trcErr(ret);
	ret= -1;
	Click();
	goto lblEnd;
lblResult:
	dspLS(1,"Result not OK!");
	Click();
	ret= 0;
lblEnd:
	tmrPause(1);
	return ret;
}
