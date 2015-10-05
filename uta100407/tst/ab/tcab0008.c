/** \file
 * Unitary test case for pri component: internal modem testing.
 * \sa
 *  - comStart()
 *  - comSet()
 *  - comStop()
 *  - comDial()
 *  - comHangStart()
 *  - comHangWait()
 *  - comRecv()
 *  - comRecvBuf()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0008.c $
 *
 * $Id: tcab0008.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "sys.h"
#include "tst.h"

void tcab0008(void) {
    int ret;
    int idx;
    byte b;
    char *ptr;
    char msg[dspW + 1];
    tBuffer buf;
    byte dat[256];

    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    memset(msg, 0, dspW + 1);
    /*! TEST SEQUENCE :
     * \par Step 1: call comStart()
     *  This function open communication perypheral for a selected channel.
     * In this case channel is internal MODEM
     * if country code is defined by __COUNTRY__, this code is set to dynamic configuration of internal modem
     */
    ret = comStart(chnMdm);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "comSet...");
    CHECK(ret >= 0, lblKO);
    ptr = "7E12400";            //SATIM
    //ptr= "8N12400"; //Oxigen
    //ptr = "7E11200";            //ATOS
//      ptr= "7E12400"; //SMT
//      ptr= "7E11200"; //SG
    //ptr = "8N119200";

    ret = dspLS(3, ptr);
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
    //ptr= "0-00-302109503410"; //Greece
    //ptr= "0-4117133"; //Oxigen
    //ptr = "0-0836065555";       //France
    //ptr = "*808";       
    //ptr= "0-00-914422540268"; //Delhi
    //ptr= "0-00-2218399550"; //Dakar
    //ptr= "0-00-21671951036"; //Tunis
    //ptr= "0-00-97165739374"; //Dubai
    //ptr= "0-00-3614212700"; //Hungary
    //ptr= "0-00-243811559000"; //Kongo
    //ptr = "0-00-21321448641";       //Algerie
    //ptr= "83794823"; //Khartoum
    //ptr = "18134";       //NCC
    //ptr = "15620";        //from Adil
    //ptr = "18134";              //from Adil 
    //ptr = "00146258134";      //from Adil 
    ptr = "0-00-3614212700";    //Hungary
    //ptr = "0-0141446942"; //IK
    ret = dspLS(3, ptr);
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

    ret = dspLS(3, "Receiving per char");
    CHECK(ret >= 0, lblKO);
    for (idx = 0; idx < 6; idx++) {
        /*!
         * \par Step 4: call comRecv(byte * b, int dly)
         * This function receive a character.
         * Receive a byte into b to the communication channel using a timeout value dly.
         * This function use comReceiveByteReq() from unicapt32.
         */
        ret = comRecv(&b, 20);  //receive characters
        if(ret < 0)
            break;
        bin2hex(msg + idx * 2, &b, 2);
        dspLS(2, msg);
    }
    ret = dspLS(3, "Receiving done");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(3, "Sending ...");
    CHECK(ret >= 0, lblKO);

    //memcpy(msg, "testCC", 6 + 1);
    memcpy(msg, "\x02testCC\x03", 8 + 1);
    ret = comSendBuf((byte *) msg, sizeof(msg));
    CHECK(ret >= 0, lblKO);

    ret = dspLS(3, "Receiving buf");
    CHECK(ret >= 0, lblKO);
    bufInit(&buf, dat, 256);
    /*!
     * \par Step 5: call comRecvBuf(tBuffer * msg, const byte * trm, byte dly)
     * This function receive a buffer until a terminator character.
     * Receive a sequence of bytes terminated by a character from trm into buffer msg to the communication channel using a timeout value dly.
     * The terminator control string consists of characters that can indicate the end of message (ETX, EOT etc).
     * This function use comReceiveMsgWait() from unicapt32
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
    ret = comRecvBuf(&buf, (byte *) "C", 20);
    CHECK(ret >= 0, lblKO);

    memcpy(msg, bufPtr(&buf), dspW);
    dspLS(2, msg);
    ret = dspLS(3, "Receiving done");
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
