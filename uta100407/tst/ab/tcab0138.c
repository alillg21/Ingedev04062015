/** \file
 * Unitary test case tcab0138.
 * Functions testing:
 * \sa
 *  - tftStart()
 *  - tftStop()
 *  - tftKey()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0138.c $
 *
 * $Id: tcab0138.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "sys.h"
#include "tst.h"

void tcab0138(void) {
    int ret;
    char buf[dspW + 1];
    char tmp[dspW + 1];
    word x, y;
    char key;
    char sta = 0;
    byte dly;

    trcS("tcab0138 Beg\n");

    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    x = 0;
    y = 0;

    ret = dspLS(0, "Touch the screen...");
    CHECK(ret >= 0, lblKO);

    memset(buf, 0, dspW + 1);
    while(42) {
        ret = tmrStart(0, 50);
        CHECK(ret >= 0, lblKO);

        ret = kbdStart(1);      //start keyboard waiting
        CHECK(ret >= 0, lblKO);

        ret = tftStart();       //start touch waiting
        CHECK(ret >= 0, lblKO);

        key = 0;
        while(tmrGet(0)) {
            ret = tftGet();     //get touch coordinates if touched
            CHECK(ret != -1, lblKO);    //error during touch waiting
            x = HWORD(ret);
            y = LWORD(ret);
            if(x)
                break;
            if(y)
                break;

            key = kbdKey();     //retrieve a key if pressed
            CHECK(key != '\xFF', lblKO);    //error during key waiting

            if(key)
                break;
        }

        if(key == kbdANN)
            break;

        ret = kbdStop();        //stop keyboard waiting
        CHECK(ret >= 0, lblKO);

        ret = tftStop();        //stop touch waiting
        CHECK(ret >= 0, lblKO);

        tmrStop(0);

        dly = 0;
        switch (tftState()) {
          case 'u':
              if(sta != 'u') {
                  ret = dspLS(3, "UP");
                  trcS("UP\n");
                  dly = 1;
              }
              sta = 'u';
              break;
          case 'd':
              if(sta != 'd') {
                  ret = dspLS(3, "DN");
                  trcS("DN\n");
                  dly = 1;
              }
              sta = 'd';
              break;
          default:
              dspClear();
              ret = dspLS(3, "NO");
              trcS("NO\n");
              sta = 0;
              break;
        }
        CHECK(ret >= 0, lblKO);

        if(!(x + y))
            continue;

        num2dec(tmp, x, 0);
        strcpy(buf, "X: ");
        strcat(buf, tmp);
        ret = dspLS(1, buf);
        CHECK(ret >= 0, lblKO);

        num2dec(tmp, y, 0);
        strcpy(buf, "Y: ");
        strcat(buf, tmp);
        ret = dspLS(2, buf);
        CHECK(ret >= 0, lblKO);

        if(dly)
            tmrSleep(100);
    }
    goto lblEnd;

  lblKO:
    trcErr(ret);
  lblEnd:                      //close resource
    tftStop();
    kbdStop();
    dspStop();
    trcS("tcab0138 End\n");
}
