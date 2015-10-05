//
// Unitary test case tctd0003.
//
// author:      Tamas Dani
// date:        24.08.2005
// description: test for functions ppdSap,ppdClear,ppdLS
//

#include "sys.h"


void tctd0003( void ){
    int ret;
//    char sap[16+1];

    ret = cryStart('m');
    CHECK(ret>=0,lblCry);

//    ret = ppdSap(sap);
//    CHECK(ret>=0,lblCry);
//    prtStart();
//    prtS("SAP code:");
//    prtS(sap);
//    prtStop();

    ret = ppdClear();
    CHECK(ret>=0,lblCry);
    tmrPause(1);

    ret = ppdLS(0, " first line ");
    CHECK(ret>=0,lblCry);
    tmrPause(1);

    ret = ppdLS(1, " second line ");
    CHECK(ret>=0,lblCry);
    tmrPause(3);

    ret = ppdClear();
    CHECK(ret>=0,lblCry);

    goto lblEnd;

lblCry:
    trcErr(ret);
lblEnd:
    cryStop('m');
}
