/** \file
 * Unitary test case tcns0018: CREDIT
 * Functions testing:
 * \sa
 */

#include "ctx.h"

#ifdef __MFC__
#include "SDK30.H"
#include "oem_cless.h"          // Telium Pass include driver
#include "TPass.h"              // Telium Pass include DLL
#include "oem_clmf1.h"          // Mifare

typedef byte TKey[6];

static TKey WriteKeyList[] = {
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA},
    {0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB},
    {0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC},
    {0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD},
    {0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};

#define BLOCK_TRAILER 3

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

static int mfcOperation(byte SB, byte cmd, byte value[4]) {
    int ret = 0;
    byte secNum = 0;
    byte blkNum = 0;

    secNum = (SB & 0xF0) >> 4;
    blkNum = (SB & 0x0F);

    if(blkNum == 3 || (!blkNum && !secNum))
        // trailer is not allowed to write with this function
        // BLOCK0 id readonly
        return -1;

    switch (cmd) {

      case MF_READ:
          ret = MF_ReadBlock(NULL, VALUE, secNum, blkNum, value);
          break;
      case MF_WRITE:
          ret = MF_WriteBlock(NULL, VALUE, secNum, blkNum, value);
          break;
      case MF_INCREMENT:
      case MF_DECREMENT:
          ret = MF_ValueOperation(NULL, cmd, secNum, blkNum, blkNum, value);
          break;
      default:
          return -2;

    }

    if(ret)
        return -ret;
    else
        return 1;

}

void tcns0018(void) {
    FILE *hDisplay;
    FILE *hPrinter;

    int ret = 0;
    unsigned char Amount[4];

    hDisplay = fopen("DISPLAY", "w");
    hPrinter = fopen("PRINTER", "w");

    ret = mfcStartWaitCard(60); // 1 min
    if(ret <= 0)
        goto lblKO;

    // AUT with key WK4(type B) to sec 4
    ret = mfcAut(WriteKeyList[4], 0xB4);
    if(ret <= 0)
        goto lblKO;
    pprintf("aut sec 4 ok\n");

    //INCREMENT (INCREASE) VALUE BLOCK with Amount[]
    Amount[0] = 0x01;
    Amount[1] = 0x00;
    Amount[2] = 0x00;
    Amount[3] = 0x00;

    ret = mfcOperation(0x40, MF_INCREMENT, Amount);
    if(ret)
        pprintf("inc ok\n", ret);
    else
        goto lblKO;

    //READ VALUE BLOCK to Amount[]
    ret = mfcOperation(0x40, MF_READ, Amount);
    if(ret) {
        pprintf("read ok\n", ret);
        pprintf("%02X%02X%02X%02X\n", Amount[3], Amount[2], Amount[1],
                Amount[0]);
    } else
        goto lblKO;

    goto lblEnd;

  lblKO:
    pprintf("ERR occured\n");
  lblEnd:
    mfcStop();
    fclose(hDisplay);
    fclose(hPrinter);
}
#else
void tcns0018(void) {
}
#endif
