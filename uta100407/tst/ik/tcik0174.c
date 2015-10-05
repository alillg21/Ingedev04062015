/// \file tcik0174.c

/** non-UTA test case for integration of RS232 CMM.
 */

#include <stdarg.h>
#include "string.h"
#include "unicapt.h"
#include "tlv.h"
#include "cmm.h"
#include "tagcmm.h"

static uint32 devHandle;

static int16 TestPrintf_tcik0174(char *format, ...) {
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

int SetRS232_tcik0174() {
    int16 ret;
    tlv_t tlv;
    uint8 tagBuff[512];

    tlv_init(&tlv, tagBuff, sizeof(tagBuff));
    tlv_add_value8(&tlv, TAG_CMM_DEVICE_TYPE, cmmDeviceRS232);

    tlv_add_value8(&tlv, TAG_CMM_RS232_PORT, 1);    //COM1 = 1, COM2 = 2
    tlv_add_value8(&tlv, TAG_CMM_DEBUG_ENABLED, TRUE);

    tlv_add_value16Little(&tlv, TAG_CMM_TIMEOUT_RECEIVE, 1500);
    tlv_add_value16Little(&tlv, TAG_CMM_TIMEOUT_INTER_CHARACTER, 10);
    tlv_add_value8(&tlv, TAG_CMM_RS232_SPEED, COM_BAUD_9600);
    tlv_add_value8(&tlv, TAG_CMM_RS232_PARITY, COM_PARITY_NONE);
    tlv_add_value8(&tlv, TAG_CMM_RS232_DATASIZE, COM_DATASIZE_8);
    tlv_add_value8(&tlv, TAG_CMM_RS232_STOPBITS, COM_STOP_1);
    tlv_add_value8(&tlv, TAG_CMM_RS232_FLOW_CONTROL, COM_FLOW_NONE);
    tlv_add_value8(&tlv, TAG_CMM_PROTOCOL, cmmProtocolAsyncLength);

//  tlv_add_value8(&tlv, TAG_CMM_PROTOCOL, cmmProtocolAsyncNone);
//  tlv_add_value8(&tlv, TAG_CMM_PROTOCOL, cmmProtocolAsyncDle);
//  tlv_add_value8(&tlv, TAG_CMM_PROTOCOL, cmmProtocolTransparent);

    ret = cmmInit(&devHandle, &tlv);
    TestPrintf_tcik0174("cmmInit = %d", ret);
    goto lblEnd;
//lblKO:
//  TestPrintf_tcik0174("RS232 layer KO");
    return -1;
  lblEnd:
    return 1;
}

int DialRS232_tcik0174() {
    //empty
    return 1;
}

void tcik0174() {
    int16 ret;
    uint8 buf[512];
    uint16 len;

    ret = SetRS232_tcik0174();
    if(ret < 0)
        goto lblKO;

    ret = DialRS232_tcik0174();
    if(ret < 0)
        goto lblKO;

    memset(buf, 0, sizeof(buf));
    memcpy(buf,
           "\x60\x06\x65\x06\x65\x02\x00\x30\x20\x05\x80\x20\xC0\x00\x04\x00\x40\x00\x00\x00\x00\x12\x11\x50\x00\x36\x09\x00\x22\x00\x12\x00\x37\x02\x01\x02\x00\x28\x42\x07\x80\xD0\x70\x20\x33\x00\x01\x23\x40\x10\x00\x00\x32\x31\x31\x39\x30\x39\x31\x38\x30\x30\x30\x30\x30\x31\x30\x34\x30\x32\x31\x37\x32\x36\x34\x00\x06\x30\x30\x31\x32\x34\x32",
           83);

    ret = cmmSend(devHandle, buf, 83);
    TestPrintf_tcik0174("cmmSend %d", ret);
    if(ret < 0)
        goto lblKO;

    ret = cmmRecv(devHandle, buf, &len, sizeof(buf));
    TestPrintf_tcik0174("cmmRecv %d", ret);
    if(ret < 0)
        goto lblKO;
    goto lblEnd;
  lblKO:
    TestPrintf_tcik0174("Test KO");
  lblEnd:
    ret = cmmEnd(devHandle);
    TestPrintf_tcik0174("cmmEnd %d", ret);

}
