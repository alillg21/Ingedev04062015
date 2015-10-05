/** \file
 * Unitary test case tcik0177.
 * Functions testing:
 * \sa
 *  - cryDukptInit()
 *  - cryDukptPinInput()
 */

#include <string.h>
#include "sys.h"
#include "tst.h"

#ifdef __DUKPT__

void tcik0177(void) {
    int ret;
    byte crt[4];
    char tmp[prtW + 1];
    byte acc[8];
    byte blk[8];
    byte loc;
    byte trn;
    byte InitSNKey[20];         // InitSNKey = KET SET ID (FFFF) + TRMS ID (9876543210) + TRANSACTION COUNTER (E00000)
    byte SNKey[10];             // SNKey return at pin = KET SET ID + TRMS ID + TRANSACTION COUNTER
    byte i;
    byte len;

    memset(tmp, 0, prtW + 1);
    memset(crt, 0, 4);

    tcab0021();                 //test key downloading

    //open ressources
    ret = prtStart();
    CHECK(ret >= 0, lblKO);
    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = cryStart('m');
    CHECK(ret >= 0, lblKO);
    
    len = 3;

    //verify that the transport key is downloaded
#ifdef __TELIUM__
    loc = 1008 / 8;             //1024-16
#endif
#ifdef __UNICAPT__
    loc = 0x10;
#endif
    //verify that the transport key 1111111111111111 is downloaded at array 1 slot 0
    ret = cryVerify(loc, crt);
    CHECK(ret >= 0, lblKO);
    CHECK(memcmp(crt, "\x82\xE1\x36\x65", len) == 0, lblKO);    //it is the certificate of the key 1111111111111111

#ifdef __TELIUM__
    loc = 8 / 8;
    trn = 1008 / 8;
    //	derivation key
    //  clear key  "\x01\x23\x45\x67\x89\xAB\xCD\xEF\xFE\xDC\xBA\x98\x76\x54\x32\x10"
    //  key encoded key with Root key
    byte *wrk =
        (byte *)
        "\x8A\x5A\xE1\xF8\x1A\xB8\xF2\xDD\x69\x7C\x55\xFA\xAC\x85\xAB\xC0";
#endif
#ifdef __UNICAPT__
    loc = 0x24;
    trn = 0x10;
    byte *wrk =
        (byte *)
        "\xFC\xAC\x1A\x1B\x0B\x12\x91\x3D\x00\x00\x00\x00\x00\x00\x00\x00";
#endif

    ret = cryLoadDKuDK(loc, trn, wrk, 'd'); //download derivation key into loc using transport key located at 1008
    CHECK(ret >= 0, lblKO);

    //*** Initialize DUKPT process ***
#ifdef __TELIUM__
    loc = 8 / 8;
    memcpy(InitSNKey, "\xFF\xFF\x98\x76\x54\x32\x10\xE0", 8);   // without Padding with 0x00
#endif
#ifdef __UNICAPT__
    loc = 0x24;
    memcpy(InitSNKey, "\x3F\x3F\x3F\x3F\x39\x38\x37\x36\x35\x34\x33\x32\x31\x30\x3E\x30\x30\x30\x30\x30", 20);  // Padding with 0x30
#endif

    ret = cryDukptInit(loc, InitSNKey);
    CHECK(ret >= 0, lblKO);

    ret = dspStop();            //close channel to give cryptomodule access to HMI
    CHECK(ret >= 0, lblKO);

    //*** First Pin entry and get pinblock ***
    memcpy(acc, "\x00\x00\x40\x12\x34\x56\x78\x90", 8);
    ret = cryDukptGetPin("AMOUNT= 1000 USD", "CODE:", acc, 60, SNKey, blk); //enter 1234 to obtain a known result
    CHECK(ret >= 0, lblKO);

    ret = dspStart();           //now we can open HMI again
    CHECK(ret >= 0, lblKO);

    //present results
    ret = bin2hex(tmp, SNKey, 10);
    CHECK(ret == 20, lblKO);    //SNKey
    ret = prtS(tmp);
    CHECK(ret >= 0, lblKO);
    memset(tmp, 0, prtW + 1);
    ret = bin2hex(tmp, blk, 8);
    CHECK(ret == 16, lblKO);    //Pinblock
    ret = prtS(tmp);
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, tmp);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    for (i = 2; i < 9; i++) {

        ret = dspStop();        //close channel to give cryptomodule access to HMI
        CHECK(ret >= 0, lblKO);

        //*** Next Pin entry and get pinblock ***
        ret = cryDukptGetPin("AMOUNT= 1000 USD", "CODE:", acc, 60, SNKey, blk); //enter 1234 to obtain a known result
        CHECK(ret >= 0, lblKO);

        ret = dspStart();       //now we can open HMI again
        CHECK(ret >= 0, lblKO);

        //present results
        ret = bin2hex(tmp, SNKey, 10);
        CHECK(ret == 20, lblKO);    //SNKey
        ret = prtS(tmp);
        CHECK(ret >= 0, lblKO);
        memset(tmp, 0, prtW + 1);
        ret = bin2hex(tmp, blk, 8);
        CHECK(ret == 16, lblKO);    //Pinblock
        ret = prtS(tmp);
        CHECK(ret >= 0, lblKO);
        ret = dspLS(0, tmp);
        CHECK(ret >= 0, lblKO);
        tmrPause(1);

        switch (SNKey[9]) {
          case 3:
              CHECK(memcmp(blk, "\x92\x5B\xC2\xA3\x96\x52\xCF\x75", 8) == 0, lblKO);    //it is the pinblock if the pin is 1234
              break;
          default:
              break;
        }

    }
    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO!");
  lblEnd:
    tmrPause(3);
    cryStop('m');
    prtStop();
    dspStop();                  //close resources
}
#else
void tcik0177(void) {
}
#endif
