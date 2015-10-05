#include <string.h>
#include <stdio.h>
#include "sys.h"
#include "tst.h"

//Unitary test case tcvp0004.
//comChn3 test for sending

void tcvp0004(void) {
    int ret;
    char txData[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    byte Counter;
    char Char;
    char buffer[400];

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

    while(1) {
        ret = dspLS(0, "SEND procedure...");
        CHECK(ret >= 0, lblKO);

        memset(buffer, 0, sizeof(buffer));
        for (Counter = 0; Counter != sizeof(txData); ++Counter) {
            Char = txData[Counter];
            ret = comSend(Char);
            CHECK(ret >= 0, lblKO);

            sprintf(buffer, "Bytes sent : %d ", Counter + 1);
            dspXYS(0, 0, buffer);

            if(Counter < 16) {
                sprintf(buffer, "%c", Char);
                dspXYS(Counter, 1, buffer);
            } else {
                sprintf(buffer, "%c", Char);
                dspXYS((byte) (Counter - 16), 2, buffer);
            }
        }

        sprintf(buffer, "%s", &txData[0]);
        ret = prtS(buffer);     //print a string
        CHECK(ret >= 0, lblKO);
        sprintf(buffer, "Bytes sent : %d ", Counter);
        ret = prtS(buffer);     //print a string
        CHECK(ret >= 0, lblKO);

        ret = dspLS(3, "Press a key...");
        ret = kbdStart(1);      //start keyboard waiting
        CHECK(ret >= 0, lblKO);

        ret = tmrStart(0, 5 * 100);
        while(tmrGet(0)) {
            ret = kbdKey();     //retrieve a key if pressed
            if(ret)
                break;          //quit the loop if a key pressed
        }
        ret = kbdStop();        //stop keyboard waiting
    }
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
