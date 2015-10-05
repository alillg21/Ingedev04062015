#include <string.h>
#include "sys.h"
#include "tst.h"
#include <unicapt.h>

//File tcbt0003.c

/** Unitary test case for sys/I32 component: Difference between 2 dates in seconds**/

void tcbt0003(void)
{
    int ret;
    int64 interval;
    int noOfdays;
    char day[10]="";

    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    dspClear();
    ret= dspLS(0,"Jan 20 2008");//Jan 20 2008
    CHECK(ret >= 0, lblKO);
    ret= dspLS(1,"Jan 30 2008");//Jan 30 2008
    CHECK(ret >= 0, lblKO);
    ret= diffSecBetweenTwoDates("080120120030","0801301700",&interval);
    CHECK(ret>0,lblKO);
    noOfdays= interval/86400;
    ret= num2dec(day,noOfdays,0);
    CHECK(ret>0,lblKO);
    ret= dspLS(2, day);
    CHECK(ret >= 0, lblKO);
    tmrPause(2);

    dspClear();
    ret= dspLS(0,"Jan 01 2007");//Jan 01 2007
    CHECK(ret >= 0, lblKO);
    ret= dspLS(1,"Jan 01 2008");//Jan 01 2008
    CHECK(ret >= 0, lblKO);
    ret= diffSecBetweenTwoDates("0701011200","0801011200",&interval);
    CHECK(ret>0,lblKO);
    noOfdays= interval/86400;
    ret= num2dec(day,noOfdays,0);CHECK(ret>0,lblKO);
    ret= dspLS(2, day);
    CHECK(ret >= 0, lblKO);
    tmrPause(2);

    dspClear();
    ret= dspLS(0,"Feb 01 2008");//Feb 01 2008
    CHECK(ret >= 0, lblKO);
    ret= dspLS(1,"Feb 29 2008");//Feb 29 2008
    CHECK(ret >= 0, lblKO);
    ret= diffSecBetweenTwoDates("0802011200","0802291200",&interval);
    CHECK(ret>0,lblKO);
    noOfdays= interval/86400;
    ret= num2dec(day,noOfdays,0);CHECK(ret>0,lblKO);
    ret= dspLS(2, day);
    CHECK(ret >= 0, lblKO);
    tmrPause(2);

    dspClear();
    ret= dspLS(0,"Feb 01 2008");//Feb 01 2008
    CHECK(ret >= 0, lblKO);
    ret= dspLS(1,"Feb 30 2008");//Feb 30 2008
    CHECK(ret >= 0, lblKO);
    ret= diffSecBetweenTwoDates("0802011200","0802301200",&interval);
    CHECK(ret>0,lblKO);
    noOfdays= interval/86400;
    ret= num2dec(day,noOfdays,0);CHECK(ret>0,lblKO);
    ret= dspLS(2, day);
    CHECK(ret >= 0, lblKO);
    tmrPause(2);

    goto lblEnd;
lblKO:
	trcErr(ret);
	dspLS(2,"KO");
lblEnd:
    tmrPause(1);
    dspStop();
}
