/// \file tcik0145.c
#include "sdk30.h"
#include "schvar_def.h"
#include "lib_iapp.h"
#include "sys.h"
#include "tst.h"

/** Non UTA unitary test case tcik0145. Load PK master at adr 72-79 in clear mode
*/

//init secret area, signature card and var numbers
#ifndef ID_SCR_XXX_BANK
#define ID_SCR_XXX_BANK   0x0E012030
#define CARD_NUMBER_SIGN  0x0000EC15
#define VAR_NUMBER_SIGN         0x0E
#endif

typedef struct {
    int CardNumber;
    int VarNumber;
    unsigned char Rfu[1000];
    unsigned char RootKey[TDES_KEY_SIZE];
} T_ROOT_KEY;

const unsigned char ThePinMasterKey[DES_KEY_SIZE + 1] = { "12345678" };
const unsigned char TheRootKey[TDES_KEY_SIZE + 1] =
    { "\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11" };

/** Interface function to call the LoadData Scheme
 * !! WARNING !! Be careful with data at offset 0 to 7
 * \author       JLM
 * \param iOffset (I) Offset in the Secret Area to load data (0 to 1023)
 * \param iLenData (I) Data length in bytes of data to load
 * \param iCipherMode (I) CipherMode parameter of LoadData Scheme (see doc)
 * \param iSecretId (I) ID of the Secret Area
 * \param pti_ucData (I) Pointer of data to load
 * \param ibIsPPR (I) TRUE for Booster of PPR, FALSE for Booster of EFT
 * \return
 * - OK(0)
 * - ERR_xxxx
 */
static int LoadDataSecure(int iOffset, int iLenData, int iCipherMode,
                          int iSecretId, unsigned char *pti_ucData,
                          int ibIsPPR) {
    int ret;
    T_IAPPSync Synchro;
    T_LOAD_DATA_RESULT loadResult;
    FILE *hdIapp;

    SetPprState(ibIsPPR);       /* TRUE for PPR Booster, FALSE for EFT Booster */
    hdIapp = open_iapp("rw");

    if(hdIapp != NULL) {
        // Load Root key, last parameter is an ID that will be register by the system
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

/** Test existence of a Secret Area
 * \author       JLM
 * \param iPPR TRUE (I) for Booster of PPR, FALSE for Booster of EFT
 * \param iIDtoSearch (I) ID of the Secret Area to search
 * \return
 * - TRUE (Secret Area exists)
 * - FALSE (Secret Area does not exist)
 */
static int DoesSecretAreaExist(int iPPR, SEG_ID iIDtoSearch) {
    int iret;
    SEG_ID lId[3] = { 1, 1, 1 };
    SERR cr;
    int iAux;

    iret = FALSE;

    iAux = 0;
    while(lId[0] && lId[1] && lId[2]) {
        if(iPPR == TRUE)
            cr = ddirUsb(iAux, &lId[0], &lId[1], &lId[2]);
        else
            cr = ddir(iAux, &lId[0], &lId[1], &lId[2]);
        iAux = 1;
        if(cr == SERR_OK) {
            if((lId[0] == iIDtoSearch) || (lId[1] == iIDtoSearch)
               || (lId[2] == iIDtoSearch)) {
                iret = TRUE;
                break;
            }
        } else {
            break;
        }
    }

    return (iret);
}

/** Interface function to call the Free Scheme
 * \author       JLM
 * \param iSecretId (I) ID of the Secret Area to free
 * \param ibIsPPR (I) TRUE for Booster of PPR, FALSE for Booster of EFT
 * \return
 * - OK(0)
 * - ERR_xxxx
 */
static int FreeSecretAreaId(int iSecretId, int ibIsPPR) {
    int ret;

    SetPprState(ibIsPPR);       /* TRUE for PPR Booster, FALSE for EFT Booster */
    ret =
        sch_init_w(ID_SCH_FREE_SECRET, NAM_SCH_FREE_SECRET, iSecretId, 0, 0, 0);
    ret = sch_end();
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

void tcik0145(void) {
    int ret;
    T_ROOT_KEY key;
    char buffer[30];

    trcS("   ***   tcik0145   ***   \n");
    //initialisation of parameters for test transport key loading into location 0
    key.CardNumber = CARD_NUMBER_SIGN;
    key.VarNumber = VAR_NUMBER_SIGN;
    memcpy(key.RootKey, TheRootKey, TDES_KEY_SIZE);

    /* Automatic Creating Secret Area and initialise secret data */
    trcS("Automatic Creating Secret Area,\n");
    trcS("transport key loading.\n");
    ret =
        LoadDataSecure(0, 1024, DATA_NO_CIPHER, ID_SCR_XXX_BANK,
                       (unsigned char *) &key, FALSE);
    sprintf(buffer, "ret = %d\n", ret);
    trcS(buffer);
    switch (ret) {
      case ERR_ID_NOT_FOUND:
          trcS(buffer);
          break;
      case ERR_TIMEOUT:
          trcS(buffer);
          break;
    }
    if(ret != OK)
        goto lblKO;
    Verify(1024 - 16);
    //Check Existance of secret area before PK master loading
    trcS("Check Existance of secret area\n");
    if(DoesSecretAreaExist(FALSE, ID_SCR_XXX_BANK) == FALSE) {
        goto lblKO;
    } else {
        /* Secret Area exists => so, you can load data in secret area */
        trcS("Load PK master in secret area\n");
        ret =
            LoadDataSecure(72, 8, DATA_NO_CIPHER, ID_SCR_XXX_BANK,
                           (unsigned char *) &ThePinMasterKey, FALSE);
        sprintf(buffer, "ret = %d\n", ret);
        trcS(buffer);
        if(ret != OK)
            goto lblKO;
    }

    ttestall(0, 100);
    goto lblEnd;
  lblKO:
    trcErr(ret);
    FreeSecretAreaId(ID_SCR_XXX_BANK, FALSE);   /* Free Secret Area in EFT Booster */
    _DrawWindow(10, 20, 120, 60, 2, (unsigned char *) "tcik0145 KO");
    PaintGraphics();
    ttestall(0, 100);
  lblEnd:
    trcS("End of tcik0145\n");
}
