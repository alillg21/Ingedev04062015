#include <string.h>
#include <unicapt.h>

#include "sys.h"
#include "tst.h"

static code const char *srcFile = __FILE__;

//Unitary test case tcik0001.
//Testing GPRS connection with choosed IP adr.

void tcik0001(void) {
    int ret;
    char *ptr;
    tBuffer buf;
    byte dat[1024];

    //for tracing
    uint8 tucBuffer[50 + 1];
    uint16 usLength;
    int16 iRet;
    uint32 fid;

    bufInit(&buf, dat, 1024);
    bufReset(&buf);

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);
    trcMode(0xFF);
    trcS("GPRS: Start\n");
    //ptr= "0000|orange.fr|danfr|mq36noqt|82.124.225.95|110";
    ptr = "0000|orange.fr|danfr|mq36noqt|82.247.161.69|6789";
    //ptr= "0000|orange.fr|danfr|mq36noqt|212.27.42.12|110";
    trcFS("valptr: ptr=%s\n", ptr);
    //for tracing
    iRet = psyFileCreate("datafile.txt");
    if((iRet == RET_OK)         // Create a local data file
       || (iRet == PSY_FILE_ERR_FILE_EXISTS))   // This file already exists
    {
        fid = psyFileOpen("datafile.txt", "w"); // Open the file on writing mode
        if(fid > 0) {
            memset(tucBuffer, 0, sizeof(tucBuffer));    // Write to file
            strcpy(tucBuffer, ptr);
            usLength = strlen(tucBuffer);
            if((iRet = psyFileWrite(fid, tucBuffer, usLength)) == RET_OK) {
                iRet = psyFileClose(fid);   // Close file
            }
        } else {
            iRet = psyFileErrnoGet();
        }
    }
    ret = dspLS(0, "utaGclStart");
    CHECK(ret >= 0, lblKO);
    ret = dspLS(1, ptr);
    CHECK(ret >= 0, lblKO);
    ret = utaGclStart(gclGprs, ptr);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, "utaGclGprsDial...");
    CHECK(ret >= 0, lblKO);
    ret = utaGclDial();
    CHECK(ret >= 0, lblKO);
    do {
        ret = utaGclDialInfo();
        switch (ret) {
          case gclStaPreDial:
              ptr = "PreDial";
              break;
          case gclStaDial:
              ptr = "Dial";
              break;
          case gclStaConnect:
              ptr = "Connect";
              break;
          case gclStaLogin:
              ptr = "Login";
              break;
          case gclStaFinished:
              ptr = "Done";
              break;
          default:
              goto lblKO;
        }
        dspLS(2, ptr);
    } while(ret != gclStaFinished);
    ret = dspLS(2, "utaGclGprsDial OK");
    CHECK(ret >= 0, lblKO);
    ret = prtS("utaGclGprsDial OK");
    CHECK(ret >= 0, lblKO);
    /*
       ret= dspLS(0,"utaGclGprsRecv.."); CHECK(ret>=0,lblKO);
       ret= utaGclRecvBuf(&buf,0,100);
       ret= prtS((char *)buf.ptr);CHECK(ret>=0,lblKO);

       ret= dspLS(0,"Receiving done"); CHECK(ret>=0,lblKO);
     */
    ret = dspLS(2, "Send preparer");
    CHECK(ret >= 0, lblKO);
    bufReset(&buf);
    bufAppStr(&buf, (byte *)
              "01010400850503TERM0001000603password0007033816412431450008031000.000001\x0D\x0A");
    //ret= prtS(buf);CHECK(ret>=0,lblKO);

    fid = psyFileOpen("datafile.txt", "a+");    // Open the file on writing mode
    if(fid > 0) {
        memset(tucBuffer, 0, sizeof(tucBuffer));    // Write to file
        strcpy(tucBuffer, (char *) buf.ptr);
        usLength = strlen(tucBuffer);
        if((iRet = psyFileWrite(fid, tucBuffer, usLength)) == RET_OK) {
            iRet = psyFileClose(fid);   // Close file
        }
    } else {
        iRet = psyFileErrnoGet();
    }

    ret = dspLS(0, "utaGclGprsSend..");
    CHECK(ret >= 0, lblKO);
    ret = utaGclSendBuf(bufPtr(&buf), bufLen(&buf));
    CHECK(ret == bufLen(&buf), lblKO);

    ret = utaGclRecvBuf(&buf, 0, 100);
    ret = prtS((char *) buf.ptr);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, "Receiving done");
    CHECK(ret >= 0, lblKO);
    fid = psyFileOpen("datafile.txt", "a+");    // Open the file on writing mode
    if(fid > 0) {
        memset(tucBuffer, 0, sizeof(tucBuffer));    // Write to file
        strcpy(tucBuffer, (char *) buf.ptr);
        usLength = strlen(tucBuffer);
        if((iRet = psyFileWrite(fid, tucBuffer, usLength)) == RET_OK) {
            iRet = psyFileClose(fid);   // Close file
        }
    } else {
        iRet = psyFileErrnoGet();
    }

    ret = dspLS(3, "utaGclGprsStop");
    CHECK(ret >= 0, lblKO);
    ret = utaGclStop();
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = utaGclDialErr();
    trcFN("valGclDial: ret=%d\n", ret);
    switch (ret) {
      case gclErrCancel:
          ptr = "User cancelled";
          break;
      case gclErrDial:
          ptr = "Dial error";
          break;
      case gclErrBusy:
          ptr = "BUSY";
          break;
      case gclErrNoDialTone:
          ptr = "NO DIAL TONE";
          break;
      case gclErrNoCarrier:
          ptr = "NO CARRIER";
          break;
      case gclErrNoAnswer:
          ptr = "NO ANSWER";
          break;
      case gclErrLogin:
          ptr = "LOGIN PBM";
          break;
      default:
          ptr = "KO!";
          break;
    }
    dspLS(0, ptr);
  lblEnd:
    tmrPause(3);
    utaGclStop();
    prtStop();
    dspStop();
    trcS("GPRS: End\n");
}
