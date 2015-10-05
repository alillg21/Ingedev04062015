/** \file
 * Unitary test case for timestamp to datetime and datetime to timestamp conversion.
 * \sa
 *  - int dt2ts(char *YYMMDDhhmmss)
 *  - int ts2dt(int timestamp, char *YYMMDDhhmmss)
 *
 * Computations based on http://www.epochconverter.com/
 * Human readable time  Seconds 
 *  - 1 minute 60 seconds 
 *  - 1 hour 3600 seconds 
 *  - 1 day 86400 seconds 
 *  - 1 week 604800 seconds 
 *  - 1 month (30.44 days)  2629743 seconds 
 *  - 1 year (365.24 days)   31556926 seconds 
 */

#include <string.h>
#include <stdio.h>
#include "pri.h"
#include "tst.h"

static int test(card ts,const char *dt,const char *msg1,const char *msg2){
    char YYMMDDhhmmss[12+1];
    card timeStamp;
    char tmp[prtW+1];
    int ret;
	
    memset(tmp, 0, sizeof(tmp));
    dspLS(0,msg1); 		
    dspLS(1,msg2); 		

    //Timestamp to DateTime
    ts2dt(ts, YYMMDDhhmmss);
    dspLS(2, YYMMDDhhmmss);
    CHECK(strcmp(YYMMDDhhmmss,dt)==0,lblKO);
	
    //DateTime to Timestamp
    timeStamp = dt2ts(dt);       
    sprintf(tmp, "%d", timeStamp);			
    dspLS(3, tmp);
    CHECK(timeStamp == ts, lblKO);
    tmrPause(1);	

    prtS(tmp);
    prtS(YYMMDDhhmmss);
    strcpy(tmp,msg1);
    strcat(tmp," ");
    strcat(tmp,msg2);
    prtS(tmp);
    prtS(" ");		

    ret= 1;
    goto lblEnd;
  lblKO:
    dspLS(1,"KO!");
    ret= -1;
  lblEnd:
  	return ret;
}

void tcmp0000(void) {
    int ret;
    trcS("tcmp0000 Beg\n");

    ret= prtStart();
    CHECK(ret >= 0, lblKO);
    ret= dspStart();
    CHECK(ret >= 0, lblKO);

    ret= test(1204406093,"080301211453","01 Mar 08","21:14:53");
    CHECK(ret >= 0, lblKO);
    ret= test(1204319693,"080229211453","29 Feb 08","21:14:53");
    CHECK(ret >= 0, lblKO);
    ret= test(1204233293,"080228211453","28 Feb 08","21:14:53");
    CHECK(ret >= 0, lblKO);
    ret= test(1222809293,"080930211453","30 Sep 08","21:14:53");
    CHECK(ret >= 0, lblKO);
    ret= test(1222285873,"080924195113","24 Sep 08","19:51:13");
    CHECK(ret >= 0, lblKO);
    ret= test(180783990,"750924094630","24 Sep 75","09:46:30");
    CHECK(ret >= 0, lblKO);
    ret= test(654169590,"900924094630","24 Sep 90","09:46:30");
    CHECK(ret >= 0, lblKO);
    ret= test(906630390,"980924094630","24 Sep 98","09:46:30");
    CHECK(ret >= 0, lblKO);
    ret= test(969788790,"000924094630","24 Sep 00","09:46:30");
    CHECK(ret >= 0, lblKO);
    ret= test(1222249590,"080924094630","24 Sep 08","09:46:30");
    CHECK(ret >= 0, lblKO);
    ret= test(1443087990,"150924094630","24 Sep 15","09:46:30");
    CHECK(ret >= 0, lblKO);
    ret= test(1600940790,"200924094630","24 Sep 20","09:46:30");
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    ret= dspLS(1,"Conv KO!"); CHECK(ret>=0,lblKO);
    trcErr(ret);
  lblEnd:
    prtStop();
    dspStop();
    tmrPause(3);	
    trcS("tcmp0000 End\n");
}
