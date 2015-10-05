/** \file
 * GMA plugins interface functions
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/sys/pgn.c $
 *
 * $Id: pgn.c 2217 2009-03-20 14:52:31Z abarantsev $
 */

#include <string.h>
#include <stdlib.h>
#include <unicapt.h>
#include "sys.h"

#ifdef __PGN_GSM__
#define __PGN_INQ__
#endif

#ifdef __PGN_POR__
#define __PGN_INQ__
#endif

#ifdef __PGN_INQ__
#include <gmaInqStatus.h>
#endif

#ifdef __PGN_SMF__
#include "smfComm.h"
#endif

#undef trcOpt
#define trcOpt (TRCOPT & BIT(trcPGN))

#ifdef __PGN_GSM__
/** Get from GSM plugin IMSI of GSM SIM carte and IMEI of GSM modem.
 * \remark
 *  - To use only when pin is already entered in GSM plugin
 * \param imei (O) Pointer for IMEI value. 15+1 characters should be reserved at imei
 * \param imsi (O) Pointer for IMSI value. 15+1 characters should be reserved at imsi
 * \return non-negative value if OK; negative otherwise
 *
 * If one of the input pointer is zero, the value is not retrieved
 *
 * \header sys\\sys.h
 * \source sys\\pgn.c
 * \test tcab0183.c
 */
int pgnImeiImsi(char *imei, char *imsi) {
    int ret;
    char tmpImsi[15 + 1];
    char tmpImei[15 + 1];

#ifdef WIN32
    strcpy(tmpImei, __IMEI__);
    strcpy(tmpImsi, __IMSI__);
    ret = RET_OK;
#else
    ret = gmaInqGsmImeiImsi(tmpImei, tmpImsi);
    trcFN("gmaInqGsmImeiImsi ret=%d\n", ret);
#endif
    CHECK(ret == RET_OK, lblKO);

    if(imei)
        strcpy(imei, tmpImei);
    if(imsi)
        strcpy(imsi, tmpImsi);

    return ret;
  lblKO:
    return -1;
}
#endif

#ifdef __PGN_GSM__
/** Get from GSM plugin the last error and network layer where error happens
 * \remark
 *  - To use only when pin is already entered in GSM plugin
 * \param connType (O) Pointer to Error Connection Type value 
 * 		 0 - general layer
 *       1 - GPRS layer
 *       2 - GSM layer
 * If one of the input pointer is zero, the value is not retrieved
 *
 * \return error code if any
 * \header sys\\sys.h
 * \source sys\\pgn.c
 * \test tcik0169.c
 */
int pgnLastErr(byte * connType) {
    int ret;
    int16 eGprs, eGsm;

    VERIFY(connType);

    ret = gmaInqGsmLastErrors(&eGprs, &eGsm);

    if(eGprs != RET_OK) {
        *connType = pgnConnGprs;
        ret = eGprs;
    }
    if(eGsm != RET_OK) {
        *connType = pgnConnGsm;
        ret = eGsm;
    }

    return ret;
}

/** Send a message to the GSM plug-in asking a connection to the GPRS network.
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\pgn.c
 * \test tcik0170.c
 */

int pgnStartGprs(void) {
    int ret;
    int16 sta;

    ret = gmaInqGsmGprsConn(&sta);
    CHECK(ret == RET_OK, lblKO);
    CHECK(sta == 0, lblKO);

    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    return ret;
}

/** Send a message to the GSM plug-in asking to stop connection to the GPRS network.
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\pgn.c
 * \test tcik0170.c
 */

int pgnStopGprs(void) {
    int ret;
    uint8 sta;

    sta = TRUE;                 //Perform detach before disconnection
    ret = gmaInqGsmGprsDiscon(sta);

    return ret;
}

#endif

#ifdef __PGN_GSM__
/** Get from GSM plugin the signal level and status
 * \remark
 *  - To use only when pin is already entered in GSM plugin
 * \param rssi (O) Pointer to rssi value (0-31)
 * \param sta (O) Pointer to GPRS status (1 means conncted to GPRS network; 0 means not connected)
 *
 * If one of the input pointer is zero, the value is not retrieved
 *
 * \return signal level if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\pgn.c
 * \test tcab0183.c
 */
int pgnSgnLvl(byte * rssi, byte * sta) {
    int ret;
    uint8 tmpRssi, tmpSta;

    trcS("pgnSgnLvl Beg\n");
#ifdef WIN32
    tmpRssi = 15;
    tmpSta = 1;
    ret = RET_OK;
#else
    ret = gmaInqGsmSignLevel(&tmpRssi, &tmpSta);
    trcFN("gmaInqGsmSignLevel ret=%d\n", ret);
#endif
    CHECK(ret == RET_OK, lblKO);

    if(rssi)
        *rssi = tmpRssi;
    if(sta)
        *sta = tmpSta;

    return tmpSta;
  lblKO:
    return -1;
}

#endif                          //def __PGN_GSM__

#ifdef __PGN_POR__
/** Get from POR plugin the battery level
 * \param docked (O) Is the terminal docked?; if zero, is not filled
 * \return battery level if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\pgn.c
 * \test tcab0183.c
 */
int pgnBatLvl(byte * docked) {
    int ret;
    uint8 lvl;
    uint8 dck;
    uint8 chg;

    trcS("pgnBatLvl Beg\n");
#ifdef WIN32
    lvl = 51;
    dck = 1;
    chg = POR_BATTERY_PACK_CHARGING;
    ret = RET_OK;
#else
    //ret = gmaInqPorBattLevel(&lvl);
    ret = gmaInqPorBattLevelExt(&lvl, &dck, &chg);
    trcFN("gmaInqPorBattLevel ret=%d\n", ret);
#endif
    CHECK(ret == RET_OK, lblKO);
    if(docked)
        *docked = dck;
    return lvl;
  lblKO:
    return -1;
}
#endif

#ifdef __PGN_SMF__
/** Start maintenance session usinf SMF GMA plugin
 * \return 1 if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\pgn.c
 * \test tcab0190.c
 */
int pgnSmfStart(void) {
    int ret;

    ret = smfCommStart(SMF_DIRECT);
    CHECK(ret == RET_OK, lblKO);
    ret = 1;
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    return ret;
}

//patch for the version 1.82.6 (gmaLibPgComm compiled in DEBUG_ON mode)
#include "DebugLib.h"
int16 debugPrintf(enum debugTarget_e target, const char *fmt, ...) {
    return 0;
}
#endif
