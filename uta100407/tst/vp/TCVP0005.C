#include <string.h>
#include <stdio.h>
#include "pri.h"
#include "tst.h"

//Unitary test case tcvp0005.
//comChn3 test for receiving

void tcvp0005(void) {
    int ret;
    byte Counter;
    char buffer[400];
    byte rxData[10];

    memset(rxData, 0, sizeof(rxData));

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "comStart...");
    CHECK(ret >= 0, lblKO);
    ret = comStart(chnCom3);
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "comStart OK");

    ret = dspLS(1, "comSet...");
    CHECK(ret >= 0, lblKO);
    ret = comSet("8N119200");
    CHECK(ret >= 0, lblKO);
    ret = dspLS(1, "comSet OK");

    ret = dspLS(0, "RECEIVE procedure...");
    CHECK(ret >= 0, lblKO);

    memset(buffer, 0, sizeof(buffer));
    Counter = 0;
    while(Counter != 26) {
        sprintf(buffer, "%2d bytes rec.", Counter);
        dspXYS(0, 3, buffer);

        ret = comRecv(rxData, 20);
        CHECK(ret >= 0, lblKO);
        if(Counter < 16) {
            sprintf(buffer, "%s", rxData);
            dspXYS(Counter, 1, buffer);
        } else {
            sprintf(buffer, "%s", rxData);
            dspXYS((byte) (Counter - 16), 2, buffer);
        }

        ++Counter;

        sprintf(buffer, "%2d bytes rec.", (Counter));
        dspXYS(0, 3, buffer);

        sprintf(buffer, "%s", rxData);
        ret = prtS(buffer);
        CHECK(ret >= 0, lblKO);
    }

    sprintf(buffer, "%2d bytes rec.", Counter);
    ret = prtS(buffer);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(3, "Press a key...");
    ret = kbdStart(1);
    CHECK(ret >= 0, lblKO);
    ret = tmrStart(0, 5 * 100);
    while(tmrGet(0)) {
        ret = kbdKey();         //retrieve a key if pressed
        if(ret)
            break;              //quit the loop if a key pressed
    }
    ret = kbdStop();            //stop keyboard waiting

    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
  lblEnd:
    comStop();
    tmrPause(3);
    dspStop();
    prtStop();
}
