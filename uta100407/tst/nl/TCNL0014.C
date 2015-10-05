#include <string.h>
#include "log.h"
#include "tst.h"
#include "err.h"

#ifdef __TEST__
static code const char *srcFile = __FILE__;

//Integration test case tcnl0014.
//Encrypt key with a masterkey is located in special loc

#define CHK CHECK(ret>=0,lblKO)

void tcnl0014(void) {
    int ret;
    byte key[8] = "";
    byte loc = 0x00;
    char buf[35] = "";

    loc = 0x00;
    trcS("Encrype Master key: Beg\n");
    ret = cryStart('m');
    CHECK(ret >= 0, lblCry);
    memcpy(key, "\x11\x11\x11\x11\x11\x11\x11\x11", 8);
    dspStop();                  //close channel to give cryptomodule access to HMI
    ret = cryEncryptWithDL(loc, key);
    bin2hex(buf, key, 8);

    trcFS("Encrypted Master key: =%s\n", buf);
    trcS("What we expect is 4F58207DA7FE1201: \n");

    if(ret > 0)
        prtS("0x00\n\n");
    else
        prtS("\n\n");

    CHECK(ret >= 0, lblCry);
    ret = dspStart();
    RTCHK;

    ret = 1;
    goto lblEnd;

    cryStop('m');
    dspStart();
    ret = 1;
    goto lblEnd;
  lblRT:
    ret = -runTimeExp;
    trcErr(ret);
    goto lblEnd;
  lblCry:                      //crypto peripheral error encountered
    ret = -cryptoExp;
    dspStart();                 //restart in case if it was stopped; the return code is not checked intentionnally
    trcErr(ret);
    goto lblEnd;
  lblEnd:
    cryStop('m');               //stop in case if it was started; the return code is not checked intentionnally       
    trcFN("Encrype Master key: ret=%d\n", ret);
}
#endif
