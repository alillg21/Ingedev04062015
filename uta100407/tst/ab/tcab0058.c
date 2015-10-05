/** \file
 * Integration test case tcab0058.
 * Setting modem & port parameters in a parallel task
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0058.c $
 *
 * $Id: tcab0058.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "pri.h"
#include "tst.h"

/** The main task creates a secondary task and gives it the control over the comPort.
 * The secondary task takes the control over the comport and set com parameters.
 * The tasks are synchronised in the following manner:
 *   main                           | sec
 * tskStart                         |
 * semInc(1): wait for sec started  |
 *                                  |semDec(1) now it is stared
 * ---------------------------------|------------------ synchronised
 * tskComCtl(1)                     |semInc(2) wait for comCtl call
 * semDec(2): now the control is given to sec
 * ---------------------------------|------------------ synchronised
 * do something                     |comSet(...)
 * semInc(0): wait for finishing sec|tskComCtl(0): return control to main
 *                                  |semDec(0): that's all 
 * ---------------------------------|------------------ synchronised
 * comDial(...)
 * 
 * Three semaphores are used to do the job:
 *  - 0: to indicate when the sec task is finished
 *    the main task waits for it to start dialing
 *  - 1: to indicate when sec start is started
 *    the main task waits for it to give the control over the com
 *  - 2: to indicate that the control over the com is given to the sec task
 *    the sec task waits for it before calling comSet
 */

static char comSettings[32 + 1];    //input data for task
static int comRet;              //task return code
static void taskComSet(card arg) {
    VERIFY(comSettings);
    trcS("comSet task\n");

    semDec(1);                  //inform the parent task that the task is started
    semInc(2);                  //wait while the parent task gives the control on the com port    
    comRet = comSet(comSettings);
    trcS("comSet done\n");
    dspLS(3, "comSet Done");
    if(arg) {                   //started as parallel task
        tskComCtl(0);           //return the control on the comport to the parent
        semDec(0);              //release semaphore
        tskStop();              //delete task
    }
}

void tcab0058(void) {
    int ret;
    int idx;
    byte b;
    char *ptr;
    char msg[dspW + 1];

    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    //com should be started in main task
    ret = comStart(0);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "Setting com...");
    strcpy(comSettings, "07E11200");

    ret = tskStart(taskComSet, 1);
    CHECK(ret >= 0, lblKO);

    semInc(1);                  //wait to be sure that the task is really started

    ret = tskComCtl(1);         //now give the control on comport to the task
    CHECK(ret >= 0, lblKO);

    semDec(2);                  //inform the task that it can perform the job now

    //Do something...
    ret = dspLS(1, "Press a key...");

    ret = kbdStart(1);          //start keyboard waiting
    CHECK(ret >= 0, lblKO);

    ret = tmrStart(0, 5 * 100);
    while(tmrGet(0)) {
        ret = kbdKey();         //retrieve a key if pressed
        if(ret)
            break;              //quit the loop if a key pressed
    }
    ret = kbdStop();            //stop keyboard waiting

    ret = dspLS(2, "Synchronize...");

    ret = semInc(0);
    CHECK(ret >= 0, lblKO);     //waiting for semDec(0) call from taskComSet

    CHECK(comRet >= 0, lblKO);
    ret = dspLS(3, "Done");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "comDial...");
    CHECK(ret >= 0, lblKO);
    ptr = "0-0836065555";       //ATOS
    ret = dspLS(1, ptr);
    CHECK(ret >= 0, lblKO);

    ret = comDial(ptr);
    if(ret < 0) {
        switch (-ret) {
          case comAnn:
              ret = dspLS(2, "comDial Aborted");
              break;
          case comBusy:
              ret = dspLS(2, "comDial BUSY");
              break;
          case comNoDialTone:
              ret = dspLS(2, "NO TONE");
              break;
          case comNoCarrier:
              ret = dspLS(2, "NO CARRIER");
              break;
          default:
              ret = dspLS(2, "UNKNOWN PB");
              break;
        }
        goto lblEnd;
    }
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "comDial OK");
    CHECK(ret >= 0, lblKO);

    memset(msg, 0, dspW + 1);
    for (idx = 0; idx < 64; idx++) {
        ret = comRecv(&b, 20);  //receive characters
        if(ret < 0)
            break;
        if('A' <= b && b <= 'Z') {
            msg[idx % dspW] = (char) b; //display ascii characters
            dspLS(2, msg);
        }
    }

    ret = dspLS(3, "Receiving done");
    CHECK(ret >= 0, lblKO);

    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
  lblEnd:
    comHangStart();
    comHangWait();
    comStop();
    tmrPause(3);
    dspStop();
}
