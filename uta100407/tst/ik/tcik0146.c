/// \file tcik0146.c
#include "sdk30.h"
#include "schvar_def.h"
#include "lib_iapp.h"
#include "sys.h"
#include "tst.h"

/** Non UTA unitary test case tcik0146. Load PK encryption key 8-15, decipered by PIN master key 72-79
*/

//init secret area, signature card and var numbers
#ifndef ID_SCR_XXX_BANK
#define ID_SCR_XXX_BANK   0x0E012030
#define CARD_NUMBER_SIGN  0x0000EC15
#define VAR_NUMBER_SIGN         0x0E
#endif

//const unsigned char ThePinKey[DES_KEY_SIZE+1]  = {"\x42\x25\xEA\x8E\x9A\x0B\x5F\x63"};
const unsigned char ThePinKey[DES_KEY_SIZE + 1] = { "23456789" };

/** Interface function to call the LoadData Scheme
 *  !! WARNING !! Be careful with data at offset 0 to 7
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
 * \param iPPR (I) TRUE for Booster of PPR, FALSE for Booster of EFT
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

void tcik0146(void) {
    int ret = 0;
    char buffer[30];

    trcS("   ***   tcik0146   ***   \n");
    trcS("Run tcik0145 before\n");
    tcik0145();

    //Check Existance of secret area before PK loading
    trcS("Check Existance of secret area\n");
    if(DoesSecretAreaExist(FALSE, ID_SCR_XXX_BANK) == FALSE) {
        goto lblKO;
    } else {
        /* Secret Area exists => so, you can load data in secret area */
        trcS("Load PK encryption key in secret area\n");
        ret =
            LoadDataSecure(8, 8, DES_CIPHERED_MODE | 72, ID_SCR_XXX_BANK,
                           (unsigned char *) &ThePinKey, FALSE);
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
    _DrawWindow(10, 20, 120, 60, 2, (unsigned char *) "tcik0146 KO");
    PaintGraphics();
    ttestall(0, 100);
  lblEnd:
    trcS("End of tcik0146\n");
}
