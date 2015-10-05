/** \file 
 * Unitary test case tcns0015.
 * Contactless TPASS:
 * Load TPass library, detect, activate the MIFARE card, read/print the BLOCK0
 * \sa
 */

#include "ctx.h"
#ifdef __MFC__
#include "SDK30.H"
#include "oem_cless.h"          // Telium Pass include driver
#include "TPass.h"              // Telium Pass include DLL
#include "oem_clmf1.h"          // Mifare

static int mfcStartWaitCard(int timeout) {  //timeout in seconds
    int nResult = 1;
    int nNumOfCards = 1;

    int ret = 0;

    ret = TPasslib_open();
    if(ret) {
        printf("TPass lib is not loaded!!!\n");
        return -4;
    }

    if(MF_OpenDriver()) {
        printf("TAP the CARD");
        nResult = MF_DetectCards(NULL, &nNumOfCards, timeout);

        if(nResult != MF_OK) {
            printf("Cards Detect ERROR\n");
            return -2;
        }

        nResult = MF_ActiveCard(NULL, 0);
        if(nResult != MF_OK) {
            printf("Card Select ERR=%i\n", nResult);
            return -3;
        }

        printf("Card activated %d\n", nNumOfCards);
        return nNumOfCards;
    } else
        return -1;
}

static int mfcStop(void) {
    return MF_CloseDriver();
}

//key: authentication key
//TS: contains two nibbles:
// - type: left nibble 0xA0 or 0xB0
// - sector number: right nibble (0..F)
static int mfcAut(byte key[6], byte TS) {
    int ret = 0;
    static byte INITIAL_KEY[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

    byte secNum = 0;
    byte keyType = KEY_A;       // A = 0x60 ,  B = 0x61

    if(key == NULL)
        key = INITIAL_KEY;

    keyType = ((TS & 0xF0) >> 4) + 0x56;
    secNum = (TS & 0x0F);
    ret = MF_Authentication(NULL, keyType, key, secNum);

    if(ret)
        return -ret;
    else
        return 1;
}

static int mfcLoadBlock(byte SB, byte block[16]) {
    int ret = 0;
    byte secNum = 0;
    byte blkNum = 0;

    secNum = (SB & 0xF0) >> 4;
    blkNum = (SB & 0x0F);

    ret = MF_ReadBlock(NULL, DATA, secNum, blkNum, block);

    if(ret)
        return -ret;
    else
        return 1;

}

void tcns0015(void) {
    FILE *hDisplay;
    FILE *hPrinter;

    int ret = 0;
    unsigned char BlockBuffer[16] = "";

    hDisplay = fopen("DISPLAY", "w");
    hPrinter = fopen("PRINTER", "w");
    ret = mfcStartWaitCard(60); // 1 min
    if(ret <= 0)
        goto lblKO;

    ret = mfcAut(0, 0xA0);
    if(ret <= 0)
        goto lblKO;

    memset(BlockBuffer, 0, sizeof(BlockBuffer));

    ret = mfcLoadBlock(0x00, BlockBuffer);
    if(ret > 0) {
        //trcBN(BlockBuffer,16);

        pprintf("BLOCK 0\n");
        pprintf("s/n  = %02X%02X%02X%02X\n", BlockBuffer[0], BlockBuffer[1],
                BlockBuffer[2], BlockBuffer[3]);
        pprintf("data = %02X%02X%02X%02X\n", BlockBuffer[4], BlockBuffer[5],
                BlockBuffer[6], BlockBuffer[7]);
        pprintf("     = %02X%02X%02X%02X\n", BlockBuffer[8], BlockBuffer[9],
                BlockBuffer[10], BlockBuffer[11]);
        pprintf("     = %02X%02X%02X%02X\n", BlockBuffer[12], BlockBuffer[13],
                BlockBuffer[14], BlockBuffer[15]);
    } else {
        pprintf("ReadBlock0 ERR=%i\n", ret);
        goto lblKO;
    }
    goto lblEnd;
  lblKO:
    pprintf("KO");
  lblEnd:
    mfcStop();
    fclose(hDisplay);
    fclose(hPrinter);
}
#else
void tcns0015(void) {
}
#endif
