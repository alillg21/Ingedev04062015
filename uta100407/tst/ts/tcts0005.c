#include <string.h>
#include "sys.h"
#include "tst.h"

static code const char *srcFile = __FILE__;

//Unitary test case tcts0005 based on tcab0074.
//Functions testing for GCL communication ISDN modem:
//  utaGclStart

void tcts0005(void) {
    int ret;
    int idx;
    byte b;

    //char *ptr;
    char msg[dspW + 1];

    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    memset(msg, 0, dspW + 1);
    ret = utaGclStart (gclISDN, "003614212700"); // Give phone number here This is pad in Hungary
    CHECK(ret >= 0, lblKO);

    ret = utaGclDial();
    CHECK(ret >= 0, lblKO);

	ret= utaGclDialWait();
    CHECK(ret >= 0, lblKO);


    for (idx = 0; idx < 128; idx++) {
        ret = utaGclRecv(&b, 20);   //receive characters
        if(ret < 0)
            break;
        if('A' <= b && b <= 'Z') {
            msg[idx % dspW] = (char) b; //display ascii characters
            dspLS(2, msg);
        }
    }
    ret = dspLS(3, "Receiving done");
    CHECK(ret >= 0, lblKO);

    ret = utaGclStop();
    CHECK(ret >= 0, lblKO);

    tmrPause(3);
    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO!");
    tmrPause(3);
  lblEnd:
    tmrPause(2);
    comStop();
    dspStop();
}
