#include <string.h>
#include "pri.h"
#include "tst.h"
#include "sys.h"

static code const char *srcFile = __FILE__;

#define CHK CHECK(ret>=0,lblKO)

//Unitary test case tcfa0003
//Function testing:

//  enterCard: Modifying on this function.

void tcfa0003(void) {
    int ret;
	char buf[128*3]="";
	int mag;
	int icc;	     

    ret = dspLS(0, "Insert a card...");

	ret= enterCard((byte *)buf,"kmc",&mag,&icc); 

	if(mag < 0)
	{
		//Magnatic reader has problem.
		ret = dspLS(0, "Mag reader problem.");
		CHECK(ret >= 0, lblKO);
	}
		
	if(icc < 0)
	{
		//Chip card reader has problem.
		ret = dspLS(0, "Chip reader problem.");
		CHECK(ret >= 0, lblKO);
	}
	

	if(ret < 0)
	{
		//problem but magnatic and chip card reader are ok.
		ret = dspLS(0, "Problem in reading");
		CHECK(ret >= 0, lblKO);
	}		
    goto lblEnd;
  lblKO:
    dspLS(0, "Error");	
  lblEnd:
	dspStop();
    ;
}
