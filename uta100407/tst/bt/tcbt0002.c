/** \file
 * Unitary test case tcbt0001.
 * Functions testing:
 * Calculating No:of days
 * \sa
 *  - getTS()
 */

#include <string.h>
#include <stdio.h>
#include "sys.h"
#include "tst.h"



void tcbt0002(void)
{
    int ret1,ret2,ret;
	int timetemp;
	char day[dspW + 1]="";
	char c[20]="";
	char d[20]="";

    trcS("tcbt0002 Beg\n");
  /*Nov 17th to Dec 9th*/
	ret= dspStart(); CHECK(ret>=0,lblErr);
	ret= prtStart(); CHECK(ret>=0,lblKO);
	dspClear();
  	ret1 = setDateTime("071117120000");
  	getDateTime(c);
  	prtS(c);
    ret1 = getTS(0);
    sprintf(d,"ret1 %d" ,ret1);
    prtS(d);
    ret2 = setDateTime("071209120000");
	getDateTime(c);
    prtS(c);
    ret2 = getTS(0);// Here ret2 gives a value that is greater than ret1 which is correct
    sprintf(d,"ret2 %d" ,ret2);
    prtS(d);
    ret=ret2-ret1;
    sprintf(d,"ret2-ret1 %d" ,ret);
    prtS(d);
    timetemp=ret/8640000;// It shd give 22days.....
    sprintf(d,"timetemp %d" ,timetemp);
    prtS(d);
    ret= num2dec(day,timetemp,0);
    CHECK(ret>=0,lblKO);
    sprintf(d,"result %s" ,day);
    prtS(d);
    dspClear();
  	dspLS(1, "Days");
	dspLS(2, day);
	tmrPause(2);

  /*Nov 17th to dec 10th*/
    dspClear(); 
    ret1 = setDateTime("071117120000");
    ret1 = getTS(0);
     sprintf(d,"ret1 %ld" ,ret1);
    prtS(d);
    ret2 = setDateTime("071210120000");
	getDateTime(c);
    ret2 = getTS(0); // Here the value of ret2 should be greater than ret1, but in this case its giving a value less than ret1.
    sprintf(d,"ret2 %ld" ,ret2);
    prtS(d);
    CHECK(ret2>=ret1 ,lblErr);
    ret=ret2-ret1; // Since ret2 is less tahn ret1 it gives a negative value.
    sprintf(d,"ret2-ret1 %ld" ,ret);
    prtS(d);
    timetemp=ret/8640000;// It shd give 23 days.....
    ret= num2dec(day,timetemp,0);
    CHECK(ret>=0,lblKO);
    sprintf(d,"result %s" ,day);
    prtS(d);
    CHECK(strcmp(day, "23")== 0, lblErr);
    dspClear();
  	dspLS(1, "Days");
	dspLS(2, day);
	prtS("OK");
	tmrPause(2);

    goto lblEnd;
lblKO:
    trcErr(ret);
    dspLS(0,"KO!");
    goto lblEnd;
lblErr:
	dspLS(0, "ERROR");
 	goto lblEnd;
lblEnd:
    tmrPause(3);
    prtStop();
    dspStop();
    trcS("tcbt0002 End\n");
}
