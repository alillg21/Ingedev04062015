/** \file
 * Unitary test case tcns0016.
 * Functions testing:
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

void tcns0016(void) {
    int ret = 0;

    // 1 . START    
    ret = mfcStartWaitCard(60); // 1 min wit for card tap
    if(ret <= 0)
        goto lblKO;

    // 2 . AUT sector 0 with INIT key A

    ret = mfcAut(0, 0xA0);
    if(ret <= 0)
        goto lblKO;

    // 3 . WRITE block 1,2 of sector 0 with strings

    ret = mfcSaveBlock(0x01, "TESTSTRING0");
    if(ret <= 0)
        goto lblKO;

    ret = mfcSaveBlock(0x02, "TESTSTRING1");
    if(ret <= 0)
        goto lblKO;

  lblKO:
    // 4 . CLOSE

    mfcStop();
}
#else
void tcns0016(void) {
}
#endif
