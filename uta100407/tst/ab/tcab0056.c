/** \file
 * Unitary test case tcab0056.
 * Functions testing:
 * \sa
 *  - tskStart()
 *  - tskStop()
 *  - semDec()
 *  - semInc()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0056.c $
 *
 * $Id: tcab0056.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "sys.h"
#include "tst.h"

static char data[prtW + 1];     //this variable will be filled by both tasks

static void play(char beg) {
    char buf[dspW + 1];
    word idx;
    byte tmr;
    static char bufM[dspW + 1];
    static char bufS[dspW + 1];

    tmr = (beg == 'A') ? 3 : 2;
    memset(buf, 0, dspW + 1);
    for (idx = 0; idx < dspW * 10; idx++) {
        buf[idx % dspW] = idx / dspW + beg;
        //dspLS(loc, buf); //it is not recommended to use display in both tasks simultaneously
        if(beg == 'A') {
            strcpy(bufM, buf);
            dspLS(2, bufS);
            dspLS(3, bufM);
        } else {
            strcpy(bufS, buf);
            if(idx % dspW == dspW - 1)
                prtS(bufS);
        }
        tskSync();
    }
}

static void testTask(card arg) {
    int ret;

    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    VERIFY(arg == 123);
    ret = dspLS(1, "testTask OK");
    CHECK(ret >= 0, lblKO);
    ret = prtS("test task");    //CHECK(ret<0,lblKO); //print failure under SIMELITE; for terminal it works
    play('a');
    strcpy(data, "test task filled");   //fill static variable
    play('A');                  //continue playing while main task is waiting
    ret = dspLS(2, "testTask Done");
    CHECK(ret >= 0, lblKO);
  lblKO:
    prtStop();
    semDec(0);                  //release semaphore
    tskStop();                  //delete task
}

void tcab0056(void) {
    int ret;

    memset(data, 0, prtW + 1);
    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

#ifndef _SIMULPC_               //Simulator is limited to one task : the application can’t use “fork” primitive.
    strcpy(data, "main task filled");   //fill static variable
    ret = prtS(data);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "tskStart");
    CHECK(ret >= 0, lblKO);

    prtStop();

    ret = tskStart(testTask, 123);
    CHECK(ret >= 0, lblKO);

    play('A');
    ret = dspLS(3, "mainTask wait");
    CHECK(ret >= 0, lblKO);

    //wait for the end of testTask..
    ret = semInc(0);
    CHECK(ret >= 0, lblKO);     //waiting for semDec() call from testTask

    //now testTask is finished
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = prtS(data);
    CHECK(ret >= 0, lblKO);     //the variable was filled by the secondary task
    ret = dspLS(3, "mainTask done");
    CHECK(ret >= 0, lblKO);
#else
    dspLS(0, "Simulator is");
    CHECK(ret >= 0, lblKO);

    dspLS(1, "limited to");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, "one task");
    CHECK(ret >= 0, lblKO);
#endif
    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
  lblEnd:
    tmrPause(3);
    prtStop();
    dspStop();
}
