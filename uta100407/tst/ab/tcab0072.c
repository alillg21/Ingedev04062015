#include "sys.h"
#include "tst.h"
#include "unicapt.h"

//Unitary test case tcab0072.
//Functions testing:
//  trcMode
//  trcS

void tcab0072(void) {
#ifdef __TEST__
    int ret;

    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    trcMode(0);                 //turn it off

    ret = prtS("It is not traced");
    CHECK(ret >= 0, lblKO);

#ifdef __DEBUG__
    ret = prtS("Trace to file");
    CHECK(ret >= 0, lblKO);
    trcMode(0xFF);              //turn it on
#else
    ret = prtS("Trace via COM1");
    CHECK(ret >= 0, lblKO);
    trcMode(1);                 //comport is used
#endif

    ret = prtS("It is traced now");
    CHECK(ret >= 0, lblKO);

    goto lblEnd;

  lblKO:
    trcErr(ret);
  lblEnd:
    prtStop();                  //close resource
    trcMode(0);
#ifdef __DEBUG__
    trcMode(0xFF);              //turn it on
#endif
#endif
}
