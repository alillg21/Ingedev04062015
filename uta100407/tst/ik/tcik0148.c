/// \file tcik0148.c
#include "sdk30.h"
#include "schvar_def.h"
#include "lib_iapp.h"
#include "sys.h"
#include "tst.h"

/**Non UTA unitary test case tcik0148. Call SchCipherData scheme in order to calculate a MAC
* \return
 - void
*/

//init secret area, signature card and var numbers
#ifndef ID_SCR_XXX_BANK
#define ID_SCR_XXX_BANK   0x0E012030
#define CARD_NUMBER_SIGN  0x0000EC15
#define VAR_NUMBER_SIGN         0x0E
#endif

unsigned char TheBlocResult[100];
unsigned char TheMacBuffer[72];
const unsigned char Track[63 + 1] =
    { "04TERMEMV2        001111;4475209900514014=97091261310375600000?" };
const unsigned char TheMacKey[DES_KEY_SIZE + 1] =
    { "\x53\x7F\x97\xB1\x11\x48\x7D\x71" };
const unsigned char ClearMacKey[DES_KEY_SIZE + 1] =
    { "\x79\x68\xFB\xD9\xB5\x67\x31\x45" };

static int LoadDataSecure(int iOffset, int iLenData, int iCipherMode,
                          int iSecretId, unsigned char *pti_ucData,
                          int ibIsPPR) {
    int ret;
    T_IAPPSync Synchro;
    T_LOAD_DATA_RESULT loadResult;
    FILE *hdIapp;

    SetPprState(ibIsPPR);       // TRUE for PPR Booster, FALSE for EFT Booster
    hdIapp = open_iapp("rw");
    if(hdIapp != NULL) {        // Load Root key, last parameter is an ID that will be register by the system
        ret =
            sch_init_w(ID_SCH_LOAD_DATA, NAM_SCH_LOAD_DATA, iOffset, iLenData,
                       iCipherMode, iSecretId);
        if(ret == OK) {
            while(IAPP_SchGetSync(1000, &Synchro, hdIapp) != OK);
            if(Synchro == SYNC_VALID) {
                IAPP_SchPutData(pti_ucData, iLenData, hdIapp);
                while(IAPP_SchGetSync(1000, &Synchro, hdIapp) != OK);
                if(Synchro == SYNC_VALID) {
                    IAPP_SchGetData(1000, (unsigned char *) &loadResult,
                                    sizeof(loadResult), hdIapp);
                    ret = loadResult.ReturnCode;
                } else {
                    ret = ERR_TIMEOUT;
                }
            } else {
                ret = ERR_TIMEOUT;
            }
        }
        sch_end();
        fclose(hdIapp);
    } else {
        ret = ERR_BAD_IAPP_OPERATION;
    }
    return (ret);
}

static void printResult(unsigned char *TheBlocResult) {
    FILE *hPrt = 0;
    int i;
    char buffer[60], bufftemp[10];

    buffer[0] = 0;
    for (i = 1; i <= (int) TheBlocResult[0]; i++) {
        sprintf(bufftemp, "%02X", TheBlocResult[i]);
        strcat(buffer, bufftemp);
    }

    hPrt = fopen("PRINTER", "w");
    SetDefaultFont(_MEDIUM_);

    pprintf("Result:\n");
    strcat(buffer, "\n\n");
    pprintf(buffer);

    fclose(hPrt);

    trcS("Result:\n");
    trcS(buffer);
}
static int Verify(int loc) {
    int ret = 0;
    T_IAPPSync Synchro;
    T_CIPHER_DATA_RESULT cipherResult;
    const unsigned char TheVoidData[8 + 1] =
        { "\x00\x00\x00\x00\x00\x00\x00\x00" };
    static FILE *pIdApp;
    unsigned char crt[4 + 1 + 1];

    memset(crt, 0, sizeof(crt));
    SetPprState(FALSE);         // TRUE for PPR Booster, FALSE for EFT Booster
    pIdApp = open_iapp("rw");
    CHECK(pIdApp != NULL, lblKO);

    ret =
        sch_init(ID_SCH_CIPHER_DATA, NAM_SCH_CIPHER_DATA, DATA_DES_CIPHER | loc,
                 sizeof(TheVoidData), sizeof(TheVoidData), ID_SCR_XXX_BANK);
    CHECK(ret == OK, lblKO);

    while(IAPP_SchGetSync(1000, &Synchro, pIdApp) != OK);
    CHECK(Synchro == SYNC_VALID, lblKO);

    IAPP_SchPutData((unsigned char *) TheVoidData, sizeof(TheVoidData), pIdApp);

    while(IAPP_SchGetSync(1000, &Synchro, pIdApp) != OK);
    CHECK(Synchro == SYNC_VALID, lblKO);

    IAPP_SchGetData(1000, (unsigned char *) &cipherResult,
                    ANSWER_HEADER_SIZE + RETURN_CODE_SIZE, pIdApp);
    ret = cipherResult.ReturnCode;
    CHECK(ret == OK, lblKO);

    IAPP_SchGetData(1000, (unsigned char *) &cipherResult.BufferOut, 8, pIdApp);
    crt[0] = '\x04';
    memcpy(crt + 1, cipherResult.BufferOut, 4);
    printResult(crt);

    ret = OK;
    goto lblEnd;
  lblKO:
    ret = -1;
    trcErr(ret);
  lblEnd:
    sch_end();
    fclose(pIdApp);
    return ret;
}

void tcik0148(void) {
    int ret = 0;
    char buffer[30];
    T_CIPHER_DATA_RESULT cipherResult;
    T_IAPPSync Synchro;
    FILE *Iapp = 0;

    trcS("   ***   tcik0148   ***   \n");
    trcS("Run tcik0145 before\n");
    tcik0145();

    //load WC, working key encrypted by transport key is 537F97B111487D71 in loc = 24, Transport key loc = 1008
    ret =
        LoadDataSecure(24, 8, DES_CIPHERED_MODE | 1016, ID_SCR_XXX_BANK,
                       (unsigned char *) &TheMacKey, FALSE);

    //ClearMacKey
    //ret = LoadDataSecure (24, 8, DATA_NO_CIPHER, ID_SCR_XXX_BANK,(unsigned char *) &ClearMacKey, FALSE);

    sprintf(buffer, "ret = %d\n", ret);
    trcS(buffer);
    if(ret != OK)
        goto lblKO;
    Verify(24 - 8);

    SetPprState(FALSE);         /* EFT Booster */
    Iapp = open_iapp("rw");

    if(Iapp == NULL)
        goto lblKO;
    // Load key, last parameter is an ID that will be register by the system

    ret =
        sch_init_w(ID_SCH_CIPHER_DATA, NAM_SCH_CIPHER_DATA,
                   DATA_MAC_ICV | (24 - 8), sizeof(TheMacBuffer),
                   sizeof(TheMacBuffer), ID_SCR_XXX_BANK);
    sprintf(buffer, "ret = %d\n", ret);
    trcS(buffer);
    if(ret != OK)
        goto lblKO;
    while(IAPP_SchGetSync(1000, &Synchro, Iapp) != OK);
    if(Synchro != SYNC_VALID)
        goto lblKO;

    // initialize a buffer to '5', except 8 first bytes that must contain
    // a initial vector (default value: 0x00)
    memset(TheMacBuffer, 0, 8); // Initial vector is set to 0x00
    memcpy(TheMacBuffer + 8, Track, 63);
    //memset (&TheMacBuffer[8], '5', (sizeof(TheMacBuffer)-8));

    // send buffer to crypto processor
    IAPP_SchPutData(TheMacBuffer, sizeof(TheMacBuffer), Iapp);

    while(IAPP_SchGetSync(1000, &Synchro, Iapp) != OK);
    if(Synchro != SYNC_VALID)
        goto lblKO;

    IAPP_SchGetData(1000, (unsigned char *) &cipherResult,
                    ANSWER_HEADER_SIZE + RETURN_CODE_SIZE, Iapp);
    ret = cipherResult.ReturnCode;
    if(ret != OK)
        goto lblKO;
    if((cipherResult.Len - RETURN_CODE_SIZE) > MAX_DATA_SIZE)
        goto lblKO;

    IAPP_SchGetData(1000, (unsigned char *) &cipherResult.BufferOut,
                    cipherResult.Len - RETURN_CODE_SIZE, Iapp);
    TheBlocResult[0] = cipherResult.Len - RETURN_CODE_SIZE;
    memcpy(&TheBlocResult[1], &cipherResult.BufferOut, (int) TheBlocResult[0]);
    printResult(TheBlocResult);

    goto lblEnd;
  lblKO:
    trcErr(ret);
    _DrawWindow(10, 20, 120, 60, 2, (unsigned char *) "tcik0148 KO");
    PaintGraphics();
    ttestall(0, 100);
  lblEnd:
    sch_end();
    fclose(Iapp);
    trcS("End of tcik0148\n");
}
