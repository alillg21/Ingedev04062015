#include <string.h>
#include "sys.h"
#include "tst.h"

//Unitary test case tcvp0016.
//Functions testing:
//  getPid
//  getAppName
//  getDateTime
//  setDateTime
//  getSapSer
//  getTS

/*
static int testGetPid(void){
    int ret;
    byte pid;
    char buf[dspW + 1];
	
    ret = dspLS(0, "getPid");
    CHECK(ret >= 0, lblKO);

    pid = getPid();
    bin2hex(buf, &pid, 1);
    ret = dspLS(1, buf);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    return 1;
lblKO:
	return -1;
}

static int testGetAppName(void){
    int ret;
    char appName[16 + 1];
	
    dspClear();
    ret = dspLS(0, "getAppName");
    CHECK(ret >= 0, lblKO);

    ret = getAppName(appName);
    CHECK(ret >= 0, lblKO);
    ret = dspLS(1, appName);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    return 1;
lblKO:
	return -1;
}
*/
/*
static int testGetDateTime(char *dateTime) {
    int ret;

    dspClear();
    ret = dspLS(0, "getDateTime");
    CHECK(ret >= 0, lblKO);
    ret = prtS("getDateTime...");
    CHECK(ret >= 0, lblKO);

    ret = getDateTime(dateTime);
    CHECK(ret >= 0, lblKO);
    ret = dspLS(1, dateTime);
    CHECK(ret >= 0, lblKO);
    ret = prtS(dateTime);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    return 1;
  lblKO:
    return -1;
}

static int testSetDateTime(const char *dateTime) {
    int ret;

    ret = dspLS(0, "setDateTime");
    CHECK(ret >= 0, lblKO);
    ret = prtS("setDateTime...");
    CHECK(ret >= 0, lblKO);

    ret = setDateTime("041125163010");
    CHECK(ret >= 0, lblKO);
    ret = dspLS(2, "041125163010");
    CHECK(ret >= 0, lblKO);
    ret = prtS("041125163010");
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = setDateTime(dateTime);
    CHECK(ret >= 0, lblKO);
    ret = dspLS(3, dateTime);
    CHECK(ret >= 0, lblKO);
    ret = prtS(dateTime);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    return 1;
  lblKO:
    return -1;
}
*/
/*
static int testGetSapSer(void){
    int ret;
	char trmSerNo[12 + 1];
    char trmSap[16 + 1];
	
    dspClear();
    ret = dspLS(0, "getSapSer");
    CHECK(ret >= 0, lblKO);

    ret = getSapSer(trmSap, trmSerNo, 't');
    CHECK(ret >= 0, lblKO);
    ret = dspLS(1, trmSerNo);
    CHECK(ret >= 0, lblKO);
    ret = dspLS(2, trmSap);
    CHECK(ret >= 0, lblKO);
    tmrPause(3);

    return 1;
lblKO:
	return -1;
}
*/

/*
static int testGetTS(void){
    int ret;
	char dateTime[12 + 1];
    char buf[dspW + 1];
    byte idx;
	
    *dateTime= 0;

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "Time stamps c:");
    CHECK(ret >= 0, lblKO);
    for (idx = 0; idx < 5; idx++) {
        ret = getTS('c');
        CHECK(ret >= 0, lblKO);
        num2dec(buf, ret, 0);
        ret = dspLS(1, buf);
        CHECK(ret >= 0, lblKO);
        tmrPause(1);
    }

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "Time stamps C:");
    CHECK(ret >= 0, lblKO);
    for (idx = 0; idx < 5; idx++) {
        ret = getTS('C');
        CHECK(ret >= 0, lblKO);
        num2dec(buf, ret, 0);
        ret = dspLS(1, buf);
        CHECK(ret >= 0, lblKO);
        tmrPause(1);
    }
    
    ret = getDateTime(dateTime);
    CHECK(ret >= 0, lblKO);

    ret = setDateTime("241019000000");
    CHECK(ret >= 0, lblKO);

    strcpy(buf,"c:");
    ret = getTS('c');
    CHECK(ret >= 0, lblKO);
    num2dec(buf+2, ret, 0);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    strcpy(buf,"C:");
    ret = getTS('C');
    CHECK(ret >= 0, lblKO);
    num2dec(buf+2, ret, 0);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    strcpy(buf,"s:");
    ret = getTS('s');
    CHECK(ret >= 0, lblKO);
    num2dec(buf+2, ret, 0);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    strcpy(buf,"S:");
    ret = getTS('S');
    CHECK(ret >= 0, lblKO);
    num2dec(buf+2, ret, 0);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    strcpy(buf,"m:");
    ret = getTS('m');
    CHECK(ret >= 0, lblKO);
    num2dec(buf+2, ret, 0);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    strcpy(buf,"M:");
    ret = getTS('M');
    CHECK(ret >= 0, lblKO);
    num2dec(buf+2, ret, 0);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    strcpy(buf,"h:");
    ret = getTS('h');
    CHECK(ret >= 0, lblKO);
    num2dec(buf+2, ret, 0);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    strcpy(buf,"H:");
    ret = getTS('H');
    CHECK(ret >= 0, lblKO);
    num2dec(buf+2, ret, 0);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    strcpy(buf,"d:");
    ret = getTS('d');
    CHECK(ret >= 0, lblKO);
    num2dec(buf+2, ret, 0);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    strcpy(buf,"D:");
    ret = getTS('D');
    CHECK(ret >= 0, lblKO);
    num2dec(buf+2, ret, 0);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    strcpy(buf,"w:");
    ret = getTS('w');
    CHECK(ret >= 0, lblKO);
    num2dec(buf+2, ret, 0);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    strcpy(buf,"W:");
    ret = getTS('W');
    CHECK(ret >= 0, lblKO);
    num2dec(buf+2, ret, 0);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    ret= 1;
    goto lblEnd;
lblKO:
	ret= -1;
    goto lblEnd;
lblEnd:
    setDateTime(dateTime);
    return ret;
}
*/

void tcvp0016(void) {
    int ret;
    char dateTime[12 + 1];
    char dateTime1[12 + 1];

    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    /*
       ret = testGetPid();
       CHECK(ret >= 0, lblKO); */

    /*
       ret = testGetAppName();
       CHECK(ret >= 0, lblKO); */

    dspClear();
    ret = dspLS(0, "getDateTime");
    CHECK(ret >= 0, lblKO);
    ret = prtS("getDateTime...");
    CHECK(ret >= 0, lblKO);

    ret = getDateTime(dateTime);
    CHECK(ret >= 0, lblKO);
    ret = dspLS(1, dateTime);
    CHECK(ret >= 0, lblKO);
    ret = prtS(dateTime);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspLS(0, "setDateTime");
    CHECK(ret >= 0, lblKO);
    ret = prtS("setDateTime...");
    CHECK(ret >= 0, lblKO);

    ret = setDateTime("041125163010");
    CHECK(ret >= 0, lblKO);
    ret = dspLS(1, "041125163010");
    CHECK(ret >= 0, lblKO);
    ret = prtS("041125163010");
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspLS(2, "getDateTime");
    CHECK(ret >= 0, lblKO);
    ret = prtS("getDateTime...");
    CHECK(ret >= 0, lblKO);

    ret = getDateTime(dateTime1);
    CHECK(ret >= 0, lblKO);
    ret = dspLS(3, dateTime1);
    CHECK(ret >= 0, lblKO);
    ret = prtS(dateTime1);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    dspClear();
    ret = dspLS(0, "setDateTime");
    CHECK(ret >= 0, lblKO);
    ret = prtS("setDateTime...");
    CHECK(ret >= 0, lblKO);

    ret = setDateTime(dateTime);
    CHECK(ret >= 0, lblKO);
    ret = dspLS(1, dateTime);
    CHECK(ret >= 0, lblKO);
    ret = prtS(dateTime);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspLS(2, "getDateTime");
    CHECK(ret >= 0, lblKO);
    ret = prtS("getDateTime...");
    CHECK(ret >= 0, lblKO);

    ret = getDateTime(dateTime);
    CHECK(ret >= 0, lblKO);
    ret = dspLS(3, dateTime);
    CHECK(ret >= 0, lblKO);
    ret = prtS(dateTime);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    /*
       ret = testGetSapSer();
       CHECK(ret >= 0, lblKO); */

    /*
       ret = testGetTS();
       CHECK(ret >= 0, lblKO); */

    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
  lblEnd:
    tmrPause(3);
    dspStop();
    prtStop();
}
