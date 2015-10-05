/// \file tcik0147.c
#include "sdk30.h"
#include "schvar_def.h"
#include "lib_iapp.h"
#include "sys.h"
#include "tst.h"

/** Non UTA unitary test case tcik0147. Get Pin and Pin block calculation testing.
*/

//init secret area, signature card and var numbers
#ifndef ID_SCR_XXX_BANK
#define ID_SCR_XXX_BANK   0x0E012030
#define CARD_NUMBER_SIGN  0x0000EC15
#define VAR_NUMBER_SIGN         0x0E
#endif

#define INTER_CAR_TIMEOUT 5000
static unsigned char TheBlocResult[100];

/** Interface function to call the Free Scheme, author JLM
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

/** SchGetKey
 */
int SchGetKey(int Timeout, FILE * Iapp) {
    int Key;
    unsigned int Cr;

    Key = 0x00;

    Cr = ttestall(IAPP | IAPP_PPR, Timeout * 100);  /* pas d attente evenement keyboard car traite par schéma */
    if((Cr & (IAPP | IAPP_PPR)) != 0) {
        Key = getc(Iapp);
    } else if((Cr & KEYBOARD) != 0) {
        printf("\x1b CANCEL Keyb");
        IAPP_SchPutSync(SYNC_CANCEL, Iapp);
    } else {
        Key = 0x00;
    }

    return (Key);
}

/** Converts PAN (ascii) to a BCD form compatible with ISO-9564
 * \param  PanAscii (I) (ascii) pointer
 * \param  PanLen (I) length
 * \param  BcdBuffer (O) (BCD format) pointer
 */
void PanToBcd(unsigned char *PanAscii, int PanLen, unsigned char *BcdBuffer) {
    int i, j;
    unsigned char tmp;

    memset(BcdBuffer, 0, 8);

    for (i = (PanLen - 13), j = 2; j < 8; i += 2, j++) {
        if((PanAscii[i] >= '0') && (PanAscii[i] <= '9'))
            tmp = PanAscii[i] - '0';
        else if((PanAscii[i] >= 'A') && (PanAscii[i] <= 'F'))
            tmp = PanAscii[i] - 'A' + 10;
        else
            tmp = 0;

        tmp <<= 4;

        if((PanAscii[i + 1] >= '0') && (PanAscii[i + 1] <= '9'))
            tmp += PanAscii[i + 1] - '0';
        else if((PanAscii[i + 1] >= 'A') && (PanAscii[i + 1] <= 'F'))
            tmp += PanAscii[i + 1] - 'A' + 10;

        BcdBuffer[j] = tmp;
    }
}

static void printPinBlock(unsigned char *TheBlocResult) {
    FILE *hPrt = 0;
    int i;
    char buffer[30], bufftemp[10];

    buffer[0] = 0;
    for (i = 1; i <= (int) TheBlocResult[0]; i++) {
        sprintf(bufftemp, "%02X", TheBlocResult[i]);
        strcat(buffer, bufftemp);
    }

    hPrt = fopen("PRINTER", "w");
    SetDefaultFont(_MEDIUM_);

    pprintf("PIN block:\n");
    strcat(buffer, "\n");
    pprintf(buffer);

    fclose(hPrt);

    trcS("PIN block:\n");
    trcS(buffer);
}

void tcik0147(void) {
    int ret = 0;
    char buffer[30];
    unsigned int Key, Cr;
    unsigned char EndEnterPIN = FALSE;
    unsigned int Indice, Column, Line;
    unsigned int ihid_key;
    T_IAPPSync Synchro;
    FILE *Iapp;
    int iret;
    int pan1, pan2;
    T_ISO9564_RESULT iso9564Result;
    unsigned char pan[20] = { "6965201019761853" }; // PAN including checksum digit
    unsigned char panBcd[8];

    trcS("   ***   tcik0147   ***   \n");
    trcS("Run tcik0146 before\n");
    tcik0146();

    SetPprState(FALSE);         // EFT Booster
    Iapp = open_iapp("rw");
    if(Iapp == NULL)
        goto lblKO;
    printf("\x1b");
    printf(" Input your code:");

    Column = 1;
    Line = 1;
    EndEnterPIN = FALSE;
    Indice = 0;
    ret = KO;
    ihid_key = 0x2A;

    // timeOut first char: 6000 ms, other 60000 ms, HidKey : 0x2A
    trcS("get Pin ( 4 to 10 digits)\n");
    ret =
        sch_init_w(ID_SCH_GET_PIN, NAM_SCH_GET_PIN, 6000, 60000, ihid_key,
                   0x0000040A);
    sprintf(buffer, "ret = %d\n", ret);
    trcS(buffer);
    if(ret != OK)
        goto lblKO;

    IAPP_SchGetSync(1000, &Synchro, Iapp);
    if(Synchro == SYNC_VALID) {
        Cr = ttestall(IAPP | IAPP_PPR, 5000 * 100);
        if(((Cr & IAPP) != 0) || ((Cr & IAPP_PPR) != 0)) {
            Key = getc(Iapp);
        } else {
            Key = 0x00;         // timeout
        }
        while(EndEnterPIN != TRUE) {
            if(Key == ihid_key) {
                if(Indice < MAX_PIN_CODE_SIZE) {
                    if(Indice == 0) {
                        gotoxy(Line, Column);
                        printf("               ");
                    }
                    gotoxy(Line, Column + Indice);
                    printf("%c", Key);
                    Indice++;
                } else {
                    trcS("Max size Pin Code\n");
                    IAPP_SchPutSync(SYNC_CANCEL, Iapp);
                    EndEnterPIN = TRUE;
                }
            } else
                switch (Key) {
                  case 0x00:   // TimeOut
                  case 0x01:   //CANCEL Response
                  case T_ANN:  // CANCEL Key
                      EndEnterPIN = TRUE;
                      break;
                  case T_VAL:
                      EndEnterPIN = TRUE;
                      trcFN(" PIN valid(%d key)\n", Indice);    // PIN valid : nb keys-> Indice
                      iret = OK;
                      break;
                  case T_CORR:
                      if(Indice != 0) {
                          Indice--;
                          gotoxy(Line, Column + Indice);
                          printf(" ");
                      }
                      break;
                }
            if(EndEnterPIN != TRUE)
                Key = (unsigned char) SchGetKey(INTER_CAR_TIMEOUT, Iapp);
        }
    }
    sch_end();

    //Encypher Pin
    trcS("Encypher Pin\n");
    PanToBcd(pan, strlen((char *) pan), panBcd);
    memcpy(&pan1, panBcd, 4);
    memcpy(&pan2, &panBcd[4], 4);

    // Load key, last parameter is an ID that will be register by the system
    ret =
        sch_init_w(ID_SCH_ISO9564, "SchIso9564", ISO9564_F0_DES, pan1, pan2,
                   ID_SCR_XXX_BANK);

    if(ret == OK) {
        IAPP_SchGetData(1000, (unsigned char *) &iso9564Result,
                        ANSWER_HEADER_SIZE + RETURN_CODE_SIZE, Iapp);
        if((iso9564Result.ReturnCode == OK)
           && ((iso9564Result.Len - RETURN_CODE_SIZE) == PIN_BLOC_SIZE)) {
            IAPP_SchGetData(1000, (unsigned char *) &iso9564Result.Answer,
                            PIN_BLOC_SIZE, Iapp);
            // for PIN code 1234 and current PIN key,
            // Pinbloc shall be  AE DF 7D 1E F8 00 91 90
            TheBlocResult[0] = PIN_BLOC_SIZE;
            memcpy(&TheBlocResult[1], &iso9564Result.Answer, PIN_BLOC_SIZE);
        }
        ret = iso9564Result.ReturnCode;
        sprintf(buffer, "ret = %d\n", ret);
        trcS(buffer);
        printPinBlock(TheBlocResult);
    }
    sch_end();
    fclose(Iapp);
    ttestall(0, 100);
    goto lblEnd;
  lblKO:
    trcErr(ret);
    _DrawWindow(10, 20, 120, 60, 2, (unsigned char *) "tcik0147 KO");
    PaintGraphics();
    ttestall(0, 100);
  lblEnd:
    FreeSecretAreaId(ID_SCR_XXX_BANK, FALSE);   /* Free Secret Area in EFT Booster */
    trcS("End of tcik0147\n");
}
