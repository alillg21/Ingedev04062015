/// \file tcik0169.c

#include <string.h>
#include "tst.h"
#include "sys.h"

/** UTA test case for integration of GPRS GSM plug-in function.
* pgnLastErr
 */

void tcik0169(void) {
#ifdef __PGN_GSM__
    int ret;
    char buf[dspW + 1];
    byte connType;

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    memset(buf, 0, sizeof(buf));

    ret = pgnLastErr(&connType);
    CHECK(ret < 0, lblEnd);     //No treatment if no error

    ret = dspLS(0, "Error code:");
    CHECK(ret >= 0, lblKO);

    switch (connType) {
      case 0:                  //General error
          break;
      case 1:                  // GSM Connection Error
          switch (ret) {
            case -1:           // PIN Required
            case -2:           // PUK Required
            case -3:           // SIM not inserted
                // TODO: Insert your code here
                break;
          }
          break;
      case 2:                  //  GPRS Connection Error
          switch (ret) {
            case -11:          // netNiStart error when the error not fit in the errors bellow
            case -12:          // netNiStart error NET_ERR_NI_IN_WRONG_STATE
            case -13:          // netNiStart error NET_ECONNREFUSED
            case -14:          // netNiStart error NET_ETIMEDOUT
            case -20:          // error in the attach process.
            case -22:          // GPRS not allowed (GSM_CME_ERROR error number 107)
            case -23:          // GPRS UNSPECIFIED error (GSM_CME_ERROR error 148)
            case -24:          // GPRS PDP AUTH FAILURE (GSM_CME_ERROR error 149)
            case -25:          // GPRS MISSING/UNK APN (GSM_CME_ERROR error 533)
            case -27:          // temp unavailable   (GSM_CME_ERROR error 134)
            case -28:          // connection timeout
            case -30:          // GPRS internal error
            case -31:          // APN not set
                // TODO: Insert your code here
                break;
          }
          break;
    }
    tmrPause(1);
  lblKO:
    goto lblEnd;
  lblEnd:
    tmrPause(3);
    prtStop();
    dspStop();
#else
    return;
#endif
}
