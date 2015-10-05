/** \file
 * Unitary test case tcjl0000.
 * Functions testing:
 * \sa
 */

#include <string.h>
#include "sdk30.h"
#include "sys.h"
#include "schvar_def.h"

#ifdef __DUKPT_T__
typedef struct {
    int CardNumber;
    int VarNumber;
    unsigned char Rfu[1000];
    unsigned char RootKey[TDES_KEY_SIZE];
} T_ROOT_KEY;

const unsigned char ThePinMasterKey2[DES_KEY_SIZE + 1] = { "12345678" };
const unsigned char TheRootKey2[TDES_KEY_SIZE + 1] =
    { "\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11" };

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

#if 0
void dukpt_load_keys(void) {

    int ret, cr;
    T_ROOT_KEY key;
    char buffer[30];

    //initialisation of parameters for test transport key loading into location 0
    key.CardNumber = CARD_NUMBER_SIGN;
    key.VarNumber = VAR_NUMBER_SIGN;
    memcpy(key.RootKey, TheRootKey2, TDES_KEY_SIZE);

    /* Automatic Creating Secret Area and initialise secret data */
    trcS("Automatic Creating Secret Area,\n");
    trcS("transport key loading.\n");
    ret =
        LoadDataSecure(0, 1024, DATA_NO_CIPHER, ID_SCR_XXX_BANK,
                       (unsigned char *) &key, FALSE);
    sprintf(buffer, "ret = %d\n", ret);
    trcS(buffer);
    if(ret != OK)
        goto lblKO;

  lblKO:
    ret = dspLS(0, "!OK");
    CHECK(ret >= 0, lblKO);
    trcErr(ret);
  lblEnd:
    prtStop();                  //close resource
    dspStop();
    //FreeSecretAreaId(ID_SCR_XXX_BANK, FALSE);
    trcS("tcjl0003 End\n");

    /*
       cr = ExeSchAndWaitEnd (ID_SCH_DUKPT_DES,"SchDukpt",&ret,DUKPT_LOAD_KEY,0,0, mySecretAreaID);
       SEC_DukptLoadKSN
       if (cr != SERR_OK)
       {
       // The scheme cant not be executed (not found, no place…)
       // insert here your treatment
       }
       else
       {
       // execution error code is into “ret”
       // insert here your treatment
       }
     */
}
#endif

void tcjl0003(void) {

    int ret;                    //*< integer value for return code */
    byte loc1 = 1008 / 8;
    byte crt[4];
    byte loc;
    const byte *wrk = (byte *) "\xFC\xAC\x1A\x1B\x0B\x12\x91\x3D";  //  =>  EDE (first part)
    const byte *nul = (byte *) "\x00\x00\x00\x00\x00\x00\x00\x00";  //  =>  Null (second part)

    //const byte *key= (byte *)"\x6A\xC2\x92\xFA\xA1\x31\x5B\x4D";
    byte Dkey[16];

    trcS("tcjl0003 Beg\n");
    memset(crt, 0, 4);

    tcab0021();                 //test key downloading

    ret = prtStart();           //open the associated channel
    CHECK(ret >= 0, lblKO);
    ret = dspStart();           //open the associated channel
    CHECK(ret >= 0, lblKO);

    ret = cryStart('m');
    CHECK(ret >= 0, lblKO);

    //verify that the transport key 1111111111111111 is downloaded at array 1 slot 0
    ret = cryVerify(loc1, crt);
    CHECK(ret >= 0, lblKO);
    CHECK(memcmp(crt, "\x82\xE1\x36\x65", 4) == 0, lblKO);  //it is the certificate of the key 1111111111111111

    loc = 0x24;
    ret = cryDukptInit(loc, wrk);
    CHECK(ret >= 0, lblKO);

    memcpy(Dkey, wrk, 8);       // First part : Key encrypted
    memcpy(Dkey + 8, nul, 8);   // Second part : Null

    ret = cryDukptLoadDerivationKey(loc, (char *) Dkey);
    CHECK(ret >= 0, lblKO);

    ret = cryDukptCreateInitKey();
    CHECK(ret >= 0, lblKO);

    goto lblEnd;

  lblKO:
    ret = dspLS(0, "!OK");
    CHECK(ret >= 0, lblKO);
    trcErr(ret);
  lblEnd:
    prtStop();                  //close resource
    dspStop();
    trcS("tcjl0003 End\n");
}
#else
void tcjl0003(void) {
    return;
}
#endif
