#include "unicapt.h"
#include "resources.h"
#include "tst.h"

static uint8 dat1[L_BUF];
static uint8 dat2[L_BUF];
static uint8 dat3[L_BUF];
static uint8 dat4[L_BUF];

void stub(const char *str) {
    uint32 handle;
    uint8 key;

    hmiOpen("DEFAULT", &handle);
    hmiADDisplayText(handle, 1, 0, (char *) str);
    hmiKeyWait(handle, &key, PSY_INFINITE_TIMEOUT);
    hmiClose(handle);
}

void treatMenu(uint16 mnu) {
    //char buf[256];
    //sprintf(buf,"mnu= %d",mnu);
    //stub(buf);

    switch (mnu) {

          //TELECOMMS
      case ID_NON_UTA_SCK:
          tcik0099();
          break;
      case ID_UTA_AB_SCK:
          tcab0177();
          break;
      case ID_UTA_GPRS_CHN:
          tcik0040();
          break;
      case ID_NON_UTA_GPRS:
          tcik0100();
          break;
      case ID_NON_UTA_SCK_CLS:
          tcik0101();
          break;
      case ID_NON_UTA_GSM_PLUGIN:
          tcik0102();
          break;
          //case ID_UTA_PGN_ATT_DETACH: tcik0107(); break;
          //test cases PERIPHERALS
      case ID_AB_SMART_CARD:
          tcab0007();
          break;
      case ID_UTA_BARCODE:
          tcik0105();
          break;
      case ID_NON_UTA_IK_BARCODE:
          tcik0103();
          break;
      case ID_NON_UTA_SIGN_CAPT:
          tcik0104();
          break;
      case ID_UTA_GUI_BCR:
          tcik0106();
          break;
      case ID_BCR_READ:
          tcik0108();
          break;
          //test cases EMV
      case ID_AB_EMV_INIT:
          tcab0011(1, dat1);
          break;
      case ID_AB_EMV_SELECT:
          tcab0012(1, dat1, dat2);
          break;
      case ID_AB_EMV_FIN_SELECT:
          tcab0013(1, dat1, dat2);
          break;
      case ID_AB_EMV_CONTEXT:
          tcab0014(1, dat1, dat2);
          break;
      case ID_AB_EMV_PREPARE:
          tcab0015(1, dat1, dat2, dat3, dat4);
          break;
      case ID_AB_EMV_CVM:
          tcab0017(1, dat1, dat2, dat3, dat4);
          break;
          //test cases GRAPHIC
      case ID_TCAB0161:
          tcab0161();
          break;

          //case ID_UTA_PRI: stub("UTA PRI selected"); break;
      default:
          stub("Nothing is selected");
          break;
    }
}
