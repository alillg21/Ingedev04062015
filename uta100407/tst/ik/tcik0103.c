/// \file tcik0103.c

#include <unicapt.h>
#include <stdio.h>
#include <stdlib.h>
#include "message.h"

uint32 hmiHandle, prnHandle;

//////////////////////////////////////////////////////////////////////////
#define IMGR_TX_BUFFER_SIZE  512
#define IMGR_RX_BUFFER_SIZE  512
#define IMGR_SEND_TIMEOUT     20
#define IMGR_RECV_TIMEOUT    1000

#define IMGR_CMD_BUFFER_SIZE 256

#define IMGR_WAIT     1
#define IMGR_NOWAIT   0

#define IMGR_ENQ 0x05
#define IMGR_ACK 0x06
#define IMGR_NAK 0x15

#define IMGR_SYN 0x16
#define IMGR_CR  0x0D
#define IMGR_CMD 'M'
#define IMGR_ON  'T'
#define IMGR_OFF 'U'

#define IMGR_QRYDFT '^'
#define IMGR_QRYCUR '?'
#define IMGR_PERM  '.'
#define IMGR_TEMP  '!'

typedef struct {
    int16 status;
    uint16 length;
    uint8 datas[2];
} oneResult_t;

uint8 imgOn = 0;
uint32 imgHandle;
static char imgDefaultCfg[] = "PRECA2;dechdr0;DECMOD0;TRGMOD0;TRGLPT240;232LPT0.";  // DEFAULT CONFIG
static char RefBarCodeNumber[50];

  //static char CmpBarCodeNumber[50];

int16 imgSend(uchar command) {
    int16 ret_code;

    ret_code = comSendByte(imgHandle, command);
    return (ret_code);
}

int16 imgClose(void) {
    int16 ret_code;

    ret_code = comClose(imgHandle);
    return (ret_code);
}

int16 imgON(void) {
    int16 ret_code;

    comReceptionFlush(imgHandle);

    ret_code = imgSend(0x16);   //SYN
    ret_code = imgSend(0x74);   //"t"
    ret_code = imgSend(0x0d);   //CR

    return (ret_code);
}

int16 imgOFF(void) {
    int16 ret_code;

    ret_code = imgSend(0x16);   //SYN
    ret_code = imgSend(0x75);   //"u"
    ret_code = imgSend(0x0d);   //CR

    return (ret_code);
}

int16 imgOpen(void) {
    uint32 retRes;
    uint16 ret_code;
    comInit_t init;
    comUartInit_t uartInit;
    comProtocolCfg_t protocol;
    comUartProtocolCfg_t uartProtocol;
    uint8 i;
    int16 result;
    char text[80];

    // Init COM4
    retRes = comOpen("COM4", &imgHandle);
    if(RET_OK != retRes) {
        hmiADDisplayText(hmiHandle, 1, 1, "Error opening COM4!");
        psyTimerWakeAfter(SYS_TIME_SECOND * 2);
        return retRes;
    }                           // end if

    //---------------------------------------------------------------------------
    // Set-up the port for use.
    //---------------------------------------------------------------------------

    init.type = COM_INIT_TYPE_UART;
    init.level = COM_INIT_LEVEL_1;
    init.init.uartInit = &uartInit;
    uartInit.speed = COM_BAUD_115200;
    uartInit.parity = COM_PARITY_NONE;
    uartInit.dataSize = COM_DATASIZE_8;
    uartInit.stopBits = COM_STOP_1;
    uartInit.sBufferSize = IMGR_TX_BUFFER_SIZE;
    uartInit.rBufferSize = IMGR_RX_BUFFER_SIZE;
    protocol.type = COM_PROT_CFG_TYPE_UART;
    protocol.level = COM_PROT_CFG_LEVEL_1;
    protocol.protCfg.uartCfg = &uartProtocol;
    uartProtocol.flowControl = COM_FLOW_NONE;
    uartProtocol.interCharTimeOut = 0;  // infini
    uartProtocol.xOnChar = 0x0;
    uartProtocol.xOffChar = 0x0;

    ret_code = comChanInit(imgHandle,
                           COM_MODIF_PARAM_TEMPORARY,
                           &init, COM_SET_PROTOCOL_TEMPORARY, &protocol);

    // Default configuration for the Imager

    //
    ret_code = imgSend(0x16);   //SYN
    ret_code = imgSend(0x74);   //"t"
    ret_code = imgSend(0x0d);   //CR

    psyTimerWakeAfter(SYS_TIME_SECOND * 2);

    // Send command
    ret_code = imgSend(0x16);   //SYN
    ret_code = imgSend(0x6d);   //"M"
    ret_code = imgSend(0x0d);   //CR

    if(ret_code != RET_OK) {
        hmiADDisplayText(hmiHandle, 5, 0, "ERROR IN TX SYN+M+CR");
        psyTimerWakeAfter(SYS_TIME_SECOND * 3);
    }
    // Send default config
    for (i = 0; i < strlen(imgDefaultCfg); i++) {
        ret_code = imgSend(imgDefaultCfg[i]);
        sprintf(text, "TX: %c", imgDefaultCfg[i]);
        hmiADDisplayText(hmiHandle, 6, 0, text);
        if(ret_code != RET_OK) {
            hmiADDisplayText(hmiHandle, 5, 0, "ERROR IN TX CFG");
            psyTimerWakeAfter(SYS_TIME_SECOND * 3);
        }
    }

    // Receive the OK response from the imager
    // Wait for response loop;
    while(1) {
        result = comReceiveByteWait(imgHandle, IMGR_RECV_TIMEOUT);

        //Timeout or error or End of response
        if(result == IMGR_PERM || result == IMGR_TEMP)
            break;

        //Command accepted successfully
        if(result == IMGR_ACK)
            ret_code = RET_OK;
        //Command accepted but incorrect for the current group context
        if(result == IMGR_NAK)
            ret_code = IMGR_NAK;
        //Command not accepted
        if(result == IMGR_ENQ)
            ret_code = IMGR_ENQ;
    }

    return (ret_code);
}

///////////////////////////// USER'S COMMENTARY //////////////////////////

//////////////////////////////////////////////////////////////////////////
void tcik0103(void) {
    uint32 ret_code;
    char st[100];
    oneResult_t receive;

    hmiOpen("DEFAULT", &hmiHandle);
    prnOpen("DEFAULT", &prnHandle, PRN_IMMEDIATE);

    /* ADD YOU CODE FOR TASK INITIALISATION HERE : */
    memset(RefBarCodeNumber, '\0', 20);
    hmiADDisplayText(hmiHandle, 1, 0, "Activating Imager...");
    ret_code = imgOpen();

    comReceptionFlush(imgHandle);
    ret_code = comSendByte(imgHandle, 0x16);    //SYN
    ret_code = comSendByte(imgHandle, 0x74);    //"t"
    ret_code = comSendByte(imgHandle, 0x0d);    //CR
    hmiADDisplayText(hmiHandle, 1, 1, "Activated");

    hmiADDisplayText(hmiHandle, 1, 1, "LEARNING ...");
    hmiADDisplayText(hmiHandle, 1, 1, "place a barcode");
    comReceiveByteReq(imgHandle);
    ret_code = psyPeripheralResultWait(COM_WAIT, 1000, 0);
    if(PSY_PER_EXECUTED_OK(ret_code, COM_WAIT)) {
        memset(receive.datas, '\0', 2);
        ret_code =
            comResultGet(imgHandle, COM_RECEIVE_EVENT, sizeof(receive),
                         &receive);
        if(ret_code == RET_OK) {
            if(receive.status == RET_OK) {
                sprintf(RefBarCodeNumber, "%s", receive.datas);
            }
        }
    } else {
        imgOFF();
        imgClose();
        hmiClose(hmiHandle);
        prnClose(prnHandle);

        return;
    }

    while(1)                    // APPRENTISSAGE
    {
        comReceiveByteReq(imgHandle);
        ret_code = psyPeripheralResultWait(COM_WAIT, 50, 0);
        if(PSY_PER_EXECUTED_OK(ret_code, COM_WAIT)) {
            memset(receive.datas, '\0', 2);
            ret_code =
                comResultGet(imgHandle, COM_RECEIVE_EVENT, sizeof(receive),
                             &receive);
        } else {
            // Time OUT end of reading
            hmiADDisplayText(hmiHandle, 1, 1, "readed Barcode Number:");
            hmiADDisplayText(hmiHandle, 2, 1, RefBarCodeNumber);
            break;
        }
        if(ret_code == RET_OK) {
            if(receive.status == RET_OK) {
                sprintf(RefBarCodeNumber, "%s%s", RefBarCodeNumber, receive.datas); // collapse barcode
            }
        }
    }

    prnPrint(prnHandle, "-----------------------");
    prnPrint(prnHandle, "       INGENICO");
    prnPrint(prnHandle, "-----------------------");
    prnPrint(prnHandle, "    CARTES SHOW 2004");
    prnPrint(prnHandle, "     BARCODE READER");
    prnPrint(prnHandle, "  \n");
    sprintf(st, "[ %s ]", RefBarCodeNumber);
    prnPrint(prnHandle, st);
    prnPrint(prnHandle, "  \n");
    prnPrint(prnHandle, "-----------------------");
    prnPrint(prnHandle, "  \n");
    prnPrint(prnHandle, "  \n");
//  prnPrint(prnHandle,"  \n");
//  prnPrint(prnHandle,"  \n");
//  prnPrint(prnHandle,"  \n");
//  prnPrint(prnHandle,"  \n");

    imgOFF();
    imgClose();
    hmiClose(hmiHandle);
    prnClose(prnHandle);
}
