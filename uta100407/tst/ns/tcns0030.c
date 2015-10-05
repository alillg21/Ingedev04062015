/// \file tcns0030.c

#include "ctx.h"
#ifdef __MFC__
#include "SDK30.H"
#include "oem_cless.h"          // Telium Pass include driver
#include "TPass.h"              // Telium Pass include DLL
#include "oem_clmf1.h"          // Mifare

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
// - sector number (0..15)
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

static int mfcSaveBlock(byte SB, byte block[16]) {
    int ret = 0;
    byte secNum = 0;
    byte blkNum = 0;

    secNum = (SB & 0xF0) >> 4;
    blkNum = (SB & 0x0F);

    if(blkNum == 3 || (!blkNum && !secNum))
        // trailer is not allowed to write with this function
        // BLOCK0 id readonly
        return -1;

    ret = MF_WriteBlock(NULL, DATA, secNum, blkNum, block);

    if(ret)
        return -ret;
    else
        return 1;

}

void tcns0030(void) {
    FILE *hDisplay;
    FILE *hPrinter;

    int ret = 0;
    unsigned char BlockBuffer[16] = "";

    hDisplay = fopen("DISPLAY", "w");
    hPrinter = fopen("PRINTER", "w");

    // 1 . START    
    ret = mfcStartWaitCard(60); // 1 min
    if(ret <= 0)
        goto lblKO;

    // 2 . AUT SECTOR 1 (PERSO)

    ret = mfcAut(0, 0xA2);      // auth with INIT ket A to the sector 1
    if(ret <= 0)
        goto lblKO;
    pprintf("aut sec 1 ok\n");

    // 3 . WRITE SECTOR PERSO
    memset(BlockBuffer, 0, sizeof(BlockBuffer));
    strcpy(BlockBuffer, "1234567890ABCDEF");

    ret = mfcSaveBlock(0x20, BlockBuffer);
    if(ret <= 0)
        goto lblKO;
    pprintf("block written\n%s\n", BlockBuffer);

    memset(BlockBuffer, 0, sizeof(BlockBuffer));

    ret = mfcLoadBlock(0x20, BlockBuffer);
    if(ret <= 0)
        goto lblKO;
    pprintf("block =\n%s\n", BlockBuffer);

    goto lblEnd;

  lblKO:
    pprintf("ERR occured\n");
  lblEnd:
    mfcStop();
    fclose(hDisplay);
    fclose(hPrinter);
}
#else
void tcns0030(void) {
}
#endif
