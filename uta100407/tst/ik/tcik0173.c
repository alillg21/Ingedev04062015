/// \file tcik0173.c

/** non-UTA test case for integration of GPRS CMM.
 */
#include <stdarg.h>
#include "string.h"
#include "unicapt.h"
#include "tlv.h"
#include "cmm.h"
#include "tagcmm.h"
static uint32 devHandle;

static int16 TestPrintf_tcik0173(char *format, ...) {
    char buffer[48 + 1];
    int16 ret;
    va_list args;
    uint32 handle;

    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);

    prnOpen("DEFAULT", &handle, PRN_IMMEDIATE);
    ret = prnPrintWait(handle, buffer);
    prnClose(handle);

    return ret;
}

int SetGprs_tcik0173() {
    int16 ret;
    tlv_t tlv;
    uint8 tagBuff[512];
    uint8 NbTry;

    char *username = "";
    char *password = "";
    uint8 gStatus;

    //set gprs parameters
    tlv_init(&tlv, tagBuff, sizeof(tagBuff));
    tlv_add_value8(&tlv, TAG_CMM_COMMAND, CMM_COMMAND_GSM_GPRS_INFO_SET);
    tlv_add_valueBytes(&tlv, TAG_CMM_GPRS_APN, 21, "internet.globe.com.ph");
    if(*username) {
        tlv_add_valueBytes(&tlv, TAG_CMM_USERNAME, strlen(username), username);
    }
    if(*password) {
        tlv_add_valueBytes(&tlv, TAG_CMM_PASSWORD, strlen(password), password);
    }
    ret = cmmGeneral(&tlv);
    TestPrintf_tcik0173("gis %d", ret);
    if(ret != 0)
        return;

    //attach to gprs network
    tlv_init(&tlv, tagBuff, sizeof(tagBuff));
    tlv_add_value8(&tlv, TAG_CMM_COMMAND, CMM_COMMAND_GSM_UP);
    ret = cmmGeneral(&tlv);
    TestPrintf_tcik0173("gsmUp %d", ret);
    switch (ret) {
      case cmmErrGprsAttachInProgress:
      case cmmSuccess:
          break;
      default:
          break;
    }
    if(ret != 0)
        return;

    NbTry = 5;
    while(1) {
        tlv_init(&tlv, tagBuff, sizeof(tagBuff));
        tlv_add_value8(&tlv, TAG_CMM_COMMAND, CMM_COMMAND_GSM_GPRS_INFO_GET);
        ret = cmmGeneral(&tlv);
        TestPrintf_tcik0173("infoget =%d", ret);

        ret = tlv_tag_search(&tlv, TAG_CMM_GSM_GPRS_STATUS);
        if(ret != 0)
            goto lblKO;

        gStatus = tlv_get_value8(&tlv, 0);
        TestPrintf_tcik0173("Gprs stat=%d", gStatus);

        switch (gStatus) {
          case cmmGprsConnected:
              goto lblEnd;
          case cmmGprsGsmDown:
              NbTry--;
              if(!NbTry)
                  goto lblKO;
          default:
              break;
        }

        psyTimerWakeAfter(150);
    }
    goto lblEnd;
  lblKO:
    TestPrintf_tcik0173("GPRS layer KO");
    return -1;
  lblEnd:
    return 1;

}

int DialGprs_tcik0173() {
    int16 ret;
    tlv_t tlv;
    uint8 tagBuff[512];

    uint8 ipHost[5];
    uint8 IpSta;
    uint8 GprsSta;

    //TCP/IP layer
    tlv_init(&tlv, tagBuff, sizeof(tagBuff));
    tlv_add_value8(&tlv, TAG_CMM_DEVICE_TYPE, cmmDeviceEthClient);
    tlv_add_value8(&tlv, TAG_CMM_PROTOCOL, cmmProtocolTCP);
    tlv_add_value8(&tlv, TAG_CMM_LENGTH_FORMAT, cmmLengthNone); //TO keep in UTA implementation

    //memcpy(ipHost, "\xcb\x75\x5b\xb5", 4);        //203.117.91.181
    memcpy(ipHost, "\xd2\x01\x4e\x4e", 4);  //210.1.78.78
    tlv_add_valueBytes(&tlv, TAG_CMM_ETH_HOST_ADDR_PRI, 4, ipHost);
    tlv_add_value16Little(&tlv, TAG_CMM_ETH_HOST_PORT_PRI, 60);

    //memcpy(ipHost, "\xcb\x75\x5b\xb5", 4);        //203.117.91.181
    memcpy(ipHost, "\xd2\x01\x4e\x4e", 4);  //210.1.78.78
    tlv_add_valueBytes(&tlv, TAG_CMM_ETH_HOST_ADDR_SEC, 4, ipHost);
    tlv_add_value16Little(&tlv, TAG_CMM_ETH_HOST_PORT_SEC, 60);

    //memcpy(ipHost, "\xcb\x75\x5b\xb5", 4);        //203.117.91.181
    memcpy(ipHost, "\xd2\x01\x4e\x4e", 4);  //210.1.78.78
    tlv_add_valueBytes(&tlv, TAG_CMM_ETH_HOST_ADDR_TER, 4, ipHost);
    tlv_add_value16Little(&tlv, TAG_CMM_ETH_HOST_PORT_TER, 60);

    tlv_add_value16Little(&tlv, TAG_CMM_TIMEOUT_RECEIVE, 500);
    tlv_add_value16Little(&tlv, TAG_CMM_TIMEOUT_CONNECT, 1500);

    ret = cmmInit(&devHandle, &tlv);
    TestPrintf_tcik0173("cmmInit %d", ret);
    if(ret != 0)
        goto lblKO;

    TestPrintf_tcik0173("Checking IP/GPRS status");
    //are we connected to TCP/IP layer?
    tlv_init(&tlv, tagBuff, sizeof(tagBuff));

    while(1) {
        ret = cmmStatus(devHandle, &tlv);
        TestPrintf_tcik0173("cmmStatus %d", ret);
        if(ret != 0)
            goto lblKO;

        ret = tlv_tag_search(&tlv, TAG_CMM_STATUS_CONNECTION);
        if(ret != 0)
            goto lblKO;

        IpSta = tlv_get_value8(&tlv, 0);
        TestPrintf_tcik0173("ip status %d", IpSta);
        switch (IpSta) {
          case cmmConnectedPrimary:
          case cmmConnectedSecondary:
          case cmmConnectedTertiary:
          case cmmConnectedPeer:
              ret = 1;
              goto lblEnd;
          default:
              break;
        }

        ret = tlv_tag_search(&tlv, TAG_CMM_GSM_GPRS_STATUS);
        if(ret != 0)
            goto lblKO;

        GprsSta = tlv_get_value8(&tlv, 0);
        TestPrintf_tcik0173("gprs status %d", GprsSta);

        if(GprsSta == cmmGprsGsmDown)
            goto lblKO;

        psyTimerWakeAfter(100);
    }
    TestPrintf_tcik0173("Connected to %d", IpSta);
    ret = 1;
    goto lblEnd;
  lblKO:
    TestPrintf_tcik0173("IP layer KO");
    ret = -1;
  lblEnd:
    return ret;
}

void tcik0173() {
    int16 ret;
    uint8 buf[512];

    //uint16 len;

    ret = SetGprs_tcik0173();
    if(ret <= 0)
        goto lblKO;

    ret = DialGprs_tcik0173();
    if(ret <= 0)
        goto lblKO;

    memset(buf, 0, sizeof(buf));
    memcpy(buf,
           "\x60\x06\x65\x06\x65\x02\x00\x30\x20\x05\x80\x20\xC0\x00\x04\x00\x40\x00\x00\x00\x00\x12\x11\x50\x00\x36\x09\x00\x22\x00\x12\x00\x37\x02\x01\x02\x00\x28\x42\x07\x80\xD0\x70\x20\x33\x00\x01\x23\x40\x10\x00\x00\x32\x31\x31\x39\x30\x39\x31\x38\x30\x30\x30\x30\x30\x31\x30\x34\x30\x32\x31\x37\x32\x36\x34\x00\x06\x30\x30\x31\x32\x34\x32",
           83);
    ret = cmmSend(devHandle, buf, 83);
    TestPrintf_tcik0173("cmmSend %d", ret);
    if(ret <= 0)
        goto lblKO;

//  ret = cmmRecv(devHandle, buf, &len, sizeof(buf));
//  TestPrintf_tcik0173("cmmRecv %d", ret);
//  if(ret<0) goto lblKO;

  lblKO:
    TestPrintf_tcik0173("Test KO");
  lblEnd:
    ret = cmmEnd(devHandle);
    TestPrintf_tcik0173("cmmEnd %d", ret);
}
