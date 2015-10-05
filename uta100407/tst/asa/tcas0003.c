/// \file tcas0003.c

#include <string.h>
#include "sys.h"
#include "tst.h"

/** Unitary test case for pri component: internal modem testing in V32.
 * \sa
 *  - comStart()
 *  - comSet()
 *  - comStop()
 *  - comDial()
 *  - comSendBuf()
 *  - comRecvBuf()
 *  - comHangStart()
 *  - comHangWait()
 */

void tcas0003(void) {
    int ret;
    char *ptr;
    char msg[dspW + 1];
    char sndmsg[250];
    tBuffer buf;
    byte dat[256];

    // FILL BUFFER
    const char FillBuffer[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.THIS MESSAGE IS SENT IN V32.ENDING CHARACTER IS:]";

    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    memset(msg, 0, dspW + 1);
    /*! TEST SEQUENCE :
     * \par Step 1: call comStart()
     *  This function open communication perypheral for a selected channel.
     * In this case channel is internal MODEM
     * Error correction is enabled
     */
    ret = comStart(chnMdm);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "comSet...");
    CHECK(ret >= 0, lblKO);
    ptr = "?8N19600";

    ret = dspLS(1, ptr);
    CHECK(ret >= 0, lblKO);

    TRCDT("comSet...");
    /*!
     * \par Step 2: call comSet(const char * init)
     * This function initiates a channel using the parameter string init.\n
     * For internal modem (chnMdm, chnHdlc) Init string:
     * \htmlonly
     <table border=1 cellspacing=0 cellpadding=0
     <tr>   <td>Init part1</td>      <td>Parameter</td>                                                                                             </tr>
     <tr>   <td>'.'</td>               <td>enable data compression (optional)</td>                                          </tr>
     <tr>   <td>'?'</td>               <td>enable error correction (optional)</td>                                          </tr>
     <tr>   <td>'#'</td>               <td>enable DTR hangup mode (optional)</td>                                               </tr>
     <tr>   <td>'p'</td>               <td>pulse dialing, fast connection not allowed (optional)</td>       </tr>
     <tr>   <td>'P'</td>               <td>pulse dialing, fast connection allowed (optional)</td>               </tr>
     <tr>   <td>'t'</td>               <td>tone dialing, fast connection not allowed (optional)</td>        </tr>
     <tr>   <td>'T'</td>               <td>tone dialing, fast connection allowed (optional)</td>                </tr>
     <tr>   <td>'one digit'</td>     <td>number of data bits in ASCII ('7' or '8')</td>                             </tr>
     <tr>   <td>'one digit'</td>     <td>parity ('N','E' or 'O')</td>                                                                   </tr>
     <tr>   <td>'one digit'</td>     <td>number of stop bits in ASCII('0','1' or '2')</td>                      </tr>
     <tr>   <td>'3-5 digits'</td>    <td>baudrate in ASCI (300,600,1200,2400,9600,19200 or 28000)</td></tr>
     <tr>   <td>'D'</td>               <td>enable dial tone monitoring (optional)</td>                                  </tr>
     <tr>   <td>'?'</td>               <td>enable error correction (optional)</td>      </tr>
     <tr>   <td>'?'</td>               <td>enable error correction (optional)</td>      </tr>
     <tr>   <td>'?'</td>               <td>enable error correction (optional)</td>      </tr>
     </table>
     \endhtmlonly
     * \n
     * \n The optional digits are independent from each other, but they are not permutable.
     * For example ".#8N11200" is right, but "#.8N11200" is wrong.
     * For HDLC connection the init string is ignored (always 1200 baud)
     * \n
     */
    ret = comSet(ptr);
    CHECK(ret >= 0, lblKO);
    TRCDT("comSet done");

    ret = dspLS(0, "comSet OK");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "comDial...");
    CHECK(ret >= 0, lblKO);

    ptr = "18134";              //NCC
    ret = dspLS(2, ptr);
    CHECK(ret >= 0, lblKO);

    TRCDT("comDial...");
    /*!
     * \par Step 3: call comDial(const char * srv)
     * This function dial (connect) a server. If the current channel is internal modem dials a phone number.
     * Dialing can be stopped by pressing a key.
     * \n The function kbdStart() is called inside this function, thus keyboard waiting should not be started before calling it.
     * For internal modem srv string containing pabx and prefix separated by ‘-‘:
     * \n srv="0-00-1234" consists of pabx="0", prefix="00" and phone itself "1234"
     * \n srv="0-1234" consists of pabx="0" and phone itself "1234" without prefix srv="1234" does not contain any pabx and prefix
     */
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
    TRCDT("comDial done");
    ret = dspLS(1, "comDial OK");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, "Sending...");
    CHECK(ret >= 0, lblKO);
    /*!
     * \par Step 4: call comSendBuf(const byte * msg, word len)
     * This function sends a buffer.
     * This function use LL_SEND (Link Layer)from Telium.
     */

    // fill the buffer to be sent
    memset(sndmsg, 0, sizeof(sndmsg));
    memcpy(sndmsg, "\x60\x00\x10\x00\x80", 5);
    memcpy(sndmsg + 5, FillBuffer, strlen(FillBuffer));

    // send buffer
    ret = comSendBuf(sndmsg, 5 + (strlen(sndmsg + 5)));
    CHECK(ret >= 0, lblKO);
    ret = dspLS(2, "Sending OK");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(3, "Receiving...");
    CHECK(ret >= 0, lblKO);
    bufInit(&buf, dat, 256);
    /*!
     * \par Step 5: call comRecvBuf(tBuffer * msg, const byte * trm, byte dly)
     * This function receive a buffer until a terminator character.
     * Receive a sequence of bytes terminated by a character from trm into buffer msg to the communication channel using a timeout value dly.
     * The terminator control string consists of characters that can indicate the end of message (ETX, EOT etc).
     * This function use LL_RECEIVE (Link Layer)From Telium
     * \n Terminators characters :
     * \htmlonly
     <table border=1 cellspacing=0 cellpadding=0>
     <tr>   <td>byte number</td>                            <td>meaning</td>                                                                            </tr>
     <tr>   <td>first byte</td>                             <td>number of terminators characters (nbTerm)</td>      </tr>
     <tr>   <td>next bytes up to nbTerm</td>    <td>terminator characters</td>                                              </tr>
     <tr>   <td>following byte</td>                     <td>number of "transparent" characters (nbTrans)</td>   </tr>
     <tr>   <td>next bytes up to nbTrans</td>   <td>transparent characters</td>                                             </tr>
     <tr>   <td>NULL pointer</td>                   <td>no ending and transparent characters defined</td>   </tr>
     </table>
     \endhtmlonly
     * \n
     */
    ret = comRecvBuf(&buf, (byte *) "]", 20);
    CHECK(ret >= 0, lblKO);
    ret = dspLS(3, "Receiving OK");
    CHECK(ret >= 0, lblKO);

    /*!
     * \par Step 6: call comHangStart()
     * This function start waiting for modem hang up. For internal modem initiates hanging up.
     * Can be applied to internal modem only.
     * \n For 32bit it does not wait for result for performance reasons.
     * However, it is necessary to call comHangWait() before closing com port.
     * \n
     */
    ret = comHangStart();
    CHECK(ret >= 0, lblKO);

    /*!
     * \par Step 7: call comHangWait()
     * This function wait until modem hangs up. For internal modem wait while hang up finishes.
     * The function comHangStart() should be started before.
     */
    ret = comHangWait();
    CHECK(ret >= 0, lblKO);

    /*!
     * \par Step 8: call comStop()
     * This function close communication perypheral and the associated channel.
     */
    ret = comStop();            //close the port
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    trcErr(ret);
    dspLS(0, "KO!");
  lblEnd:
    tmrPause(3);
    comStop();
    dspStop();
}
