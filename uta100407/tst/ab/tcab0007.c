/** \file
 * Unitary test case for sys component: smart card processing.
 * \sa
 *  - iccStart()
 *  - iccStop()
 *  - iccCommand()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0007.c $
 *
 * $Id: tcab0007.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "sys.h"
#include "tst.h"

static int prtH(const byte * buf, byte len) {
    int ret;
    char hex[512 + 1];
    char *ptr;
    word idx;

    VERIFY(buf);

    memset(hex, 0, 512 + 1);
    bin2hex(hex, buf, len);
    idx = (word) len + len;
    ptr = hex;
    while(idx) {
        ret = prtS(ptr);
        CHECK(ret >= 0, lblKO);
        if(idx < prtW)
            break;
        idx -= prtW;
        ptr += prtW;
    }
    return len;
  lblKO:
    return -1;
}

void tcab0007(void) {
    int ret;
    int len;
    byte rsp[260];
    char *ptr;

    trcS("tcab0007 Beg\n");

    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "Insert a card...");
    CHECK(ret >= 0, lblKO);

    ret = tmrStart(0, 60 * 100);    //60 seconds timeout
    CHECK(ret >= 0, lblKO);

    /*! TEST SEQUENCE :
     * \par Step 1: call iccStart()
     *  This function open Integrated Circuit Card perypheral.
     * Open the associated channel for the default card reader.
     */
    ret = iccStart(0);          //Open associated channel and start waiting for a card
    CHECK(ret >= 0, lblKO);

    len = 0;
    while(tmrGet(0)) {
        /*!
         * \par Step 2: call iccCommand(byte rdr, const byte * cmd, const byte * dat, byte * rsp)
         * This function send a command cmd followed by data dat to the card; capture the result into rsp.
         * The command cmd consists of 5 bytes:
         * \htmlonly
         <table border=1 cellspacing=0 cellpadding=0>
         <tr>   <td>Bytes</td>   <td>Value</td>                     </tr>
         <tr>   <td>0</td>         <td>TYP: the type of command, can take values in the range 0..4:
         <center><table border=1 cellspacing=0 cellpadding=0>
         <tr><td>0</td> <td>without any protocol</td></tr>
         <tr><td>1</td> <td>no input, no output</td></tr>
         <tr><td>2</td> <td>no input, some output</td></tr>
         <tr><td>3</td> <td>some input, no ouput</td></tr>
         <tr><td>4</td> <td>some input, some output</td></tr>
         </table></center>
         </td>      </tr>
         <tr>   <td>1</td>         <td>CLA: class byte</td>     </tr>
         <tr>   <td>2</td>         <td>INS: instruction byte</td>       </tr>
         <tr>   <td>3</td>         <td>P1: parameter 1 byte</td>        </tr>
         <tr>   <td>4</td>         <td>P1: parameter 2 byte The data dat consists of the length of data and the data itself.</td>       </tr>
         </table>
         \endhtmlonly
         * \n
         \b Possible \b values \b of \b parameters:
         * \htmlonly
         <table border=1 cellspacing=0 cellpadding=0>
         <tr>   <td>Parameters</td>  <td>(I/O)</td> <td>Value</td>                  </tr>
         <tr>   <td>rdr</td>                 <td>(I)</td>   <td>Reader number, 0..4</td>        </tr>
         <tr>   <td>cmd</td>             <td>(I)</td>       <td>Command to send, consists of 6 bytes: TYP,CLA,INS,P1,P2,LC;<br>
         for POWER ON command the pointer cmd should be zero.</td>      </tr>
         <tr>   <td>dat</td>             <td>(I)</td><td>Data to send; zero pointer if there is no data.<br>
         The first byte is the length of the data;<br>
         after that the data array follows.</td>        </tr>
         <tr>   <td>rsp</td>             <td>(O)</td><td>Buffer to capture card response, this size of the buffer should be 260.<br>
         If rsp is zero, it means that no output is expected.</td>      </tr>
         </table>
         \endhtmlonly
         * \n
         Here iccCommand is called for POWER ON command
         */
        ret = iccCommand(0, (byte *) 0, (byte *) 0, rsp);   //look for a result; if a card is inserted, capture ATR
        if(ret == -iccCardRemoved)
            continue;
        CHECK(ret >= 0, lblKO);
        if(!ret)
            continue;
        len = ret;
        break;
    }

    if(!tmrGet(0))
        goto lblEnd;            //card is not inserted
    tmrStop(0);

    ret = prtS("ATR:");
    CHECK(ret >= 0, lblKO);

    ret = prtH(rsp, (byte) len);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "ATR printed");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "Wait...");
    CHECK(ret >= 0, lblKO);

    tmrPause(5);

    /*!
     * \par Step 3: call iccCommand(byte rdr, const byte * cmd, const byte * dat, byte * rsp)
     * Send command to card for application selection
     */
    ret = iccCommand(0, (byte *) "\x04\x00\xA4\x04\x00 ", (byte *) "\x0E" "1PAY.SYS.DDF01", rsp);   //APPLICATION SELECT
    if(ret <= 0) {
        switch (-ret) {
          case 0:
          case iccCardRemoved:
              ptr = "Card removed";
              break;
          case iccCardMute:
              ptr = "Card mute";
              break;
          case iccCardPb:
              ptr = "Card Pb";
              break;
          case iccDriverPb:
              ptr = "Dirver Pb";
              break;
          case iccKO:
          default:
              ptr = "Unknown Pb";
              break;
        }
        dspLS(1, ptr);
    } else {
        len = ret;
        ret = prtS("APPLICATION SELECT:");
        CHECK(ret >= 0, lblKO);

        ret = prtH(rsp, (byte) len);
        CHECK(ret >= 0, lblKO);

        ret = dspLS(1, "APP SELECT OK");
        CHECK(ret >= 0, lblKO);
    }

    ret = dspLS(2, "Remove card...");
    CHECK(ret >= 0, lblKO);

    tmrPause(5);
    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO!");
    tmrPause(3);
  lblEnd:
    /*!
     * \par Step 3: call iccStop()
     * This function close the associated channel of Integrated Circuit Card perypheral.
     */
    iccStop(0);                 //stop waiting card and close associated channel
    prtStop();
    dspStop();

    /*!
     * \par Step 4: Analyse of traces
     * \n
     * Smard card commands: \n
     * \include tcab0007_smc.trc
     * \n
     * Trace file content: \n
     * \include tcab0007.trc
     * \n
     * Remarks : \n
     * - If card is not inserted during 60 seconds the application returns to idle prompt.\n
     * - If at the step 3 the card is removed instead of waiting for 5 seconds, it should be detected. The line "Card removed" must be displayed.
     * - If at the step 4 the card is not removed the application returns to idle prompt. Just remove the card in this case.
     * \n
     */
    trcS("tcab0007 End\n");
}
