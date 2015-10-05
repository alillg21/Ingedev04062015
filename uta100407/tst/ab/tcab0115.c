#include "sys.h"
#include "tst.h"
#include <string.h>
#include <unicapt.h>

static code const char *srcFile = __FILE__;

//Unitary test case tcab0115.
//Small display fonts

#define _dspH 8
#define _dspW 16

static uint32 _hHmi = 0;
static byte _hmiStarted = 0;

static int _dspStart(void) {
    int ret;

    trcFS("%s\n", "_dspStart");
    ret = hmiOpen("DEFAULT", &_hHmi);
    CHECK(ret == RET_OK, lblKO);

    _hmiStarted = 1;
    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

static int _dspStop(void) {
    int ret;

    trcFS("%s\n", "_dspStop");
    ret = hmiClose(_hHmi);
    //CHECK(ret==RET_OK,lblKO);

    _hHmi = 0;
    _hmiStarted = 0;
    return _hHmi;
//lblKO:
//    return -1;
}

static int _dspClear(void) {
    int ret;

    trcFS("%s\n", "_dspClear");

    if(!_hmiStarted) {
        ret = _dspStart();
        CHECK(ret, lblKO);
    }

    ret = hmiADClearLine(_hHmi, HMI_ALL_LINES);
    CHECK(ret == RET_OK, lblKO);

    return _hHmi;
  lblKO:
    return -1;
}

static byte isUpper(char mod) {
    if(mod < 'A')
        return 0;
    if(mod > 'Z')
        return 0;
    return 1;
}

static int _dspLS(int loc, const char *str) {
    int ret;
    char buf[_dspW + 1];

    VERIFY(str);
    VERIFY((loc & 0x0F) < _dspH);
    trcFN("_dspLS loc=%02X ", loc);
    trcFS("str=%s\n", str);

    if(!_hmiStarted) {
        ret = _dspStart();
        CHECK(ret, lblKO);
    }
#define FONT3 HMI_INTERNAL_FONT_3
#define FONT4 HMI_INTERNAL_FONT_4

    memcpy(buf, str, _dspW);
    buf[_dspW] = 0;             //truncate the string if it is too long
    if(loc & 0x20) {            //right justified
        byte len;

        len = (byte) strlen(buf);
        if(len < _dspW) {
            memmove(buf + _dspW - len, buf, len);
            memset(buf, ' ', _dspW - len);
        }
    }

    if(loc & 0x40) {            //bold font
        ret = hmiADFont(_hHmi, FONT4);
    } else {                    //normal font
        ret = hmiADFont(_hHmi, FONT3);
    }
    CHECK(ret == RET_OK, lblKO);

    if(loc & 0x80) {            //reversed mode
        hmiOutputConfig_t cfg;

        //read the configuration to modify it
        ret = hmiOutputConfigRead(_hHmi, &cfg);
        CHECK(ret == RET_OK, lblKO);

        //configure reversed mode
        ret = OpSetForeground(cfg.wgraphicContext, 0);
        CHECK(ret == OPTIC_NO_ERROR, lblKO);
        ret = OpSetBackground(cfg.wgraphicContext, 1);
        CHECK(ret == OPTIC_NO_ERROR, lblKO);

        //clear line and display text
        loc &= 0x0F;
        ret = hmiADClearLine(_hHmi, (uint16) loc);
        CHECK(ret == RET_OK, lblKO);
        ret = hmiADDisplayText(_hHmi, loc, 0, buf);
        CHECK(ret == RET_OK, lblKO);

        //restore default normal mode
        ret = OpSetForeground(cfg.wgraphicContext, 1);
        CHECK(ret == OPTIC_NO_ERROR, lblKO);
        ret = OpSetBackground(cfg.wgraphicContext, 0);
        CHECK(ret == OPTIC_NO_ERROR, lblKO);
    } else {
        loc &= 0x0F;
        //just clear line and display text
        ret = hmiADClearLine(_hHmi, (uint16) loc);
        CHECK(ret == RET_OK, lblKO);
        ret = hmiADDisplayText(_hHmi, loc, 0, buf);
        CHECK(ret == RET_OK, lblKO);
    }

    //restore normal font
    ret = hmiADFont(_hHmi, HMI_INTERNAL_FONT_4);
    CHECK(ret == RET_OK, lblKO);

    return strlen(buf);
  lblKO:
    return -1;
}

void tcab0115(void) {
    int ret;

    //open the associated channel
    ret = _dspStart();
    CHECK(ret >= 0, lblKO);

    hmiADFont(_hHmi, 0);
    hmiADDisplayText(_hHmi, 0, 0, "FONT0");

    hmiADFont(_hHmi, HMI_INTERNAL_FONT_1);
    hmiADDisplayText(_hHmi, 1, 0, "FONT1");

    hmiADFont(_hHmi, HMI_INTERNAL_FONT_2);
    hmiADDisplayText(_hHmi, 2, 0, "FONT2");

    hmiADFont(_hHmi, HMI_INTERNAL_FONT_3);
    hmiADDisplayText(_hHmi, 3, 0, "FONT3");

    hmiADFont(_hHmi, HMI_INTERNAL_FONT_4);
    hmiADDisplayText(_hHmi, 0, 0, "FONT4");

    hmiADFont(_hHmi, HMI_INTERNAL_FONT_5);
    hmiADDisplayText(_hHmi, 1, 0, "FONT5");

    hmiADFont(_hHmi, HMI_INTERNAL_FONT_6);
    hmiADDisplayText(_hHmi, 2, 0, "FONT6");

    hmiADFont(_hHmi, HMI_INTERNAL_FONT_7);
    hmiADDisplayText(_hHmi, 3, 0, "FONT7");

    hmiADFont(_hHmi, HMI_INTERNAL_FONT_8);
    hmiADDisplayText(_hHmi, 0, 0, "FONT8");

    hmiADFont(_hHmi, HMI_INTERNAL_FONT_9);
    hmiADDisplayText(_hHmi, 1, 0, "FONT9");

    hmiADFont(_hHmi, HMI_INTERNAL_FONT_10);
    hmiADDisplayText(_hHmi, 2, 0, "FONT10");

    //display the string "NORMAL" at the 1st line of the screen
    ret = _dspLS(0, "NORMAL");
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    //display the string "INVERTED" at the 2nd line of the screen in inverted mode
    ret = _dspLS(INV(1), "INVERTED");
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    //display the string "BOLD" at the 3rd line of the screen in bold font
    ret = _dspLS(RGT(BLD(2)), "BOLD RIGHT");
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    //display the string "INVERTED BOLD" at the 4th line of the screen in bold font in inverted mode
    ret = _dspLS(INV(BLD(3)), "INVERTED BOLD");
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    //clear display
    ret = _dspClear();
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    goto lblEnd;
  lblKO:
    trcErr(ret);
    tmrPause(3);
  lblEnd:
    _dspStop();                 //close resource
}
