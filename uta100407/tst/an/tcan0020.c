#include <string.h>
#include "sys.h"
#include "tst.h"
#include "LinkLayer.h"

//Integration test case tcan0020.
//Testing PPP connection with BKI M2M Switch


//connection error handler
#ifdef __TEST__
static void cnxError (void) 
{

	// Local variables 
    // ***************
	char *pcError;
	
	// List error messages 
    // *******************
	switch (comGetLastError())
	{
	// *** GPRS status ***
	case LL_STATUS_GPRS_ERROR_NO_SIM:                pcError="NO SIM";           break;
	case LL_STATUS_GPRS_ERROR_SIM_LOCK:              pcError="SIM LOCKED";       break;
	case LL_STATUS_GPRS_ERROR_BAD_PIN:               pcError="BAD PIN";          break;
	case LL_STATUS_GPRS_ERROR_NO_PIN:                pcError="PIN REQUIRED";     break;
	                       // ERRORS CODES
    // *** Generic errors ***
	case LL_ERROR_UNKNOWN:                     pcError="UNKNOWN ERROR";    break;
	case LL_ERROR_UNKNOWN_CONFIG:              pcError="UNKOWN CONFIG";    break;
	case LL_ERROR_INVALID_HANDLE:              pcError="INVALID HANDLE";   break;
	case LL_ERROR_SERVICE_NOT_SUPPORTED:       pcError="INVALID SERVICE";  break;
	case LL_ERROR_TIMEOUT:                     pcError="TIMEOUT";          break;
	case LL_ERROR_INVALID_PARAMETER:           pcError="INVALID PARAM";    break;
	case LL_ERROR_NOT_CONNECTED:               pcError="NOT CONNECT";      break;
	case LL_ERROR_ALREADY_CONNECTED:           pcError="ALREADY CONNECT";  break;
	case LL_ERROR_DISCONNECTED:                pcError="DISCONNECT";       break;
	case LL_ERROR_BUSY:                        pcError="BUSY";             break;
	case LL_ERROR_SERVICE_FORBIDDEN:           pcError="SERVICE REFUSED";  break;
	case LL_ERROR_NOT_READY:                   pcError="NOT READY";        break;
	case LL_ERROR_NETWORK_NOT_READY:           pcError="NETWRK NOT READY"; break;
	case LL_ERROR_NETWORK_NOT_SUPPORTED:       pcError="INVALID NETWORK";  break;
	case LL_ERROR_NETWORK_ALREADY_CONNECTED:   pcError="NETWORK CONNECT";  break;
	case LL_ERROR_NETWORK_ERROR:               pcError="NETWORK ERROR";    break;
	case LL_ERROR_DLL:                         pcError="SSL DLL MISSING";  break;
	case LL_ERROR_OUTPUT_BUFFER_TOO_SHORT:     pcError="BUFFER TOO SHORT"; break;
	case LL_ERROR_SERVICE_CALL_FAILURE:        pcError="NO LINK LAYER";    break;
	case LL_ERROR_IP_VERSION_REQUIRED:         pcError="WRONG LINK LAYER"; break;
	// *** Physical layer errors ***
	case LL_ERROR_PHYSICAL_OVERRUN_BUFFER:     pcError="OVERRUN BUFFER";   break;
	case LL_ERROR_PHYSICAL_OVERRUN:            pcError="OVERRUN";          break;
	case LL_ERROR_PHYSICAL_FRAMING:            pcError="FRAMING ERROR";    break;
	case LL_ERROR_PHYSICAL_PARITY:             pcError="PARITY ERROR";     break;
	// *** Modem errors ***
	case LL_MODEM_ERROR_SEND_BUFFER_OVERRUN:   pcError="AT CMD OVERRUN";   break;
	case LL_MODEM_ERROR_RESPONSE_ERROR:        pcError="INVALID AT CMD";   break;
	case LL_MODEM_ERROR_RESPONSE_BLACKLISTED:  pcError="RESP BLACKLISTED"; break;
	case LL_MODEM_ERROR_RESPONSE_DELAYED:      pcError="RESP DELAYED";     break;
	case LL_MODEM_ERROR_RESPONSE_NO_DIALTONE:  pcError="NO DIALTONE";      break;
	case LL_MODEM_ERROR_RESPONSE_BUSY:         pcError="LINE BUSY";        break;
	case LL_MODEM_ERROR_RESPONSE_NO_ANSWER:    pcError="NO ANSWER";        break;
	case LL_MODEM_ERROR_RESPONSE_NO_CARRIER:   pcError="NO CARRIER";       break;
	case LL_MODEM_ERROR_RESPONSE_INVALID:      pcError="INVALID ANSWER";   break;
	// *** SDL errors ***
	case LL_HDLC_ERROR_CONNECT_TIMEOUT:        pcError="SDLC TIMEOUT";     break;
	// *** PPP errors ***
	case LL_ERROR_PPP_CONNECT:                 pcError="PPP CONNECT ERR";  break;
	case LL_ERROR_PPP_AUTHENTICATION:          pcError="PPP AUTH ERROR";   break;
	// *** TCP/IP errors ***
	case LL_ERROR_UNKNOWN_HOST:                pcError="UNKNOWN HOST";     break;
	case LL_ERROR_CREATE_SOCKET:               pcError="SOCK CREATE ERR";  break;
	case LL_ERROR_CANNOT_CONNECT:              pcError="SOCK CONNECT ERR"; break;
	case LL_ERROR_CONNECTION_REFUSED:          pcError="CONNECT REFUSED";  break;
	case LL_ERROR_HOST_UNREACHABLE:            pcError="HOST UNREACHABLE"; break;
	case LL_ERROR_NETWORK_UNREACHABLE:         pcError="NET UNREACHABLE";  break;
	// *** X28 errors ***
	case LL_X28_ERROR:                         pcError="X28 ERROR";        break;
	case LL_X28_LIB:                           pcError="X28 LIB";          break;
	case LL_X28_RESET:                         pcError="X28 RESET";        break;
	case LL_X28_CLR:                           pcError="X28 CLR";          break;
	// *** SSL errors ***
	case LL_ERROR_SSL_PROFILE:                 pcError="SSL PROFILE ERR";  break;
	// *** Other errors
	default:                                   pcError="ERROR UNKNOWN";    break;
	}
	
	dspLS(1, pcError);
	return;

}
#endif

void tcan0020(void) {
#ifdef __TEST__
    int ret;
    char *ptr;
    tBuffer buf;
    byte dat[128];

    bufInit(&buf, dat, 128);

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    
    ret = dspLS(0, "comStart...");
    CHECK(ret >= 0, lblKO);
    TRCDT("comStart beg");
    ret = comStart(chnPpp);
    CHECK(ret >= 0, lblKO);
    TRCDT("comStart end");
    ret = dspLS(0, "comStart OK");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "comSet...");
    CHECK(ret >= 0, lblKO);
    TRCDT("comSet beg");
    
   // ret = comSet("00041840555555|sunrise|freesurf|60");//swisse
   // ret = comSet("9,01841840555555|sunrise|freesurf|60|195.186.004.111");//swisse+DNS
   ret = comSet("111|test|1NgenicO|60|192.168.040.001");//swisse+DNS
      
    //ptr= "217.193.162.206|9001"; //swisse
    //ptr = "195.186.004.111|9001";
    //ptr = "test.ingenico.ch|9001"; //swisse+DNS
    ptr = "192.168.040.001|60";

    //ret = comSet("9,88280030|testpos|testpos|60");
    CHECK(ret >= 0, lblKO);    
    TRCDT("comSet end");
    ret = dspLS(1, "comSet OK");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, "comDial...");
    CHECK(ret >= 0, lblKO);
    //ptr = "10.0.53.108|6130";   //BKI Router

    ret = dspLS(3, ptr);
    CHECK(ret >= 0, lblKO);

    TRCDT("comDial beg");
    ret = comDial(ptr);
    TRCDT("comDial end");
    CHECK(ret >= 0, lblKO);
    ret = dspLS(3, "comDial OK");
    CHECK(ret >= 0, lblKO);

    
    bufCpy(&buf, "Test Msg for Switch", 19);
    ret = dspLS(0, "comSendBuf start");
    CHECK(ret >= 0, lblKO);
    TRCDT("comSendBuf beg");
    ret = comSendBuf(bufPtr(&buf), bufLen(&buf));
    CHECK(ret > 0, lblKO);
    TRCDT("comSendBuf end");
    ret = dspLS(0, "comSendBuf done");
    CHECK(ret >= 0, lblKO);
    
    ret = dspLS(1, "comRecvBuf start");
    CHECK(ret >= 0, lblKO);
    TRCDT("comRecvBuf beg");
    ret = comRecvBuf(&buf, 0, 10);
    CHECK(ret > 0, lblKO);
    TRCDT("comRecvBuf end");
    ret = dspLS(0, "comRecvBuf done");
    CHECK(ret >= 0, lblKO);
    ret = dspLS(1, (char *) bufPtr(&buf));
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
  	
    trcErr(ret);
    dspLS(0, "KO!");
    cnxError();
    tmrPause(3);
  lblEnd:
    tmrPause(2);
    comHangStart();
    comHangWait();
    comStop();
    dspStop();
#endif
}
