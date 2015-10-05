/// \file tcik0172.c

/** non-UTA test case for integration of SSL CMM.
- PC with Server must have fixed IP adress !!!
 */
#include <stdarg.h>
#include "string.h"
#include "unicapt.h"
#include "tlv.h"
#include "cmm.h"
#include "tagcmm.h"
#include "lnetssl.h"

static uint32 devHandle;

static int16 TestPrintf_tcik0172(char *format, ...) {
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

int SetSSL_tcik0172() {
    int16 ret;
    tlv_t tlv;
    uint8 tagBuff[512];
    uint8 buff[5];

    tlv_init(&tlv, tagBuff, sizeof(tagBuff));
    tlv_add_value8(&tlv, TAG_CMM_COMMAND, CMM_COMMAND_ETH_INFO_SET);
    tlv_add_value8(&tlv, TAG_CMM_ETH_DHCP, TRUE);

    ret = cmmGeneral(&tlv);
    TestPrintf_tcik0172("eis %d", ret);
    if(ret != 0)
        goto lblKO;

    tlv_init(&tlv, tagBuff, sizeof(tagBuff));
    tlv_add_value8(&tlv, TAG_CMM_COMMAND, CMM_COMMAND_LNET_UP);
    ret = cmmGeneral(&tlv);
    TestPrintf_tcik0172("lnetUp %d", ret);
    if(ret != 0)
        goto lblKO;

    tlv_init(&tlv, tagBuff, sizeof(tagBuff));
    tlv_add_value8(&tlv, TAG_CMM_COMMAND, CMM_COMMAND_ETH_INFO_GET);
    ret = cmmGeneral(&tlv);
    TestPrintf_tcik0172("eig %d", ret);
    if(ret != 0)
        goto lblKO;
    if(tlv_tag_search(&tlv, TAG_CMM_ETH_ADDRESS) == 0) {
        tlv_get_valueBytes(&tlv, 0, buff, 4);
        TestPrintf_tcik0172("MyIP %d.%d.%d.%d", buff[0], buff[1], buff[2],
                            buff[3]);
    }
    goto lblEnd;
  lblKO:
    TestPrintf_tcik0172("ETH layer KO");
    return -1;
  lblEnd:
    return 1;
}

int DialSSL_tcik0172() {
    int16 ret;
    tlv_t tlv;
    uint8 tagBuff[512];
    uint8 ipHost[5];

    tlv_init(&tlv, tagBuff, sizeof(tagBuff));
    tlv_add_value8(&tlv, TAG_CMM_DEVICE_TYPE, cmmDeviceEthClient);
    tlv_add_value8(&tlv, TAG_CMM_PROTOCOL, cmmProtocolTCP);
    tlv_add_value8(&tlv, TAG_CMM_LENGTH_FORMAT, cmmLengthBCD);

    memcpy(ipHost, "\x0a\x11\x20\x72", 4);  //10.17.32.114
    tlv_add_valueBytes(&tlv, TAG_CMM_ETH_HOST_ADDR_PRI, 4, ipHost);
    tlv_add_value16Little(&tlv, TAG_CMM_ETH_HOST_PORT_PRI, 443);

    memcpy(ipHost, "\x0a\x11\x20\x72", 4);  //10.17.32.114
    tlv_add_valueBytes(&tlv, TAG_CMM_ETH_HOST_ADDR_SEC, 4, ipHost);
    tlv_add_value16Little(&tlv, TAG_CMM_ETH_HOST_PORT_SEC, 443);

    tlv_add_value16Little(&tlv, TAG_CMM_TIMEOUT_RECEIVE, 500);
    tlv_add_value16Little(&tlv, TAG_CMM_TIMEOUT_CONNECT, 1000);

    tlv_add_value8(&tlv, TAG_CMM_ETH_SSL, TRUE);
    tlv_add_value8(&tlv, TAG_CMM_ETH_SSL_METHOD, SSL_V3_CLIENT);
    //tlv_add_value8(&tlv, TAG_CMM_ETH_SSL_METHOD, SSL_AUTO_CLIENT);
    //tlv_add_value8(&tlv, TAG_CMM_ETH_SSL_CIPHER_AUTO, TRUE); //default value

    ret = cmmInit(&devHandle, &tlv);
    TestPrintf_tcik0172("cmmInit %d", ret);
    if(ret != 0)
        goto lblKO;

    goto lblEnd;
  lblKO:
    TestPrintf_tcik0172("IP layer KO");
    ret = -1;
  lblEnd:
    return ret;
}

void tcik0172() {
    int16 ret;
    uint8 buf[512];
    uint16 len;

    ret = SetSSL_tcik0172();
    if(ret < 0)
        goto lblKO;

    ret = DialSSL_tcik0172();
    if(ret < 0)
        goto lblKO;

    memset(buf, 0, sizeof(buf));
    memcpy(buf,
           "\x60\x06\x65\x06\x65\x02\x00\x30\x20\x05\x80\x20\xC0\x00\x04\x00\x40\x00\x00\x00\x00\x12\x11\x50\x00\x36\x09\x00\x22\x00\x12\x00\x37\x02\x01\x02\x00\x28\x42\x07\x80\xD0\x70\x20\x33\x00\x01\x23\x40\x10\x00\x00\x32\x31\x31\x39\x30\x39\x31\x38\x30\x30\x30\x30\x30\x31\x30\x34\x30\x32\x31\x37\x32\x36\x34\x00\x06\x30\x30\x31\x32\x34\x32",
           83);

    ret = cmmSend(devHandle, buf, 83);
    TestPrintf_tcik0172("cmmSend %d", ret);
    if(ret < 0)
        goto lblKO;

    ret = cmmRecv(devHandle, buf, &len, sizeof(buf));
    TestPrintf_tcik0172("cmmRecv %d", ret);
    if(ret < 0)
        goto lblKO;

  lblKO:
    TestPrintf_tcik0172("Test KO");
  lblEnd:
    ret = cmmEnd(devHandle);
    TestPrintf_tcik0172("cmmEnd %d", ret);

}
