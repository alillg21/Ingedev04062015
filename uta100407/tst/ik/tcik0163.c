/// \file tcik0163.c

#include <string.h>
#include "tst.h"

/** Non-UTA test case for integration of cmm module.
 */
#ifdef __CMM__
#include "cmm.h"
#include "tagcmm.h"

static uint32 hHmi = 0;
static uint32 hPrt = 0;
static void tc_show(const char *str, int idx) {
    static uint16 row = 0;

    if(idx == 0)
        row = 0;
    hmiADClearLine(hHmi, row);
    hmiADDisplayText(hHmi, row, 0, (char *) str);
    row += 1;
    if(row >= 15)
        row = 0;
}

static void tc_print(const char *str) {
    prnPrint(hPrt, (char *) str);
}

void tcik0163(void) {
    int ret;
    tlv_t IpTags;

    //tlv_t * pTags = &tags;
    tlv_t tlv;
    uint32 deviceHandle;
    uint8 tlvBuffer[512];
    uint8 IpBuffer[512];
    char text[16 + 1];
    uint8 msg[128];
    uint16 len;
    uint8 connType;
    uint16 IpAdr[4];
    uint8 IPAddress[4] = { 10, 110, 1, 125 };
    uint8 SubnetMask[4] = { 255, 255, 255, 0 };
    uint8 Gateway[4] = { 10, 110, 1, 1 };

    hmiOpen("DEFAULT", &hHmi);
    prnOpen("DEFAULT", &hPrt, PRN_IMMEDIATE);
    tc_show("fill parameters", 1);
    tlv_init(&tlv, tlvBuffer, sizeof(tlvBuffer));

    //ret = cmmEnd(NULL);

    connType = 1;
    switch (connType) {
      case 0:                  //modem
          tlv_add_value8(&tlv, TAG_CMM_DEVICE_TYPE, cmmDeviceModem);
          tlv_add_value8(&tlv, TAG_CMM_PROTOCOL, cmmProtocolTransparent);
          tlv_add_value16Little(&tlv, TAG_CMM_TIMEOUT_RECEIVE, 3000);   //30secs
          tlv_add_value16Little(&tlv, TAG_CMM_TIMEOUT_CONNECT, 4500);   //45secs
          tlv_add_value8(&tlv, TAG_CMM_DIAL_BLIND, TRUE);
          tlv_add_value8(&tlv, TAG_CMM_DIAL_PULSE, FALSE);
          tlv_add_value8(&tlv, TAG_CMM_DIAL_FASTCONNECT, FALSE);
          tlv_add_value8(&tlv, TAG_CMM_DIAL_SPEED, COM_BPS_DEFAULT);
          tlv_add_value8(&tlv, TAG_CMM_DIAL_COMPRESSION_TYPE,
                         COM_COMPRESSION_DEFAULT);
          tlv_add_value8(&tlv, TAG_CMM_DIAL_CORRECTION_TYPE,
                         COM_CORRECTION_DEFAULT);
          tlv_add_value8(&tlv, TAG_CMM_DIAL_BACKGROUND, FALSE);
          tlv_add_valueBytes(&tlv, TAG_CMM_DIAL_NUMBER_PRI, 4, "*808");
          tlv_add_value8(&tlv, TAG_CMM_CONNECT_ATTEMPTS_PRI, 2);
          tlv_add_valueBytes(&tlv, TAG_CMM_DIAL_NUMBER_SEC, 4, "*808");
          tlv_add_value8(&tlv, TAG_CMM_CONNECT_ATTEMPTS_SEC, 2);
          tlv_add_valueBytes(&tlv, TAG_CMM_DIAL_NUMBER_TER, 4, "*808");
          tlv_add_value8(&tlv, TAG_CMM_CONNECT_ATTEMPTS_TER, 2);

          tlv_add_valueBytes(&tlv, TAG_CMM_DIAL_NUMBER_PRI, 11, "00836065555");
          tlv_add_value8(&tlv, TAG_CMM_CONNECT_ATTEMPTS_PRI, 1);

          tc_show("call cmmInit", 2);
          ret = cmmInit(&deviceHandle, &tlv);
          break;
      case 1:                  //ethernet
          deviceHandle = 0;
          /*
             memset(IpBuffer, 0, sizeof(IpBuffer));
             ret = tlv_init(&IpTags, IpBuffer, sizeof(IpBuffer));
             ret = tlv_add_value8(&IpTags, TAG_CMM_COMMAND, CMM_COMMAND_ETH_INFO_GET); 
             ret = cmmGeneral(&IpTags);   
             ret = tlv_tag_search(&IpTags, TAG_CMM_ETH_PRESENT);
             ret = tlv_get_value8(&IpTags, 0);
             if(ret != TRUE)
             goto lblKO;
           */
          //Set IP level config
          tlv_init(&IpTags, IpBuffer, sizeof(IpBuffer));
          tlv_add_value8(&IpTags, TAG_CMM_COMMAND, CMM_COMMAND_ETH_INFO_SET);

          ret = tlv_add_value8(&IpTags, TAG_CMM_ETH_DHCP, TRUE);
          //ret = tlv_add_valueBytes(&IpTags, TAG_CMM_ETH_ADDRESS, sizeof(IPAddress), IPAddress);
          //ret = tlv_add_valueBytes(&IpTags, TAG_CMM_ETH_SUBNET_MASK, sizeof(SubnetMask), SubnetMask);
          //ret = tlv_add_valueBytes(&IpTags, TAG_CMM_ETH_GATEWAY, sizeof(Gateway), Gateway);

          ret = cmmGeneral(&IpTags);
          memset(text, 0, sizeof(text));
          sprintf(text, "Set DHCP %u", ret);
          tc_print(text);
          if(ret != 0)
              goto lblKO;

          //START IP level
          tlv_init(&tlv, tlvBuffer, sizeof(tlvBuffer));
          tlv_add_value8(&tlv, TAG_CMM_COMMAND, CMM_COMMAND_LNET_UP);
          ret = cmmGeneral(&tlv);
          memset(text, 0, sizeof(text));
          sprintf(text, "lnetUp %u", ret);
          tc_print(text);
          if(ret != 0)
              goto lblKO;
          /*
             //Get Ethernet info
             tlv_init(&IpTags, IpBuffer, sizeof(IpBuffer));
             tlv_add_value8(&IpTags, TAG_CMM_COMMAND, CMM_COMMAND_ETH_INFO_GET);
             ret = cmmGeneral(&IpTags);
             memset(text, 0, sizeof(text));
             sprintf(text,"eth info %u", ret);
             tc_print(text);
             if( tlv_tag_search(&IpTags, TAG_CMM_ETH_ADDRESS) == 0 ){
             memset(IpAdr, 0, sizeof(IpAdr));
             ret = tlv_get_valueBytes(&IpTags, 0, IpAdr, 4);
             memset(text, 0, sizeof(text));
             sprintf(text,"MyIP %u.%u.%u.%u", LBYTE(IpAdr[0]), HBYTE(IpAdr[0]), LBYTE(IpAdr[1]), HBYTE(IpAdr[1]));
             tc_print(text);
             }
             if(tlv_tag_search(&IpTags, TAG_CMM_ETH_PRESENT)==0){
             ret = tlv_get_value8(&IpTags, 0); 
             memset(text, 0, sizeof(text));
             sprintf(text,"eth  %u", ret);
             tc_print(text);
             }
             if(tlv_tag_search(&IpTags, TAG_CMM_ETH_CABLE_CONNECTED)==0){
             ret = tlv_get_value8(&IpTags, 0); 
             memset(text, 0, sizeof(text));
             sprintf(text,"connect %u", ret);
             tc_print(text);
             if(ret != 1)
             goto lblKO;
             }
           */
          ret = tlv_init(&tlv, tlvBuffer, sizeof(tlvBuffer));
          tlv_init(&IpTags, IpBuffer, sizeof(IpBuffer));

          ret = tlv_add_value8(&tlv, TAG_CMM_DEVICE_TYPE, cmmDeviceEthClient);
          ret = tlv_add_value8(&tlv, TAG_CMM_PROTOCOL, cmmProtocolTCP);

          memset(IpAdr, 0, sizeof(IpAdr));
          //memcpy(IpAdr, "\x7f\x00\x00\x01", 4); //127.0.0.1
          memcpy(IpAdr, "\x0a\x6e\x01\x68", 4); //10.110.1.104

          ret = tlv_add_valueBytes(&tlv, TAG_CMM_ETH_HOST_ADDR_PRI, 4, IpAdr);
          ret = tlv_add_value16Little(&tlv, TAG_CMM_ETH_HOST_PORT_PRI, 9702);
          ret = tlv_add_value8(&tlv, TAG_CMM_LENGTH_FORMAT, cmmLengthNone);
          ret = tlv_add_value8(&tlv, TAG_CMM_DIAL_BACKGROUND, FALSE);

          tc_show("call cmmInit", 2);
          ret = cmmInit(&deviceHandle, &tlv);
          break;
      default:
          goto lblKO;
    }
    memset(text, 0, sizeof(text));
    sprintf(text, "cmmInit=%d", ret);
    tc_print(text);
    if(ret != 0)
        goto lblKO;
    //if(deviceHandle==0)
    //  goto lblKO;
    memset(msg, 0, sizeof(msg));
    len = 82;
    memcpy(msg,
           "\x60\x04\x15\x30\x00\x02\x00\x30\x00\x05\x80\x00\xC0\x00\x04\x00\x00\x00\x00\x00\x00\x06UU\x00\x00\x02\x00.\x04\x15\x007Ir\x03vA\x05\x17\x17\xD0\x81!\x05`F@$P\x00\x0F00000000000000000000000\x00\x06000008",
           len);
    ret = cmmSend(deviceHandle, msg, len);
    memset(text, 0, sizeof(text));
    sprintf(text, "cmmSend=%d", ret);
    tc_print(text);
    if(ret)
        goto lblKO;

    memset(text, 0, sizeof(text));
    ret = cmmRecv(deviceHandle, text, &len, sizeof(text));
    memset(text, 0, sizeof(text));
    sprintf(text, "cmmRecv=%d", ret);
    tc_print(text);
    if(ret)
        goto lblKO;

    goto lblEnd;

  lblKO:
    tc_print("tcik0163 KO");
  lblEnd:
    //STOP IP level
    tlv_init(&tlv, tlvBuffer, sizeof(tlvBuffer));
    tlv_add_value8(&tlv, TAG_CMM_COMMAND, CMM_COMMAND_LNET_DOWN);
    ret = cmmGeneral(&tlv);
    memset(text, 0, sizeof(text));
    sprintf(text, "lnetDown %u", ret);
    tc_print(text);
    if(ret != 0)
        goto lblKO;

    ret = cmmEnd(deviceHandle);
    memset(text, 0, sizeof(text));
    sprintf(text, "cmmEnd=%d", ret);
    tc_print(text);
    tc_print("tcik0163 End");
    hmiClose(hHmi);
    prnClose(hPrt);
    return;
}
#else
void tcik0163(void) {
}
#endif
