#include "pri.h"
#include "tc.h"

//Unitary test case tcjd0003.
//Functions testing:
//  ecrOnLineSale
//  ecrCloseOnLine

void tcjd0003(void)
{
	int ret;
	char *fnc = "CloseOnLine";
	char *info = "Finish sale manually on ECR";
	
	tcDspTitle(fnc, "");

	ret = ecrStart();
    CHECK(ret == 1, lblKO);

	ret = ecrOnLineSale(9, 1);
	CHECK(ret == 1, lblKO);
 
	ret = ecrCloseOnLine();
	CHECK(ret == 1, lblKO);
	
	tcPrt(info);
	tcDspS(" OK");
	tcDspS(info);
	
    goto lblEnd;
	 
  lblKO:
	tcDspFN("KO! (NACK = %d)", ecrGetAck());
	tcPrt((char *) 0);
	
  lblEnd:
  	ecrStop();
}
