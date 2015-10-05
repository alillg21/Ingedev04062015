#include <stdio.h>
#include <string.h>
#include <unicapt.h>

//NON UTA Unitary test case tcvp0006.
//COM3 test for sending

static uint32 displayHandle;
static uint32 prnHandle;
static uint32 comHandle;
static int16 retCode;
static uint8 key;

typedef struct {
    int16 status;
    uint16 length;
    uint8 datas[1];
} oneResult_t;

//static oneResult_t receive;
//static comProtocolCfg_t protocol;
//static comUartProtocolCfg_t uartProtocol;
static comUartInit_t modemInit2;
static comInit_t comModemInit;
static int16 state;

static char txData[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static uint8 Counter;
static char Char;
static char buffer[400];

void tcvp0006(void) {
    memset(buffer, 0, sizeof(buffer));

    hmiOpen("DEFAULT", &displayHandle);
    prnOpen("DEFAULT", &prnHandle, PRN_IMMEDIATE);
    prnPrint(prnHandle, "\x1B\x17\x1B\x1a" "RS485 DEMO");
    prnPrint(prnHandle, "\x1B\x17\x1B\x1a" "----------");
    prnLinefeed(prnHandle, 3);

// open COM channel
    retCode = comOpen("COM3", &comHandle);
    if(retCode == RET_OK)
        prnPrint(prnHandle, "COM3 open\n");
    else
        return;
// prior initialisation to use comSetPortParameters
    modemInit2.speed = COM_BAUD_19200;
    modemInit2.parity = COM_PARITY_NONE;
    modemInit2.dataSize = COM_DATASIZE_8;
    modemInit2.stopBits = COM_STOP_1;
    modemInit2.rBufferSize = 1024;
    modemInit2.sBufferSize = 1024;
    comModemInit.type = COM_INIT_TYPE_UART; //COM_INIT_TYPE_MODEM;
    comModemInit.level = COM_INIT_LEVEL_1;
    comModemInit.init.uartInit = &modemInit2;

    // Set port parameters TEMPORARY

    state =
        comSetPortParameters(comHandle, COM_MODIF_PARAM_TEMPORARY,
                             &comModemInit);
    if(state != RET_OK) {
        sprintf(buffer, "comSetPortParameters : %02x", state);
        prnPrint(prnHandle, buffer);
    } else {
        // print what is set inside
        prnLinefeed(prnHandle, 2);
        prnPrint(prnHandle, "RS485 :");
        sprintf(buffer, "speed : %02x", modemInit2.speed);
        prnPrint(prnHandle, buffer);
        sprintf(buffer, "parity : %02x", modemInit2.parity);
        prnPrint(prnHandle, buffer);
        sprintf(buffer, "dataSize : %02x", modemInit2.dataSize);
        prnPrint(prnHandle, buffer);
        sprintf(buffer, "stopBits : %02x", modemInit2.stopBits);
        prnPrint(prnHandle, buffer);
        sprintf(buffer, "rBufferSize : %02x", modemInit2.rBufferSize);
        prnPrint(prnHandle, buffer);
        sprintf(buffer, "sBufferSize : %02x", modemInit2.sBufferSize);
        prnPrint(prnHandle, buffer);
        prnLinefeed(prnHandle, 4);
    }

    while(1) {
// Transmitting
        prnPrint(prnHandle, "SEND procedure");

        for (Counter = 0; Counter != sizeof(txData); ++Counter) {
            Char = txData[Counter];
            retCode = comSendByte(comHandle, Char);
            sprintf(buffer, "Bytes sent : %d ", Counter + 1);
            hmiADDisplayText(displayHandle, 0, 0, buffer);

            if(Counter < 16) {
                sprintf(buffer, "%c", Char);
                hmiADDisplayText(displayHandle, 1, Counter, buffer);
            } else {
                sprintf(buffer, "%c", Char);
                hmiADDisplayText(displayHandle, 2, (Counter - 16), buffer);
            }
        }
        sprintf(buffer, "%s", &txData[0]);
        prnPrint(prnHandle, buffer);
        sprintf(buffer, "Bytes sent : %d ", Counter);
        prnPrint(prnHandle, buffer);
        prnLinefeed(prnHandle, 2);

        hmiKeyWait(displayHandle, &key, PSY_INFINITE_TIMEOUT);
    }
    prnClose(prnHandle);
    hmiClose(displayHandle);
    retCode = comClose(comHandle);
    if(retCode == RET_OK)
        prnPrint(prnHandle, "Close COM3");
    else
        return;

}
