/** \file
 * Unitary test case tcns0017: PERSO
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
typedef byte TCondition[4];

static TKey ReadKeyList[] = {
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    {0x11, 0x11, 0x11, 0x11, 0x11, 0x11},
    {0x22, 0x22, 0x22, 0x22, 0x22, 0x22},
    {0x33, 0x33, 0x33, 0x33, 0x33, 0x33},
    {0x44, 0x44, 0x44, 0x44, 0x44, 0x44},
    {0x55, 0x55, 0x55, 0x55, 0x55, 0x55},
    {0x66, 0x66, 0x66, 0x66, 0x66, 0x66}
};

static TKey WriteKeyList[] = {
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA},
    {0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB},
    {0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC},
    {0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD},
    {0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};

static TCondition accessConditionList[] = {
    {0xFF, 0x07, 0x80, 0xFF},   // INITIT CONDITIONS 
    {0x78, 0x77, 0x88, 0xFF},   // Trailer cond = 011, Data = 100 COND_1
    {0x08, 0x77, 0x8F, 0xFF}    // Trailer cond = 011, Data = 110 COND_2        
};

#define COND_1 1
#define COND_2 2
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
static int mfcWriteTrailer(byte secNum, byte KA[6], byte AC[4], byte KB[6]) {

    byte BlockBuffer[16] = "";
    int i = 0;
    int j = 0;
    int ret = 1;

    if(!secNum || KA == NULL || AC == NULL || KB == NULL)
        goto lblKO;

    for (i = 0; i < 6; i++, j++)
        BlockBuffer[j] = KA[i];
    for (i = 0; i < 4; i++, j++)
        BlockBuffer[j] = AC[i];
    for (i = 0; i < 6; i++, j++)
        BlockBuffer[j] = KB[i];

    ret = MF_WriteBlock(NULL, DATA, secNum, BLOCK_TRAILER, BlockBuffer);

    if(ret)
        goto lblKO;
    else
        goto lblEnd;

  lblKO:
    return -ret;
  lblEnd:
    return 1;
}

static int mfcFormat(byte SB, byte blkType) {

    int ret = 0;
    byte secNum = 0;
    byte blkNum = 0;
    byte block[16] = "";
    byte value[4] = "";

    secNum = (SB & 0xF0) >> 4;
    blkNum = (SB & 0x0F);

    memset(block, 0, sizeof(block));
    memset(value, 0, sizeof(value));

    if(blkNum == 3 || (!blkNum && !secNum))
        // trailer is not allowed to write with this function
        // BLOCK0 id readonly
        return -1;

    switch (blkType) {
      case VALUE:
          ret = MF_WriteBlock(NULL, VALUE, secNum, blkNum, value);
          break;

      case DATA:
      default:
          ret = MF_WriteBlock(NULL, DATA, secNum, blkNum, block);
          break;
    };

    if(ret)
        return -ret;
    else
        return 1;

}

void tcns0017(void) {
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

    ret = mfcAut(0, 0xA1);      // auth with INIT ket A to the sector 1
    if(ret <= 0)
        goto lblKO;
    pprintf("aut sec 1 ok\n");

    // 3 . WRITE SECTOR PERSO
    memset(BlockBuffer, 0, sizeof(BlockBuffer));
    strcpy(BlockBuffer, "PERSO Firstname");

    ret = mfcSaveBlock(0x10, BlockBuffer);
    if(ret <= 0)
        goto lblKO;
    pprintf("block written\n%s\n", BlockBuffer);

    memset(BlockBuffer, 0, sizeof(BlockBuffer));
    strcpy(BlockBuffer, "Perso Lastname");

    ret = mfcSaveBlock(0x11, BlockBuffer);
    if(ret <= 0)
        goto lblKO;
    pprintf("block written\n%s\n", BlockBuffer);

    memset(BlockBuffer, 0, sizeof(BlockBuffer));
    strcpy(BlockBuffer, "Perso Patronym");

    ret = mfcSaveBlock(0x12, BlockBuffer);
    if(ret <= 0)
        goto lblKO;
    pprintf("block written\n%s\n", BlockBuffer);

    // 4. UPDATE KEYS AND CONDITIONS
    // SEC1: RK1|COND1|WK1

    ret = mfcWriteTrailer(1 /*SECTR_1 */ , ReadKeyList[1] /*NEW KEY A */ ,
                          accessConditionList[COND_1] /*NEW COND */ ,
                          WriteKeyList[1] /*NEW KEY B */ );
    if(ret <= 0)
        goto lblKO;
    pprintf("keys and cond updated\n");

    // 5 . AUT SECTOR 2 (Card info)

    ret = mfcAut(0, 0xA2);      // auth with INIT ket A to the sector 2
    if(ret <= 0)
        goto lblKO;
    pprintf("aut sec 2 ok\n");

    // 6 . WRITE SECTOR CARDINFO
    memset(BlockBuffer, 0, sizeof(BlockBuffer));
    strcpy(BlockBuffer, "SN1234567890");

    ret = mfcSaveBlock(0x20, BlockBuffer);
    if(ret <= 0)
        goto lblKO;
    pprintf("block written\n%s\n", BlockBuffer);

    memset(BlockBuffer, 0, sizeof(BlockBuffer));
    strcpy(BlockBuffer, "PASS CARD TYPE");

    ret = mfcSaveBlock(0x21, BlockBuffer);
    if(ret <= 0)
        goto lblKO;
    pprintf("block written\n%s\n", BlockBuffer);

    memset(BlockBuffer, 0, sizeof(BlockBuffer));
    strcpy(BlockBuffer, "KEY SET NUM");

    ret = mfcSaveBlock(0x22, BlockBuffer);
    if(ret <= 0)
        goto lblKO;
    pprintf("block written\n%s\n", BlockBuffer);

    // 7. UPDATE KEYS AND CONDITIONS
    // SEC1: RK2|COND1|WK2
    ret =
        mfcWriteTrailer(2, ReadKeyList[2], accessConditionList[COND_1],
                        WriteKeyList[2]);
    if(ret <= 0)
        goto lblKO;
    pprintf("keys and cond updated\n");

    // 8 . AUT SECTOR 3 (RECHARGE INFO)

    ret = mfcAut(0, 0xA3);      // auth with INIT ket A to the sector 3
    if(ret <= 0)
        goto lblKO;
    pprintf("aut sec 3 ok\n");

    // 9 . WRITE SECTOR 
    memset(BlockBuffer, 0, sizeof(BlockBuffer));
    strcpy(BlockBuffer, "270808 154321");

    ret = mfcSaveBlock(0x30, BlockBuffer);
    if(ret <= 0)
        goto lblKO;
    pprintf("block written\n%s\n", BlockBuffer);

    // 10. UPDATE KEYS AND CONDITIONS
    // SEC1: RK3|COND1|WK3
    ret =
        mfcWriteTrailer(3, ReadKeyList[3], accessConditionList[COND_1],
                        WriteKeyList[3]);
    if(ret <= 0)
        goto lblKO;
    pprintf("keys and cond updated\n");

    // 11 . AUT SECTOR 5 (DEBIT INFO)

    ret = mfcAut(0, 0xA5);      // auth with INIT ket A to the sector 3
    if(ret <= 0)
        goto lblKO;
    pprintf("aut sec 5 ok\n");

    // 12 . WRITE SECTOR 
    memset(BlockBuffer, 0, sizeof(BlockBuffer));
    strcpy(BlockBuffer, "270808 154321");

    ret = mfcSaveBlock(0x50, BlockBuffer);
    if(ret <= 0)
        goto lblKO;
    pprintf("block written\n%s\n", BlockBuffer);

    // 13. UPDATE KEYS AND CONDITIONS
    // SEC1: RK5|COND1|WK5
    ret =
        mfcWriteTrailer(5, ReadKeyList[5], accessConditionList[COND_1],
                        WriteKeyList[5]);
    if(ret <= 0)
        goto lblKO;
    pprintf("keys and cond updated\n");

    // 14 . AUT SECTOR 4 (DEBIT)            
    ret = mfcAut(0, 0xA4);      // auth with INIT ket A to the sector 4
    if(ret <= 0)
        goto lblKO;
    pprintf("aut sec 4 ok\n");

    // 15. FORMAT BLOCK AS VALUE
    ret = mfcFormat(0x40, VALUE);
    pprintf("format as val ok=%i\n", ret);

    // 16. UPDATE KEYS AND CONDITIONS
    // SEC1: RK4|COND2|WK4
    ret =
        mfcWriteTrailer(4, ReadKeyList[4], accessConditionList[COND_2],
                        WriteKeyList[4]);
    if(ret <= 0)
        goto lblKO;
    pprintf("keys and cond updated\n");

    // 17 . AUT SECTOR 6 (CREDIT)           
    ret = mfcAut(0, 0xA6);      // auth with INIT ket A to the sector 6
    if(ret <= 0)
        goto lblKO;
    pprintf("aut sec 6 ok\n");

    // 18. FORMAT BLOCK AS VALUE
    ret = mfcFormat(0x60, VALUE);
    pprintf("format as val ok=%i\n", ret);

    // 19. UPDATE KEYS AND CONDITIONS
    // SEC1: RK6|COND2|WK6
    ret =
        mfcWriteTrailer(6, ReadKeyList[6], accessConditionList[COND_2],
                        WriteKeyList[6]);
    if(ret <= 0)
        goto lblKO;
    pprintf("keys and cond updated\n");

    goto lblEnd;

  lblKO:
    pprintf("ERR occured\n");
  lblEnd:
    mfcStop();
    fclose(hDisplay);
    fclose(hPrinter);
}
#else
void tcns0017(void) {
}
#endif
