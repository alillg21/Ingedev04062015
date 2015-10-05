/** \file
 * Unitary test case for sys component: EMV selection testing.
 * \sa
 *  - emvSetFSbuf()
 *  - emvFinalSelect()
 *  - emvGetLang()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0013.c $
 *
 * $Id: tcab0013.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "sys.h"
#include "tst.h"

int tcab0013(int stop, byte * dat1, byte * dat2) {
#ifdef __EMV__
    int ret;
    char lan[8 + 1];
    static byte fsBuf[__FSBUFLEN__];

    trcS("tcab0013 Beg\n");

    VERIFY(dat1);
    VERIFY(dat2);

    /*! TEST SEQUENCE :
     * \par Step 1: Call tcab0012 to start EMV select processing
     * \sa  tcab0012.c
     */
    ret = tcab0012(0, dat1, dat2);  //Start emv select processing
    CHECK(ret >= 0, lblKO);
    dspClear();

    /*!
     * \par Step 2: call emvSetFSbuf(byte * buf)
     * This function set Final Select buffer pointer. It is not a wrapper.
     * It is an artefact of EMV Kernel architecture.
     * A piece of memory of size __FSBUFLEN__ bytes will be used to transmit an information from Selection to Transaction module of EMV Kernel.
     * It contains the response to the last APPLICATION SELECT command.
     * The function should be called just before emvFinalSelect function.
     * The memory will be used during emvContext function call, and it will not be used afterwards.
     */
    emvSetFSbuf(fsBuf);         //set the buffer to hold the last APPLICATION select command

    ret = dspLS(0, "emvFinalSelect...");
    CHECK(ret >= 0, lblKO);

    /*!
     * \par Step 3: call emvFinalSelect(const byte * aid)
     * This function is a wrapper around the function amgEmvFinalSelect.
     * Perform an explicite final APPLICATION SELECT command to before starting the EMV transaction flow.
     * It is the last operation of application selection procedure.
     * The candidate is selected before from the list using a menu or application selection algorithm.
     */
    ret = emvFinalSelect((byte *) "\x07\xA0\x00\x00\x00\x03\x10\x10");  //select a VSDC application
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, "OK");
    CHECK(ret >= 0, lblKO);

    /*!
     * \par Step 4: call emvGetLang(char * lan)
     * This function is a wrapper around the function amgEmvGetLanguagePreference. Retrieve language preference tag (5F2D).
     */
    ret = emvGetLang((char *) lan);
    VERIFY(ret <= 4);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, lan);
    CHECK(ret >= 0, lblKO);

    dspLS(3, "OK!");
    tmrPause(1);

    ret = 1;
    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(3, "KO!");
    Beep();
    tmrPause(3);
    ret = -1;
  lblEnd:
    if(stop) {
        emvStop();
        prtStop();
        dspStop();
    }
    trcS("tcab0013 End\n");
    return ret;
    /*!
     * \par Step 5: Analyse of traces
     Smard card commands:\n
     * \htmlonly
     <table border=1 cellspacing=0 cellpadding=0>
     <tr>   <td>Exchange number</td><td>Command</td>        </tr>
     <tr>   <td>1</td>      <td>--> New Power On</td>       </tr>
     <tr>   <td>2</td>      <td>cardStandards=0x01 GRClassByte00=0x01 preferredProtocol=0xFF</td>       </tr>
     <tr>   <td>3</td>      <td><-- 0x3B 0x6E 0x00 0xFF 0x53 0x46 0x53 0x45 0x2D 0x43 0x31 0x36<br>
     0x34 0x2D 0x56 0x05 0x02 0x00 </td>        </tr>
     <tr>   <td>4</td>      <td>--> Async Command #4</td>       </tr>
     <tr>   <td>5</td>      <td>CLA=0x00 INS=0xA4 P1=0x04 P2=0x00 Lc=0x0E Le=0x00 dataIn=0x31 <br>
     0x50 0x41 0x59 0x2E 0x53 0x59 0x53 0x2E 0x44 0x44 0x46 0x30 0x31</td>      </tr>
     <tr>   <td>6</td>      <td><-- 0x6F 0x22 0x84 0x0E 0x31 0x50 0x41 0x59 0x2E 0x53 0x59 0x53<br>
     0x2E 0x44 0x44 0x46 0x30 0x31 0xA5 0x10 0x88 0x01 0x01 0x5F 0x2D<br>
     0x06 0x6A 0x61 0x6B 0x6F 0x7A 0x68 0x9F 0x11 0x01 0x01 0x90 0x00 </td>     </tr>
     <tr>   <td>7</td>      <td>--> Async Command #2</td>       </tr>
     <tr>   <td>8</td>      <td>CLA=0x00 INS=0xB2 P1=0x01 P2=0x0C Le=0x00</td>      </tr>
     <tr>   <td>9</td>      <td><-- 0x70 0x2C 0x61 0x2A 0x4F 0x07 0xA0 0x00 0x00 0x00 0x03 0x10<br>
     0x10 0x87 0x01 0x01 0x50 0x0B 0x56 0x49 0x53 0x41 0x20 0x43 0x52<br>
     0x45 0x44 0x49 0x54 0x9F 0x12 0x0E 0x56 0x49 0x53 0x41 0x20 0x50<br>
     0x52 0x45 0x46 0x45 0x52 0x52 0x45 0x44 0x90 0x00</td>     </tr>
     <tr>   <td>10</td>     <td>--> Async Command #2</td>       </tr>
     <tr>   <td>11</td>     <td>CLA=0x00 INS=0xB2 P1=0x02 P2=0x0C Le=0x00</td>      </tr>
     <tr>   <td>12</td>     <td><-- 0x6A 0x83 </td>     </tr>
     <tr>   <td>13</td>     <td>--> Async Command #4</td>       </tr>
     <tr>   <td>14</td>     <td>CLA=0x00 INS=0xA4 P1=0x04 P2=0x00 Lc=0x07 Le=0x00<br>
     dataIn=0xA0 0x00 0x00 0x00 0x03 0x10 0x10 </td>        </tr>
     <tr>   <td>15</td>     <td><-- 0x6F 0x39 0x84 0x07 0xA0 0x00 0x00 0x00 0x03 0x10 0x10 0xA5<br>
     0x2E 0x50 0x0B 0x56 0x49 0x53 0x41 0x20 0x43 0x52 0x45 0x44 0x49<br>
     0x54 0x5F 0x2D 0x06 0x6A 0x61 0x6B 0x6F 0x7A 0x68 0x9F 0x12 0x0E<br>
     0x56 0x49 0x53 0x41 0x20 0x50 0x52 0x45 0x46 0x45 0x52 0x52 0x45<br>
     0x44 0x87 0x01 0x01 0x9F 0x11 0x01 0x01 0x90 0x00 </td>        </tr>
     </table>
     \endhtmlonly
     * \n
     * \b Trace \b file \b content:
     * \n
     * \include tcab0013.trc
     * \n
     \b Remarks \b for \b trace \b file :
     * \n
     * \htmlonly
     <table border=1 cellspacing=0 cellpadding=0>
     <tr>   <td>Line</td><td>Remark</td>        </tr>
     <tr>   <td>Line 46.</td>           <td>The emvFinalSelect function is called here.</td>        </tr>
     <tr>   <td>Line 46.</td>           <td>The buffer buf contains the response to the last APPLICATION SELECT command<br>
     The first two bytes are SW1SW2= 90 00 in the reversed order
     The following two bytes is the length of the response in reversed order</td>       </tr>
     <tr>   <td>Lines 49-50.</td>   <td>The APPLICATION SELECT command for VSDC application</td>        </tr>
     <tr>   <td>Line 51.</td>           <td>The card responds sending a constructed tag.</td>       </tr>
     <tr>   <td>Line 52.</td>           <td>Tag 6F is FCI Template. It is mandatory in the response to APPLICATION SELECT command<br>
     In our case it consists of tags 84 and A5</td>         </tr>
     <tr>   <td>Line 53.</td>           <td>The tag 84 is DF Name. It is also mandatory and it contains the application name selected</td>      </tr>
     <tr>   <td>Line 54.</td>           <td>The tag A5 is FCI Proprietary Template (mandatory)<br>
     It is constructed template and contains the tags 50, 5F2D,9F12,87 and 9F11</td>        </tr>
     <tr>   <td>Line 55.</td>           <td>The tag 50 is Application Label, VISA CREDIT in our case.</td>      </tr>
     <tr>   <td>Line 56.</td>           <td>The tag 5F2D is Language Preference: ja means japan, ko means korean, etc</td>      </tr>
     <tr>   <td>Line 57.</td>           <td>The tag 9F12 is Application Preferred Name, VISA PREFERRED in our case.</td>        </tr>
     <tr>   <td>Line 58.</td>           <td>Tag 87 is Application Priority Indicator.   It has an internal structure:
     <ul>bit8=1: Application shall not be selected without confirmation of cardholder<br>
     bit8=0: Application may be selected without confirmation of cardholder<br>
     bits 7-5:   RFU (000)<br>
     bits 4-1:
     <ul>0000 = No priority assigned<br>
     xxxx = Order in which the application is to be<br>
     listed or selected, ranging from 1 to 15,<br>
     with 1 being the highest priority</ul></ul></td>       </td>       </tr>
     <tr>   <td>Line 59.</td>           <td>The tag 9F11 is Issuer Code Table Index.    The value means latin alphabet</td>         </tr>
     </table>
     \endhtmlonly
     */
#else
    return -1;
#endif
}
