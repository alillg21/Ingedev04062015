/** \file
 * Unitary test case tcab0023.
 * Functions testing:
 * \sa
 *  - cryGetPin()
 *  - crySetPpd() //Not implemented for TELIUM
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0023.c $
 *
 * $Id: tcab0023.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "sys.h"
#include "tst.h"

void tcab0023(void) {
    int ret;
    byte crt[4];
    char tmp[prtW + 1];
    byte acc[8];
    byte blk[8];
    byte loc;
    byte len;

    const char *cfgDft = 0;     //default pinpad configuration
    const char *msgDft = "AMOUNT= 1000 USD\nCODE:\n\n";

    const char *cfgNbd = "0104C1301060";    //NBD-NBK-MB pinpad configuration
    const char *msgNbd = "AMOUNT= 1000 UAE\nPIN\n\n";

    const char *cfgKnet = "0124C3301060";   //K-net pinpad configuration
    const char *msgKnet = "AMOUNT= 1000 QWD\nCustomer PIN\nEntry\n";

    const char *cfgSama = "0104C2301060";   //SAMA pinpad configuration
    const char *msgSama = "AMOUNT= 1000 SAD\nEnter PIN\n \n";

    const char *cfg3010 = "0104C1301060";   //3010 pinpad configuration
    const char *msg3010 = "AMOUNT= 1000 EUR\nPIN:\n\n";

    //const char *cfg4lines= "0124C3301060"; //4 lines pinpad configuration
    //const char *msg4lines= "AMOUNT= 5000 USD\nPlease enter\nPIN CODE:\n";

    const char *cfg4lines = "0124C3301060"; //4 lines pinpad configuration with arab font
    const char *msg4lines =
        "AMOUNT= 5000 USD\n\xB1\xF3\xF8\x93\xEC\xF5\nPIN CODE:\n";

    byte ctl;                   //control byte defining the configuration
    const char *cfg;
    const char *msg;

#ifdef __ICT220__
    len = 3;
#else
    len = 4;
#endif

    trcS("tcab0023 End\n");
    memset(tmp, 0, prtW + 1);
    memset(crt, 0, 4);

    tcab0022();                 //test and working keys downloading

    cfg = cfgDft;
    msg = msgDft;
    ctl = 0;
    switch (ctl) {
      case 1:
          cfg = cfgNbd;
          msg = msgNbd;
          break;
      case 2:
          cfg = cfgKnet;
          msg = msgKnet;
          break;
      case 3:
          cfg = cfgSama;
          msg = msgSama;
          break;
      case 4:
          cfg = cfg3010;
          msg = msg3010;
          break;
      case 5:
          cfg = cfg4lines;
          msg = msg4lines;
          break;
      default:
          cfg = 0;
          break;
    }

    //open ressources
    ret = prtStart();
    CHECK(ret >= 0, lblKO);
    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = cryStart('m');
    CHECK(ret >= 0, lblKO);

#ifdef __TELIUM__
    //verify that the working key 6B218F24DE7DC66C is downloaded at offset 8/8 = 1
    loc = 8 / 8;                //offset
#endif
#ifdef __UNICAPT__
    //verify that the working key 6B218F24DE7DC66C is downloaded at array 2 slot 4
    loc = 0x24;
#endif
    ret = cryVerify(loc, crt);
    CHECK(ret >= 0, lblKO);

    CHECK(memcmp(crt, "\xF7\x53\xC2\x98", len) == 0, lblKO);    //it is the certificate of the key 6B218F24DE7DC66C

    //start testing
    dspStop();                  //close channel to give cryptomodule access to HMI

    //crySetPpd("2314AX301060");
    if(cfg)
        crySetPpd(cfg);
    memcpy(acc, "\x00\x00\x78\x90\x12\x34\x56\x74", 8);
    //ret = cryGetPin("AMOUNT= 1000 USD\nEnter 1234\nCODE:\n", acc, loc, 60, blk);    //enter 1234 to obtain a known result
    //ret = cryGetPin("AMOUNT= 1000 USD\nCODE:\n\n", acc, loc, blk);    //enter 1234 to obtain a known result

    ret = cryGetPin(msg, acc, loc, blk);    //enter 1234 to obtain a known result
    CHECK(ret > 0, lblKO);

    ret = dspStart();           //now we can open HMI again
    CHECK(ret >= 0, lblKO);

    //present results
    ret = bin2hex(tmp, blk, 8);
    CHECK(ret == 16, lblKO);
    ret = prtS(tmp);
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, tmp);
    CHECK(ret >= 0, lblKO);

    CHECK(memcmp(blk, "\x06\xDA\x91\xDF\xF2\x76\xD3\xA8", 8) == 0, lblKO);  //it is the pinblock if the pin is 1234

    tmrPause(1);
    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO!");
    tmrPause(3);
  lblEnd:
    cryStop('m');
    prtStop();
    dspStop();                  //close resources
    trcS("tcab0023 End\n");
}
