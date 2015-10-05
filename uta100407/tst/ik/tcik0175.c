/// \file tcik0175.c

/** non-UTA test case for integration of Wifi CMM.
//WiFi Tags
#define TAG_CMM_WIFI_PRESENT			TAG_CMM_LEAF"\x81"	// uint8
#define TAG_CMM_WIFI_SIGNAL_STRENGTH	TAG_CMM_LEAF"\x82"	// uint8
#define TAG_CMM_WIFI_MODE_ADHOC			TAG_CMM_LEAF"\x83"	// uint8 True/False [FALSE]
#define TAG_CMM_WIFI_SSID_REMOTE		TAG_CMM_LEAF"\x84"	// uint8 *
#define TAG_CMM_WIFI_SSID_LOCAL			TAG_CMM_LEAF"\x85"	// uint8 * ["IngWiFi"]
#define TAG_CMM_WIFI_CHANNEL			TAG_CMM_LEAF"\x86"	// uint8 [1]
#define TAG_CMM_WIFI_AUTH_SHARED_KEY	TAG_CMM_LEAF"\x87"	// uint8 True/False [FALSE]
#define TAG_CMM_WIFI_WEP_ENABLED		TAG_CMM_LEAF"\x88"	// uint8 True/False [FALSE]
#define TAG_CMM_WIFI_WEP_KEY_TYPE		TAG_CMM_LEAF"\x89"	// uint8 0=64, 1=128 [0]
#define TAG_CMM_WIFI_WEP_KEY_DATA		TAG_CMM_LEAF"\x8A"	// uint8 *

 */

#include <stdarg.h>
#include "string.h"
#include "unicapt.h"
#include "tlv.h"
#include "cmm.h"
#include "tagcmm.h"

static uint32 devHandle;

static int16 TestPrintf_tcik0175(char *format, ...) {
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

int SetWifi_tcik0175() {
    int16 ret;
    tlv_t tlv;
    uint8 tagBuff[512];
    uint8 rsp[5];
    uint8 net_id[64];
    uint8 WifiKey[5];
    uint8 buff[32];
    uint8 sign;

    memset(rsp, 0, sizeof(rsp));

    tlv_init(&tlv, tagBuff, sizeof(tagBuff));
    tlv_add_value8(&tlv, TAG_CMM_COMMAND, CMM_COMMAND_WIFI_INFO_GET);
    ret = cmmGeneral(&tlv);
    TestPrintf_tcik0175("info %d", ret);
    if(ret != 0)
        goto lblKO;

    ret = tlv_tag_search(&tlv, TAG_CMM_WIFI_PRESENT);
    if(ret != 0)
        goto lblKO;
    tlv_get_valueBytes(&tlv, 0, rsp, 1);
    TestPrintf_tcik0175("Wifi rsp %d", rsp[0]);

    tlv_init(&tlv, tagBuff, sizeof(tagBuff));
    tlv_add_value8(&tlv, TAG_CMM_COMMAND, CMM_COMMAND_WIFI_INFO_SET);
    memcpy(net_id, "linksys", 7);   //"IngWiFi"
    tlv_add_valueBytes(&tlv, TAG_CMM_WIFI_SSID_REMOTE, strlen(net_id), net_id);
    //tlv_add_value8(&tlv, TAG_CMM_WIFI_MODE_ADHOC, TRUE);
    tlv_add_value8(&tlv, TAG_CMM_WIFI_AUTH_SHARED_KEY, TRUE);
    tlv_add_value8(&tlv, TAG_CMM_WIFI_WEP_ENABLED, TRUE);
    tlv_add_value8(&tlv, TAG_CMM_WIFI_CHANNEL, 11);
    memcpy(WifiKey, "\x0D\x0A\x36\xF6\xDA", 5);
    tlv_add_valueBytes(&tlv, TAG_CMM_WIFI_WEP_KEY_DATA, 5, WifiKey);

    ret = cmmGeneral(&tlv);
    TestPrintf_tcik0175("WifiSet %d", ret);
    if(ret != 0)
        goto lblKO;

    tlv_init(&tlv, tagBuff, sizeof(tagBuff));
    tlv_add_value8(&tlv, TAG_CMM_COMMAND, CMM_COMMAND_WIFI_SIGNAL_STRENGTH);
    ret = cmmGeneral(&tlv);
    TestPrintf_tcik0175("WifiSign %d", ret);
    if(ret < 0)
        goto lblKO;

    tlv_init(&tlv, tagBuff, sizeof(tagBuff));
    tlv_add_value8(&tlv, TAG_CMM_COMMAND, CMM_COMMAND_WIFI_INFO_GET);
    ret = cmmGeneral(&tlv);
    TestPrintf_tcik0175("WifiGet %d", ret);
    if(ret != 0)
        goto lblKO;
    ret = tlv_tag_search(&tlv, TAG_CMM_WIFI_SIGNAL_STRENGTH);
    TestPrintf_tcik0175("search sign %d", ret);
    if(!ret) {
        sign = tlv_get_value8(&tlv, 0);
        TestPrintf_tcik0175("Wifi sign %d", sign);
    }

    tlv_init(&tlv, tagBuff, sizeof(tagBuff));
    tlv_add_value8(&tlv, TAG_CMM_COMMAND, CMM_COMMAND_ETH_INFO_SET);
    tlv_add_value8(&tlv, TAG_CMM_ETH_DHCP, TRUE);

    ret = cmmGeneral(&tlv);
    TestPrintf_tcik0175("eis %d", ret);
    if(ret != 0)
        goto lblKO;

    tlv_init(&tlv, tagBuff, sizeof(tagBuff));
    tlv_add_value8(&tlv, TAG_CMM_COMMAND, CMM_COMMAND_LNET_UP);
    ret = cmmGeneral(&tlv);
    TestPrintf_tcik0175("netUp %d", ret);

    tlv_init(&tlv, tagBuff, sizeof(tagBuff));
    tlv_add_value8(&tlv, TAG_CMM_COMMAND, CMM_COMMAND_ETH_INFO_GET);
    ret = cmmGeneral(&tlv);
    TestPrintf_tcik0175("eig %d", ret);
    if(ret != 0)
        goto lblKO;
    if(tlv_tag_search(&tlv, TAG_CMM_ETH_ADDRESS) == 0) {
        tlv_get_valueBytes(&tlv, 0, buff, 4);
        TestPrintf_tcik0175("MyIP %d.%d.%d.%d", buff[0], buff[1], buff[2],
                            buff[3]);
    }
    goto lblEnd;
  lblKO:
    TestPrintf_tcik0175("ETH layer KO");
    return -1;
  lblEnd:
    return 1;

}

int DialWifi_tcik0175() {

}

void tcik0175() {
    int16 ret;
    uint8 buf[512];
    uint16 len;

    ret = SetWifi_tcik0175();
    //if(ret<0) goto lblKO;

    ret = DialWifi_tcik0175();
    //if(ret<0) goto lblKO;

    memset(buf, 0, sizeof(buf));
    memcpy(buf,
           "\x60\x06\x65\x06\x65\x02\x00\x30\x20\x05\x80\x20\xC0\x00\x04\x00\x40\x00\x00\x00\x00\x12\x11\x50\x00\x36\x09\x00\x22\x00\x12\x00\x37\x02\x01\x02\x00\x28\x42\x07\x80\xD0\x70\x20\x33\x00\x01\x23\x40\x10\x00\x00\x32\x31\x31\x39\x30\x39\x31\x38\x30\x30\x30\x30\x30\x31\x30\x34\x30\x32\x31\x37\x32\x36\x34\x00\x06\x30\x30\x31\x32\x34\x32",
           83);

    ret = cmmSend(devHandle, buf, 83);
    TestPrintf_tcik0175("cmmSend %d", ret);
    if(ret < 0)
        goto lblKO;

    ret = cmmRecv(devHandle, buf, &len, sizeof(buf));
    TestPrintf_tcik0175("cmmRecv %d", ret);
    if(ret < 0)
        goto lblKO;

  lblKO:
    TestPrintf_tcik0175("Test KO");
  lblEnd:
    ret = cmmEnd(devHandle);
    TestPrintf_tcik0175("cmmEnd %d", ret);

}
