/** \file
 * Various UNICAPT functions
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/sys/i32.c $
 *
 * $Id: i32.c 2616 2010-04-07 11:16:41Z abarantsev $
 */

#include <string.h>
#include <stdio.h>
#include <unicapt.h>
#include <hmi.h>
#include "sys.h"

#undef trcOpt
#define trcOpt (TRCOPT & BIT(trcI32))

/** Get Process Identifier.
 * \remark
 *    a simple wrapper around psyPidGet.
 * \return process identifier
 * \header sys\\sys.h
 * \source sys\\i32.c
 * \test tcab0050.c
 */
byte getPid(void) {
    psyPid_t ret;

    psyPidGet(&ret);
    return ret;
}

/** Get Application name.
 * \param       name (O) Buffer to accept application name retrieved (the length of buffer is 16+1)
 * \remark
 *    retrieve application name from an internal OS table.
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\i32.c
 * \test tcab0050.c
 */
int getAppName(char *name) {
    int ret, idx, AppIdx;
    amgAppliTable_t tab;

    VERIFY(name);
    memset(name, 0, 16 + 1);

    ret = amgGetAppliTable(&tab);
    CHECK(ret == RET_OK, lblKO);

    for (AppIdx = 0; AppIdx < tab.nbAppli; AppIdx++) {
        if(tab.appliTable[AppIdx].pid == getPid())
            break;
    }

    if(AppIdx >= tab.nbAppli)
        return -1;

    for (idx = 0; idx < 16; idx++) {
        if(tab.appliTable[AppIdx].appliName[idx] == 0x0d)
            break;
        name[idx] = tab.appliTable[AppIdx].appliName[idx];
    }

    return idx;
  lblKO:
    trcErr(ret);
    return -1;
}

/** Get Application Version
 * \param appVer (O) Buffer to save application version (the length of buffer is 3+1)
 * \param appName (I) Binary Name of Application
 * \return strlen(appVer)=3 if found; otherwise -1
 * \header sys\\sys.h
 * \source sys\\i32.c 
 * \test tcnl0026.c
 */
int getAppVer(char *appVer, const char *appName) {
    int ret;
    contentInfo_t inf[AMG_MAX_SOFT];
    int len, dim, idx;
    const char *ptr;

    VERIFY(appVer);
    VERIFY(appName);

    len = strlen(appName);
    memset(appVer, 0, 3 + 1);
    ret = amgGetContent(inf, AMG_MAX_SOFT);
    CHECK(ret >= 0, lblKO);
    dim = ret;
    ret = -1;
    for (idx = 0; idx < dim; idx++) {
        switch (inf[idx].type) {
          case AMG_KERNEL:
              break;
          case AMG_PERIPHERAL:
              break;
          default:
              ptr = inf[idx].fileName;
              if(memcmp(ptr, appName, len) == 0) {
                  memcpy(appVer, ptr + 8, 3);
                  ret = strlen(appVer);
                  break;
              }
        }
    }
    goto lblEnd;

  lblKO:
    trcErr(ret);
    return -1;
  lblEnd:
    return ret;
}

/** Convert date from internal Ingedev format into ASCII format YYMMDD.
 * \param YYMMDD (O) Buffer to accept date converted.
 * \param       date (I) Original date in internal Ingedev format.
 * \return no
 * \header sys\\sys.h
 * \source sys\\i32.c
 * \test tcab0066.c
 */
void dat2asc(char *YYMMDD, card date) {
    uint32 tmp;

    VERIFY(YYMMDD);

    //date,Bits 31-16:       Year (AD)
    //date,Bits 15-8:        Month (1-12)
    //date,Bits 7-0:         Day (1-31)

    tmp = date >> 16;
    tmp %= 100;
    YYMMDD[0] = (char) (tmp / 10 + '0');
    YYMMDD[1] = (char) (tmp % 10 + '0');

    tmp = date & 0xFFFF;
    tmp >>= 8;
    YYMMDD[2] = (char) (tmp / 10 + '0');
    YYMMDD[3] = (char) (tmp % 10 + '0');

    tmp = date & 0xFF;
    YYMMDD[4] = (char) (tmp / 10 + '0');
    YYMMDD[5] = (char) (tmp % 10 + '0');

    YYMMDD[6] = 0;
}

/** Convert time from internal Ingedev format into ASCII format hhmmss.
 * \param hhmmss (O) Buffer to accept time converted.
 * \param time (I) Original date in internal Ingedev format.
 * \return no
 * \header sys\\sys.h
 * \source sys\\i32.c
 * \test tcab0066.c
 */
void tim2asc(char *hhmmss, card time) {
    uint32 tmp;

    VERIFY(hhmmss);

    //time,Bits 31-16:       Hour (0-23)
    //time,Bits 15-8:        Minute (0-59)
    //time,Bits 7-0:         Second (0-59)

    tmp = time >> 16;
    hhmmss[0] = (char) (tmp / 10 + '0');
    hhmmss[1] = (char) (tmp % 10 + '0');

    tmp = time & 0xFFFF;
    tmp >>= 8;
    hhmmss[2] = (char) (tmp / 10 + '0');
    hhmmss[3] = (char) (tmp % 10 + '0');

    tmp = time & 0xFF;
    hhmmss[4] = (char) (tmp / 10 + '0');
    hhmmss[5] = (char) (tmp % 10 + '0');

    hhmmss[6] = 0;
}

/** Get system date and time in format YYMMDDhhmmss.
 * \param YYMMDDhhmmss (O) Buffer[12+1] to accept date and time retrieved.
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\i32.c
 * \test tcab0050.c
 */
int getDateTime(char *YYMMDDhhmmss) {
    int ret;
    uint32 date, time;

    VERIFY(YYMMDDhhmmss);

    ret = psyDateTimeGet(&date, &time);
    CHECK(ret == RET_OK, lblKO);

    dat2asc(YYMMDDhhmmss, date);
    VERIFY(strlen(YYMMDDhhmmss) == 6);

    tim2asc(YYMMDDhhmmss + 6, time);
    VERIFY(strlen(YYMMDDhhmmss) == 12);

    trcFS("getDateTime YYMMDDhhmmss=%s\n", YYMMDDhhmmss);
    ret = 1;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    return ret;
}

/** Convert date from ASCII format YYMMDD into internal Ingedev format.
 * \param date (O) Points to the target date in internal Ingedev format.
 * \param       YYMMDD (I) Original date in ASCII format YYMMDD.
 * \return no
 * \header sys\\sys.h
 * \source sys\\i32.c
*/
void asc2dat(card * date, const char *YYMMDD) {
    uint32 tmp;
    const char *ptr;

    VERIFY(date);
    VERIFY(YYMMDD);
    ptr = YYMMDD;

    //date,Bits 31-16:       Year (AD)
    dec2num(&tmp, ptr, 2);      //YY
    tmp += 2000;
    *date = tmp;
    *date <<= 16;

    //date,Bits 15-8:        Month (1-12)
    ptr += 2;                   //MM
    dec2num(&tmp, ptr, 2);
    tmp <<= 8;
    *date |= tmp;

    //date,Bits 7-0:         Day (1-31)
    ptr += 2;                   //DD
    dec2num(&tmp, ptr, 2);
    *date |= tmp;
}

/** Convert time from ASCII format  hhmmss into internal Ingedev format.
 * \param time (O) points to the target time in internal Ingedev format.
 * \param       hhmmss (I) original time in ASCII format hhmmss.
 * \return no
 * \header sys\\sys.h
 * \source sys\\i32.c
*/
void asc2tim(card * time, const char *hhmmss) {
    uint32 tmp;
    const char *ptr;

    VERIFY(time);
    VERIFY(hhmmss);
    ptr = hhmmss;

    //time,Bits 31-16:       Hour (0-23)
    dec2num(&tmp, ptr, 2);
    *time = tmp;
    *time <<= 16;

    //time,Bits 15-8:        Minute (0-59)
    ptr += 2;                   //mm
    dec2num(&tmp, ptr, 2);
    tmp <<= 8;
    *time |= tmp;

    //time,Bits 7-0:         Second (0-59)
    ptr += 2;                   //ss
    dec2num(&tmp, ptr, 2);
    *time |= tmp;
}

/** Set system date and time in format YYMMDDhhmmss.
 * \param       YYMMDDhhmmss (I) Buffer containing the new date and time
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\i32.c
 * \test tcab0050.c
 */
int setDateTime(const char *YYMMDDhhmmss) {
    int ret;
    uint32 date, time;

    VERIFY(YYMMDDhhmmss);
    trcFS("setDateTime YYMMDDhhmmss=%s\n", YYMMDDhhmmss);

    asc2dat(&date, YYMMDDhhmmss);
    asc2tim(&time, YYMMDDhhmmss + 6);

    ret = psyDateTimeApprovedUpdate(date, time);
    CHECK(ret == RET_OK, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    return ret;
}

#define tmrN 4
static psyTime_t timer[tmrN];

/** Start a timer number tmr for dly/100 seconds.
 *
 * There are 4 timers numbered from 0 to 3.
 *
 * After starting a timer a function tmrGet() should be called to know whether it is over or not.
 *
 * The timer should be over after dly/100 seconds.
 *
 * \param tmr (I)  Timer number 0..tmrN-1
 * \param dly (I)  Initial timer value in hundredth of second
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\i32.c
 * \test tcab0001.c
*/
int tmrStart(byte tmr, int dly) {
    int ret;

    VERIFY(tmr < tmrN);

    ret = psyTimeGet(&timer[tmr]);  //get timestamp
    CHECK(ret == RET_OK, lblKO);

    timer[tmr] += dly;          //calculate the ending timestamp

    return dly;
  lblKO:                       //trace in case of problem only
    trcFS("%s ", "tmrStart");
    trcErr(ret);
    return -1;
}

/** Return the state of the timer tmr.
 * \param tmr (I) Timer number 0..tmrN-1
 * \return
 *    - negative if failure.
 *    - otherwise the number of centiseconds rest.
 * \header sys\\sys.h
 * \source sys\\i32.c
 * \test tcab0001.c
 */
int tmrGet(byte tmr) {
    int ret;
    psyTime_t t;

    VERIFY(tmr < 4);

    ret = psyTimeGet(&t);
    CHECK(ret == RET_OK, lblKO);
    if(t < timer[tmr]) {        //if the ending timestamp is not achieved return delay left
        ret = (int) (timer[tmr] - t);
    } else {                    //the ending timestamp bypassed; finished waiting
        ret = 0;
    }
    return ret;
  lblKO:                       //trace in case of problem only
    trcFS("%s ", "tmrGet");
    trcErr(ret);
    return -1;
}

/** Stop a timer number tmr.
 *
 * Should be called when the timer is no more needed.
 * \param tmr (I) Timer number 0..tmrN-1
 * \return no
 * \header sys\\sys.h
 * \source sys\\i32.c
 * \test tcab0001.c
 */
void tmrStop(byte tmr) {
    VERIFY(tmr < 4);
    timer[tmr] = 0;             //reset timer
}

/** Sleep for dly centiseconds.
 * \param dly (I) Number of centiseconds to sleep
 * \return no
 * \header sys\\sys.h
 * \source sys\\i32.c
 * \sa tmrPause
 */
void tmrSleep(card dly) {
    psyTimerWakeAfter(dly);
}

/** Get Time Stamp.
 * \param unit:
 *  - 'C': return number of centiseconds since 01/01/1970 (the last 31 bits)
 *  - 'c': return number of centiseconds since first call
 *  - 'S': return number of seconds since 01/01/1970 (the last 31 bits)
 *  - 's': return number of seconds since first call
 *  - 'A': return number of seconds since 01/01/1970 (all)
 *  - 'M': return number of minutes since 01/01/1970
 *  - 'm': return number of minutes since first call
 *  - 'H': return number of hours since 01/01/1970
 *  - 'h': return number of hours since first call
 *  - 'D': return number of days since 01/01/1970
 *  - 'd': return number of days since first call
 *  - 'W': return number of weeks since 01/01/1970
 *  - 'w': return number of weeks since first call
 * \return the number of units since first call or since 01/01/1970
 * \header sys\\sys.h
 * \source sys\\i32.c
 * \test tcab0050.c
 *
 * This function is designed for performance testing and timeout management.
*/
int getTS(char unit) {
    int ret;
    psyTime_t t;
    static psyTime_t t0 = 0;
    card m;

    if(unit == 0)               //compatibility with old implementation
        unit = 'C';
    if(unit == 1)               //compatibility with old implementation
        unit = 'c';

    ret = sizeof(t);
    ret = sizeof(int);
    ret = psyTimeGet(&t);
    CHECK(ret == RET_OK, lblKO);
    trcFN("psyTimeGet: %x\n", t);
    if(!t0)
        t0 = t;

    if('a' <= unit && unit <= 'z')
        t -= t0;

    if(unit == 'C' || unit == 'S')
        t &= 0x7FFFFFFFL;

    m = 0;
    switch (unit) {
      case 'c':
      case 'C':
          m = 1;
          break;
      case 's':
      case 'S':
      case 'A':
          m = 100;
          break;
      case 'm':
      case 'M':
          m = 60 * 100;
          break;
      case 'h':
      case 'H':
          m = 60 * 60 * 100;
          break;
      case 'd':
      case 'D':
          m = 24 * 60 * 60 * 100;
          break;
      case 'w':
      case 'W':
          m = 7 * 24 * 60 * 60 * 100;
          break;
      default:
          break;
    }
    VERIFY(m);

    t /= m;
    ret = t;
    trcFN("getTS: %d\n", ret);
    return ret;
  lblKO:                       //trace in case of problem only
    trcErr(ret);
    return -1;
}

static uint32 hMag = 0;

/** Open the associated channel.
 * Start waiting the magnetic stripe card.
 * Normally this function should be called just before magnetic card acquisition.
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\i32.c
 * \test tcab0006.c
 */
int magStart(void) {
    int ret;                    //to be returned: resource handle

    trcFS("%s\n", "magStart");

    ret = magOpen("DEFAULT", &hMag);    //open channel
    CHECK(ret == RET_OK, lblKO);

    ret = magReadReq(hMag, MAG_ALL_ISO);    //start waiting mag stripe swiping
    CHECK(ret == RET_RUNNING, lblKO);

    return 1;
  lblKO:
    return -1;
}

/** Stop waiting the magnetic stripe card.
 * Close the associated channel.
 * Normally this function should be called just after magnetic card acquisition.
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\i32.c
 * \test tcab0006.c
 */
int magStop(void) {
    int ret;                    //to be returned: normally zero

    trcFS("%s\n", "magStop");

    magCancel(hMag);            //stop waiting
    ret = magClose(hMag);       //free resource
    CHECK(ret == RET_OK, lblKO);

    hMag = 0;
    return hMag;
  lblKO:
    return -1;
}

static int getTrk123(char *trk1, char *trk2, char *trk3,
                     const magTripleResult_t * rsp) {
    int ret;                    //to be returned: number of tracks extracted
    char *ptr;                  //pointer to the current track
    byte idx;                   //counter in rsp.mag_resultAvailableTrack array
    byte dim;                   //number of tracks
    byte len;                   //track length
    byte asc;                   //character for ASCII conversion

    VERIFY(rsp);

    ret = rsp->numberOfResults; //number of tracks read
    VERIFY(ret <= 3);
    dim = ret;
    ret = 0;
    for (idx = 0; idx < dim; idx++) {
        switch (rsp->mag_resultAvailableTrack[idx].isoTrackNumber) {
          case 1:
              ptr = trk1;
              asc = 0x20;
              break;
          case 2:
              ptr = trk2;
              asc = 0x30;
              break;
          case 3:
              ptr = trk3;
              asc = 0x30;
              break;
          default:
              continue;
        }
        if(!ptr)
            continue;
        len = (byte) rsp->mag_resultAvailableTrack[idx].isoLength;
        VERIFY(len <= 128);
        memcpy(ptr, rsp->mag_resultAvailableTrack[idx].isoTrack, len);  //get the track
        if(len < 128)
            ptr[len] = 0;       //it is ASCIIZ
        ret++;
        //if(rsp->mag_resultAvailableTrack[idx].isoTrackNumber!=2) continue;
        //while(len--)
        //if(ptr[len] < 0x10)
        //    ptr[len] |= asc;    //convert to ASCII
    }
    if(trk1)
        trcFS("getTrk123 trk1=%s\n", trk1);
    if(trk2)
        trcFS("getTrk123 trk2=%s\n", trk2);
    if(trk3)
        trcFS("getTrk123 trk3=%s\n", trk3);

    return ret;
}

/** Verify whether a card is swiped; if so, copy its tracks into trk1, trk2, trk3 buffers.
 * If a pointer trk1, trk2, or trk3 is zero, the related track data will be ignored.
 * The size of buffers pointed by trk1, trk2, trk3 should be 128.
 * \param trk1 (O) Buffer to capture track1, this size of the buffer should be 128
 * \param trk2 (O) buffer to capture track2, this size of the buffer should be 128
 * \param trk3 (O) Buffer to capture track3, this size of the buffer should be 128
 * \return
 *    - negative if failure.
 *    - number of tracks read.
 *    - zero means that the card is not swiped.
 * \header sys\\sys.h
 * \source sys\\i32.c
 * \test tcab0006.c
 */
int magGet(char *trk1, char *trk2, char *trk3) {
    int ret;                    //to be returned: number of tracks read
    magTripleResult_t rsp;      //os result

    memset(&rsp, 0, sizeof(rsp));
    if(trk1)
        *trk1 = 0;              //zero pointer means that the info is not needed
    if(trk2)
        *trk2 = 0;
    if(trk3)
        *trk3 = 0;
    ret =
        magResultGet(hMag, MAG_ALL_REQUESTED | MAG_WANT_ASCII, sizeof(rsp),
                     &rsp);
    if(ret == RET_RUNNING)
        return 0;               //the card is not swiped yet
    CHECK(ret == RET_OK, lblKO);
    ret = getTrk123(trk1, trk2, trk3, &rsp);
    return ret;
  lblKO:
    return -1;
}

/** Process card input. The application waits for an external event: chip card inserted
 * or magnetic stripe card swiped or a key pressed or 1-minute timeout according to control string
 * ctl settings.
 *
 * The control string contains characters 'c', 'm' or 'k' to indicate what type of input is accepted.
 *
 * If a chip card is inserted the ATR is saved into buf and the value ‘c’ is returned.
 *
 * If a magnetic stripe card is swiped the track1 is saved into buf+0,
 * the track2 into buf+128 and the track3 into buf+256 and the value 'm' is returned.
 * 
 * If a key is pressed it is saved into buf and the value 'k' is returned
 * Timeout of 60 seconds is set. Timer 0 is used.
 *
 * \param buf (I) Buffer to capture the input (character or ATR or 3 tracks)
 * \param ctl (I) control string containing 'c','k','m' to indicate what type of input is accepted
 *
 * \pre 
 *    - buf!=0; the size of the buffer should be not less that 128*3
 *    - ctl!=0
 *
 * \return  
 *    - 'k' if a key is pressed.
 *    - 'm' if a card is swiped.
 *    - 'c' if a card is inserted.
 *    -  0 in case of timeout.
 *    - negative in case of error.
 *
 * \sa Header: sys\\sys.h
 * \sa Source: sys\\i32.c
 * \test tcmp0002.c
 */
int getCard(byte * buf, const char *ctl) {
    int ret, mag, icc, kbd;

    VERIFY(buf);
    VERIFY(ctl);

    memset(buf, 0, 128 * 3);
    mag = -1;
    icc = -1;
    kbd = -1;
    ret = tmrStart(0, 60 * 100);
    CHECK(ret, lblKO);
    while(*ctl) {
        switch (*ctl++) {
          case 'm':
              mag = magStart();
              break;
          case 'c':
              icc = iccStart(0);
              break;
          case 'k':
              kbd = 1;
              break;
          default:
              break;
        }
    }
    ret = kbdStart(1);
    CHECK(ret >= 0, lblKO);

    while(tmrGet(0)) {
        if(icc >= 0) {          //we process chip?
            ret = iccCommand(0, (byte *) 0, (byte *) 0, buf);   //try reset

            if(ret == -iccCardRemoved) {

                ret = 0;        //removed=inserted
            }
            if(ret == -iccDriverPb) {   //driver works fine?

                ret = 0;
                icc = -1;       //don't ask for icc any more
            }
            //change it to falback to mag by nooshin
            //CHK; //other error codes are not acceptable
#ifndef __HYBRDR__
            else if(ret < 0) {

                ret = -iccReadFailure;
                goto lblEnd;
            }
#else
            if(ret < 0) {
                removeCard();
            }
#endif
            if(ret > 0) {
                ret = 'c';      //chip car inserted
                break;
            }
        }

        if(mag >= 0) {          //we process magstripe?
            ret = magGet((char *) buf, (char *) buf + 128, (char *) buf + 256);
            CHECK(ret >= 0, lblKO); //is the mag card swiped?
            if(ret) {
                ret = 'm';      //yes, finished
                break;
            }
        }
        //now check keyboard
        ret = kbdKey();
        CHECK(ret != '\xFF', lblKO);    //errors are not acceptable        
        if(ret) {               //key pressed
            switch (ret) {
              case '0':
              case '1':
              case '2':
              case '3':
              case '4':
              case '5':
              case '6':
              case '7':
              case '8':
              case '9':
                  if(kbd < 0) {
                      Beep();
                      ret = kbdStart(1);
                      CHECK(ret >= 0, lblKO);
                      continue;
                  }
                  *buf = ret;   //put it into buffer
                  ret = 'k';
                  break;
              case kbdANN:
                  *buf = ret;   //put it into buffer
                  ret = 'k';
                  goto lblEnd;
              default:
                  Beep();
                  kbdStop();
                  ret = kbdStart(1);
                  CHECK(ret >= 0, lblKO);
                  continue;
            }
            break;
        }
    }
    if(!tmrGet(0))
        ret = 0;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    tmrStop(0);
    if(mag >= 0)
        magStop();
    if(icc >= 0)
        iccStop(0);
    kbdStop();
    return ret;
}

static uint32 hSmc[5] = { 0, 0, 0, 0, 0 };
static byte iccType = 0;

/** Open the associated channel for the default card reader.
 * \param rdr (I) 
 *  - right nibble: Reader number, 0..4
 *  - left nibble: 0 for EMV cards, 1 for ICC7816
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\i32.c
 * \test tcab0007.c
 */
int iccStart(byte rdr) {
    int ret;
    char tmp[4 + 1];            //used to generate channel name

    trcFN("iccStart rdr=%d\n", (int) rdr);

    iccType = rdr >> 4;
    rdr &= 0x0F;
    VERIFY(rdr < 5);
    strcpy(tmp, "ICC0");
    tmp[3] = '0' + rdr;
    ret = smcOpen(tmp, &hSmc[rdr]);
    CHECK(ret == RET_OK, lblKO);

    return 1;
  lblKO:
    return -1;
}

/** Close the associated channel.
 * \param rdr (I) Reader number, 0..4
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\i32.c
 * \test tcab0007.c
 */
int iccStop(byte rdr) {
    int ret;

    VERIFY(rdr < 5);
    trcFN("iccStop rdr=%d\n", (int) rdr);

    if(hSmc[rdr]) {
        ret = smcClose(hSmc[rdr]);
        CHECK(ret == RET_OK, lblKO);
        hSmc[rdr] = 0;
    }
    return hSmc[rdr];
  lblKO:
    return -1;
}

int iccDetect(byte rdr) {
    int ret;

    VERIFY(rdr < 5);
    trcFN("iccDetect rdr=%d\n", (int) rdr);

    CHECK(hSmc[rdr], lblKO);
    ret = smcDetect(hSmc[rdr]);
    CHECK(ret == RET_OK, lblKO);

    return 1;
  lblKO:
    return 0;
}

#define RSP_LEN 1024
typedef struct {
    int16 sta;
    uint16 len;
    char buf[RSP_LEN];
} tResponse;

enum eCmd { cmdTYP, cmdCLA, cmdINS, cmdP1, cmdP2, cmdLC, cmdEnd };

/** Send a command cmd followed by data dat to the card; capture the result into rsp.
 *
 * The command cmd consists of  5 bytes:
 *  - 0) TYP is the command type, can take values in the range 0..4:
 *    - 0: without any protocol
 *    - 1:  no input, no output
 *    - 2: no input, some output
 *    - 3: some input, no ouput
 *    - 4: some input, some output
 *  - 1) CLA: class byte
 *  - 2) INS: instruction byte
 *  - 3) P1: parameter 1 byte
 *  - 4) P1: parameter 2 byte
 * The data dat consists of the length of data and the data itself.
 * So:
 * - the length of input data, if any, is given in the first byte of dat array
 * - the length of output data, if any, is given in the cmdLC byte of command
 *
 * The pointers can take NULL values:
 *  - If cmd is zero, it means POWER ON command
 *  - If dat is zero, it means that there is no data (the type byte should take proper value in this case)
 *  - If rsp is zero, it means that no output is expected
 *
 * Response length can be given as the first byte of rsp; zero means: get all data
 * \param rdr (I) Reader number, 0..4
 * \param cmd (I) Command to send, consists of 6 bytes: TYP,CLA,INS,P1,P2,LC; for POWER ON command the pointer cmd should be zero
 * \param dat (I) Data to send; zero pointer if there is no data. The first byte is the length of the data; after that the data array follows
 * \param rsp (O) Buffer to capture card response, this size of the buffer should be 260
 * \return
 *    - (-eIcc code) if failure.
 *    - length of response if OK.
 *    - Zero return value means that there is not card inserted (no response).
 * \header sys\\sys.h
 * \source sys\\i32.c
 * \test tcab0007.c
 */
int iccCommand(byte rdr, const byte * cmd, const byte * dat, byte * rsp) {
#define CIT cit.asyncCommand.asyncCommand
    int ret;
    smcCommand_t com;
    union smcCommandIn_t cit;
    tResponse res;

    VERIFY(rdr < 5);

    memset(&com, 0, sizeof(com));
    memset(&cit, 0, sizeof(cit));
    if(cmd) {                   //there is a command to send
        com.commandType = SMC_ASYNC_COMMAND;
        cit.asyncCommand.asyncCommandType = cmd[cmdTYP];
        switch (cmd[cmdTYP]) {
          case 0:              //without any protocol
              VERIFY(dat);
              cit.asyncCommand.asyncCommand.asyncCommandType0.Lc = *dat;
              CIT.asyncCommandType0.ptIn = (uint8 *) (dat + 1);
              break;
          case 1:              //no input, no output
              CIT.asyncCommandType1.CLA = cmd[cmdCLA];
              CIT.asyncCommandType1.INS = cmd[cmdINS];
              CIT.asyncCommandType1.P1 = cmd[cmdP1];
              CIT.asyncCommandType1.P2 = cmd[cmdP2];
              break;
          case 2:              //no input, some output
              VERIFY(rsp);
              CIT.asyncCommandType2.CLA = cmd[cmdCLA];
              CIT.asyncCommandType2.INS = cmd[cmdINS];
              CIT.asyncCommandType2.P1 = cmd[cmdP1];
              CIT.asyncCommandType2.P2 = cmd[cmdP2];
              CIT.asyncCommandType2.Le = cmd[cmdLC];    //output data length is given in command
              break;
          case 3:              //some input, no output
              VERIFY(dat);
              CIT.asyncCommandType3.CLA = cmd[cmdCLA];
              CIT.asyncCommandType3.INS = cmd[cmdINS];
              CIT.asyncCommandType3.P1 = cmd[cmdP1];
              CIT.asyncCommandType3.P2 = cmd[cmdP2];
              CIT.asyncCommandType3.Lc = *dat;  //input data length is given as first data byte
              CIT.asyncCommandType3.ptIn = (uint8 *) (dat + 1);
              break;
          case 4:              //some input, some output
              VERIFY(dat);
              VERIFY(rsp);
              CIT.asyncCommandType4.CLA = cmd[cmdCLA];
              CIT.asyncCommandType4.INS = cmd[cmdINS];
              CIT.asyncCommandType4.P1 = cmd[cmdP1];
              CIT.asyncCommandType4.P2 = cmd[cmdP2];
              CIT.asyncCommandType4.Le = cmd[cmdLC];    //output data length is given in command
              CIT.asyncCommandType4.Lc = *dat;  //input data length is given as first data byte
              CIT.asyncCommandType4.ptIn = (uint8 *) (dat + 1);
              break;
          default:
              VERIFY(cmd[cmdTYP] < 5);
              break;
        }
    } else {                    //no command, POWERON assumed
        byte std = SMC_EMV;

        com.commandType = SMC_NEW_POWER_ON;
        if(iccType) {           //SMC_ISO7816;
            std = SMC_ISO_MAX_LIMIT;
            dat = 0;
        }
        cit.newPowerCommand.cardStandards = std;
        cit.newPowerCommand.GRClassByte00 = SMC_CLA_00;
        cit.newPowerCommand.preferredProtocol = SMC_DEFAULT_PROTOCOL;
    }
    com.commandIn = &cit;
    if(rsp)
        memset(rsp, 0, 260);

    ret = smcCommandReq(hSmc[rdr], &com);   //here the command is sent to the card
    if(ret == SMC_ERR_NO_ICC)
        return 0;
    CHECK(ret == RET_RUNNING, lblKO);
    trcFN("iccCommand rdr= %d cmd=", (int) rdr);
#ifdef __TEST__
    if(!cmd) {
        trcS("NewPowerOn");
    } else {
        trcBN(cmd, cmdEnd);
        if(dat) {
            trcBN(dat, *dat + 1);
        }
    }
#endif
    trcS("\n");
    memset(&res, 0, sizeof(res));

    do {                        //waiting for result
        ret = smcResultGet(hSmc[rdr], SMC_OTHERS, sizeof(res), &res);
    } while(ret == RET_RUNNING);

    if(ret != RET_OK) {
        trcFS("iccCommand %s", "Driver Pb\n");
        return -iccDriverPb;
    }

    switch (res.sta) {
      case RET_OK:             //main branch: no problem detected
          ret = res.len;
          if(rsp) {             //if response anticipated
              memcpy(rsp, res.buf, ret);
              trcFS("iccCommand rsp=", "");
              trcBN(rsp, (word) ret);
              trcS("\n");
          }
          break;
      case SMC_CARD_REMOVAL:
          ret = -iccCardRemoved;
          trcFS("iccCommand %s\n", "Card removed");
          break;
      case SMC_CARD_MUTE:
          ret = -iccCardMute;
          trcFS("iccCommand %s\n", "Card mute");
          break;
      default:                 //there are too many other error types
          ret = -iccCardPb;
          trcFS("iccCommand %s", "Card Pb\n");
          break;
    }
    return ret;
  lblKO:
    return -1;
}

/** Get Random Number. Implemented only for 32 bit.
 * \return RANDOM VALUE
 * \header sys\\sys.h
 * \source sys\\i32.c
*/
card sysRand(void) {
    uint32 rnd;

    psyCryRndNextRandom(&rnd);
    return rnd / 13;            //always returns multiples of 44. (?!)
}

/** Reboot the terminal
 * \header sys\\sys.h
 * \source sys\\i32.c
 * \test tcab0190.c
 */
void reboot(void) {
    psySoftReset();
}

/** Returns the Font copyright string according to copyright string
 * \param copyright (0) The font required. (Null terminated string)
 * \param font (O) The pointer to the copyright string buffer
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\i32.c
 * \test tcjl0008.c
 */
byte getFontInfoByCopyright(char *copyright, char *font) {
    FontInfo *fontInfo;

    if(psyFntGetFontInfoByCopyright(copyright, &fontInfo) != RET_OK) {
        return -1;
    }
    memcpy(font, fontInfo->copyright, 40);

    return 1;
}

/** Get the configuration files and code files and Crcs of the code files in the application 
 * \param fDetails (O) Structure to accept the file names and the Crcs.
 * \param cFile (I) part of configuration file name, first 6 characters.
 * \return number of files
 * \header sys\\sys.h
 * \source sys\\i32.c
 * \test tcma0010.c
*/
int getCodeFileList(tFileDetails * fDetails, char *cFile) {
    int ret, j, FileNo, idx, fNum;
    int16 codFileNo, i;
    char FileName[20];
    contentInfo_t contentInfo[AMG_MAX_SOFT];
    contentInfo_t contentInfoCodeFL[AMG_MAX_SOFT];
    struct0_t contentDetail;
    char FileCrc[20] = "";

    codFileNo = amgGetContent(contentInfo, AMG_MAX_SOFT);

    for (i = 0, j = 0; i < codFileNo; i++) {
        switch (contentInfo[i].type) {
          case AMG_KERNEL:
              break;
          case AMG_PERIPHERAL:
              break;
          default:
              memcpy(&contentInfoCodeFL[j], &contentInfo[i],
                     sizeof(contentInfo_t));
              j++;
              break;
        }
    }

    FileNo = j;

    for (j = 0, fNum = 0; j < FileNo; j++) {
        sprintf(FileName, "%s", contentInfoCodeFL[j].fileName);

        if(memcmp(FileName, cFile, 6) == 0) {
            memcpy(&fDetails[fNum].fileName, contentInfoCodeFL[j].fileName, 12);
            //prtS(fDetails[fNum].fileName);
            fNum++;
            for (idx = 1; idx <= contentInfoCodeFL[j].codeFileNumber; idx++) {
                ret =
                    amgGetContentDetail(AMG_INFO_SOFT_STRUCT0, &contentDetail,
                                        contentInfoCodeFL[j].fileName,
                                        (uint16) idx);
                memcpy(&fDetails[fNum].fileName, contentDetail.fileName, 12);
                //prtS(fDetails[fNum].fileName);

                //For CRC
                fDetails[fNum].fCrc = contentDetail.crc;
                sprintf(FileCrc, "CRC: %d", fDetails[fNum].fCrc);
                //prtS(FileCrc);
                fNum++;
            }
        }
    }

    return fNum;                //Returns the number of files
}
