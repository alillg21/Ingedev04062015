#include "sys.h"
#include "tst.h"

//Unitary test case tcik0071.
//Functions testing:
//  prtStart
//  prtStop
//  prtS
//  prtCtrl

void tcik0071(void) {
    int ret;

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();           //open the associated channel
    CHECK(ret >= 0, lblKO);

    ret = prtCtrl();

    if(ret < 0)
        dspLS(0, "Paper is over");
    else
        dspLS(0, "Paper is present");
	tmrPause(2);
	
	CHECK(ret >= 0, lblKO); //do not print if no paper

    ret = prtS("Short string"); //print a string
    CHECK(ret >= 0, lblKO);

    tmrPause(2);

    goto lblEnd;
  lblKO:
    trcErr(ret);
    dspLS(0, "KO!");
    tmrPause(2);
  lblEnd:
    prtStop();                  //close resource
    dspStop();
}
