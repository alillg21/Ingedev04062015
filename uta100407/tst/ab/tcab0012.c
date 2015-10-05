/** \file
 * Unitary test case for sys component: EMV select processing testing.
 * \sa
 *  - emvStart()
 *  - emvStop()
 *  - emvSelect()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0012.c $
 *
 * $Id: tcab0012.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "sys.h"
#include "tst.h"

int tcab0012(int stop, byte * dat, byte * tmp) {
#ifdef __EMV__
#define L_BUF 2048
    int ret;
    tQueue que;
    tBuffer buf;

    trcS("tcab0012 Beg\n");
    VERIFY(dat);
    VERIFY(tmp);

    /*! TEST SEQUENCE :
     * \par Step 1: Call tcab0011 for EMV initialisation (set AIDs)
     * \sa  tcab0011.c
     */
    ret = tcab0011(0, dat);     //To be sure that emvInit is called
    CHECK(ret >= 0, lblKO);

    dspClear();
    dspLS(0, "Insert ADVTK03...");

    ret = tmrStart(0, 60 * 100);    //60 seconds timeout
    CHECK(ret >= 0, lblKO);

    ret = iccStart(0);          //Open associated channel and start waiting for a card
    CHECK(ret >= 0, lblKO);

    while(tmrGet(0)) {
        ret = iccCommand(0, (byte *) 0, (byte *) 0, tmp);   //look for a result; if a card is insertes, capture ATR
        if(ret == -iccCardRemoved)
            continue;
        CHECK(ret >= 0, lblKO);
        if(!ret)
            continue;
        break;
    }

    iccStop(0);

    if(!tmrGet(0))
        goto lblEnd;            //card is not inserted
    tmrStop(0);

    /*!
     * \par Step 2: call emvStart()
     * This function is a wrapper around the function amgInitPayment.
     * This function should be called before starting EMV processing.
     * this wrapper dans l'information about the means of payment set following parameters :
     * - msgType = MSG_SMART_CARD
     * - msgLength = 0
     * - receiverPid = 0xFFFF (not used)
     * - callerTaskId = 0xFFFF (not used)
     */
    ret = emvStart();
    CHECK(ret >= 0, lblKO);

    bufInit(&buf, dat, L_BUF);
    queInit(&que, &buf);

    dspLS(1, "emvSelect...");
    /*!
     * \par Step 3: call emvSelect(tQueue * que)
     * This function is a wrapper around the function amgAskSelection.
     * Creates a list of AIDs from the card that are accepted by the Kernel (see emvInit function).
     * Returns a queue of (AID, application name,priority indicator) triples sorted according to the priority.
     * This queue should be used by the application to select the first element of the queue,
     * or to organise a menu of AIDs, ask for the confirmation or just perform a final select call,
     * depending on the application capabilities. According the EMV specification,
     * the confirmation should be asked if the highest bit of priority indicator is set.
     * For an idle application manipulating with several EMV applications this wrapper is not suitable:
     * the association between AIDs and PIDs is removed.
     */
    ret = emvSelect(&que);      //ask EMV Kernel to prepare a candidate list
    CHECK(ret >= 0, lblKO);
    CHECK(ret == 1, lblKO);
    CHECK(queLen(&que) == 3, lblKO);    //for test card 2 there is only 1 candidate triple (AID,name,priority)

    CHECK(queNxt(&que) <= 16, lblKO);
    memset(tmp, 0, L_BUF);
    ret = queGet(&que, tmp);    //aid
    CHECK(ret > 0, lblKO);
    CHECK(memcmp("\xA0\x00\x00\x00\x03\x10\x10", tmp, 7) == 0, lblKO);  //VSDC

    CHECK(queNxt(&que) <= 16, lblKO);
    memset(tmp, 0, L_BUF);
    ret = queGet(&que, tmp);    //name
    CHECK(ret > 0, lblKO);
    dspLS(2, (char *) tmp);
    tmrPause(1);

    CHECK(queNxt(&que) == 1, lblKO);
    memset(tmp, 0, L_BUF);
    ret = queGet(&que, tmp);    //priority
    CHECK(ret == 1, lblKO);
    CHECK(queLen(&que) == 0, lblKO);

    dspLS(3, "OK!");
    tmrPause(1);

    ret = 1;
    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(3, "KO!");
    iccStop(0);
    Beep();
    tmrPause(3);
    ret = -1;
  lblEnd:
    if(stop) {
        /*!
         * \par Step 4: call emvStop()
         * This function is a wrapper around the functions amgEndPayment and amgEmvCloseContext.
         * This function should be called after finishing EMV processing and in case of exceptions
         */
        emvStop();
        prtStop();
        dspStop();
    }
    trcS("tcab0012 End\n");
    return ret;
    /*!
     * \par Step 5: Analyse of traces
     Smard card commands:\n
     * \htmlonly
     <table border=1 cellspacing=0 cellpadding=0>
     <tr>   <td>Exchange number</td><td>Command</td>        </tr>
     <tr>   <td>1</td><td>--> New Power On</td>     </tr>
     <tr>   <td>2</td><td>cardStandards=0x01 GRClassByte00=0x01 preferredProtocol=0xFF</td>     </tr>
     <tr>   <td>3</td><td><-- 0x3B 0x6E 0x00 0xFF 0x53 0x46 0x53 0x45 0x2D 0x43 0x31 <br>
     0x36 0x34 0x2D 0x56 0x05 0x02 0x00</td>        </tr>
     <tr>   <td>4</td><td>--> Async Command #4</td>     </tr>
     <tr>   <td>5</td><td>CLA=0x00 INS=0xA4 P1=0x04 P2=0x00 Lc=0x0E Le=0x00 dataIn=0x31 <br>
     0x50 0x41 0x59 0x2E 0x53 0x59 0x53 0x2E 0x44 0x44 0x46 0x30 0x31</td>      </tr>
     <tr>   <td>6</td><td><-- 0x6F 0x22 0x84 0x0E 0x31 0x50 0x41 0x59 0x2E 0x53 0x59 0x53 <br>
     0x2E 0x44 0x44 0x46 0x30 0x31 0xA5 0x10 0x88 0x01 0x01 0x5F 0x2D <br>
     0x06 0x6A 0x61 0x6B 0x6F 0x7A 0x68 0x9F 0x11 0x01 0x01 0x90 0x00 </td>     </tr>
     <tr>   <td>7</td><td>--> Async Command #2</td>     </tr>
     <tr>   <td>8</td><td>CLA=0x00 INS=0xB2 P1=0x01 P2=0x0C Le=0x00</td>        </tr>
     <tr>   <td>9</td><td><-- 0x70 0x2C 0x61 0x2A 0x4F 0x07 0xA0 0x00 0x00 0x00 0x03 0x10 <br>
     0x10 0x87 0x01 0x01 0x50 0x0B 0x56 0x49 0x53 0x41 0x20 0x43 0x52 <br>
     0x45 0x44 0x49 0x54 0x9F 0x12 0x0E 0x56 0x49 0x53 0x41 0x20 0x50 <br>
     0x52 0x45 0x46 0x45 0x52 0x52 0x45 0x44 0x90 0x00</td>     </tr>
     <tr>   <td>10</td><td>--> Async Command #2</td>        </tr>
     <tr>   <td>11</td><td>CLA=0x00 INS=0xB2 P1=0x02 P2=0x0C Le=0x00</td>       </tr>
     <tr>   <td>12</td><td><-- 0x6A 0x83 </td>      </tr>
     </table>
     \endhtmlonly
     * \n
     \b Trace \b file \b content:
     * \n
     * \include tcab0012.trc
     * \n
     \b Remarks \b for \b trace \b file :
     * \n
     * \htmlonly
     <table border=1 cellspacing=0 cellpadding=0>
     <tr>   <td>Line</td><td>Remark</td>        </tr>
     <tr>   <td>Lines 17-18.</td>   <td>The EMV Kernel starts by selecting PSE      CLA INS = 00 A4 means APPLICATION SELECT<br>
     P1 = 04 is reference control parameter (select by name)<br>
     The data 1PAY.SYS.DDF01 is reserved name for PSE</td>      </tr>
     <tr>   <td>Line 19.</td>           <td>The card responds sending a constructed tag.</td>       </tr>
     <tr>   <td>Line 20.</td>           <td>Tag 6F is FCI Template. It is mandatory in the response to APPLICATION SELECT command<br>
     In our case it consists of tags 84 and A5</td>     </tr>
     <tr>   <td>Line 21.</td>           <td>The tag 84 is DF Name. It is also mandatory and it contains the application name selected</td>      </tr>
     <tr>   <td>Line 22.</td>           <td>The tag A5 is FCI Proprietary Template (mandatory)<br>
     It is constructed template and contains the tags 88, 5F2D and 9F11</td>        </tr>
     <tr>   <td>Line 23.</td>           <td>The tag 88 is SFI of the directory elementary file.
     In our case it is 1. Thus, the file SFI=1 will be read to retrieve the list of applications.</td>      </tr>
     <tr>   <td>Line 24.</td>           <td>The tag 5F2D is Language Preference:    ja means japan, ko means korean, etc</td>       </tr>
     <tr>   <td>Line 25.</td>           <td>The tag 9F11 is Issuer Code Table Index. The value means latin alphabet</td>        </tr>
     <tr>   <td>Line 26.</td>           <td>SW1SW2= 90 00 means OK</td>     </tr>
     <tr>   <td>Line 27.</td>           <td>Now EMV Kernel reads SFI=1 to retrive the list of applications from the card directory<br>
     CLA INS = 00 B2 means READ RECORD command<br>
     P1 = 01 is the record number according to P2 value<br>
     P2 has an internal structure:<br>
     P2 - xxxxx100: xxxxx=00001=SFI; 100 indicates that P1 is a record number</td>      </tr>
     <tr>   <td>Line 28.</td>         <td>The card responds sending a constructed tag.</td>     </tr>
     <tr>   <td>Line 29.</td>           <td>The Tag 70 is AEF Data Template.    In our case it contains only one tag 61</td>        </tr>
     <tr>   <td>Line 30.</td>           <td>Tag 61 is Application Template<br>
     It is constructed template and contains the tags 4F,87,50 and 9F12</td>        </tr>
     <tr>   <td>Line 31.</td>           <td>Tag 4F is Application Identifier<br>
     The value A0 00 00 00 03 10 10 means VSDC<br>
     It will be put into the output queue</td>      </tr>
     <tr>   <td>Line 32.</td>           <td>Tag 87 is Application Priority Indicator<br>
     It has an internal structure:
     <ul>bit8=1: Application shall not be selected without confirmation of cardholder<br>
     bit8=0: Application may be selected without confirmation of cardholder<br>
     bits 7-5:   RFU (000)<br>
     bits 4-1:
     <ul>0000 = No priority assigned<br>
     xxxx = Order in which the application is to be<br>
     listed or selected, ranging from 1 to 15,<br>
     with 1 being the highest priority<br>
     It will be put into the output queue</ul></ul></td>        </tr>
     <tr>   <td>Line 33.</td>           <td>Tag 50 is Application Label<br>
     It can be put into the output queue if there is no Application preferred name tag 9F12</td>        </tr>
     <tr>   <td>Line 34.</td>           <td>Tag 9F12 is Application Preferred Name<br>
     It will be put into the output queue</td>      </tr>
     <tr>   <td>Line 36.</td>           <td>EMV Kernel tries to read the next record of SFI=1.</td>     </tr>
     <tr>   <td>Line 37.</td>           <td>The response 6A83 means that there is no more records.</td>     </tr>
     <tr>   <td>General</td>            <td>The EMV Kernel found only one application for the card inserted.<br>
     It fills the output queue by the triple (AID,name,priority) corresponding to this application.</td>        </tr>
     <tr>   <td>General</td>            <td><If the card is not removed at the end of processing during one second,<br>
     the idle application may ask to enter amount.  Just remove the card in this case.</td>     </tr>
     </table>
     \endhtmlonly
     * \n
     */
#else
    return -1;
#endif

}
