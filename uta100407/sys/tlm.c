/** \file
 * Various TELIUM functions
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/sys/tlm.c $
 *
 * $Id: tlm.c 2616 2010-04-07 11:16:41Z abarantsev $
 *
 *Joshua Guo fixed can't get magnet  iso3 issue 2010-03-25
 */

#include <stdlib.h>
#include <sdk30.h>
#include <Libgr.h>
#include "sys.h"
#undef Beep

#undef trcOpt
#define trcOpt (TRCOPT & BIT(trcI32))

#define tmrN 4
typedef struct {
    card hi;                    /*!< \brief Most significant 32bits of the timer value. */
    card lo;                    /*!< \brief Least significant 32bits of the timer value. */
} tTimer;
static tTimer timer[tmrN];

#define EVENT_0 (1<<0)
#define EVENT_1 (1<<1)

#define __PSY_EV_NOWAIT 0x0001
#define __PSY_EV_WAIT 0x0002
#define __PSY_EV_ANY 0x0004
#define __PSY_EV_ALL 0x0008

#define UTA_TASK_NAME "T0"
#define UTA_SEM_NAME0 "S0"
#define UTA_SEM_NAME1 "S1"
#define UTA_SEM_NAME2 "S2"

typedef struct {
    char taskName[2];
    t_topstack *taskID;
    word taskNumber;
} task_t;

static task_t taskList[20];
static word semaphore;
static word mainTaskNumber;
static int tskS;                //secondary task ID

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
 * \source sys\\i32.c sys\\tlm.c
 * \test tcab0001.c
 */
int tmrStart(byte tmr, int dly) {
    tTimer cnt;

    VERIFY(tmr < tmrN);

    /* Get the current timer value */
    cnt.hi = 0;
    cnt.lo = get_tick_counter();    // In 10ms ticks.

    /* Set the timer a 'period' into the future */
    cnt.lo += dly;

    /* If an overflow occurs, adjust the high tick counter */
    //if ((unsigned int)dly > (0xffffffffL - cnt.lo))
    if(cnt.lo < (card) dly)
        (cnt.hi)++;

    /* Assign it to the proper timer */
    timer[tmr].hi = cnt.hi;
    timer[tmr].lo = cnt.lo;

    return dly;
}

/** Return the state of the timer tmr.
 * \param tmr (I) Timer number 0..tmrN-1
 * \return
 *    - negative if failure.
 *    - otherwise the number of centiseconds rest.
 * \header sys\\sys.h
 * \source sys\\i32.c sys\\tlm.c
 * \test tcab0001.c
 */
int tmrGet(byte tmr) {
    card hi, lo;
    tTimer cnt;

    VERIFY(tmr < 4);

    /* Get the new time */
    cnt.hi = timer[tmr].hi;
    cnt.lo = timer[tmr].lo;

    hi = 0;
    lo = get_tick_counter();    // In 10ms ticks.

    if(lo < cnt.lo)
        return (cnt.lo - lo);
    return 0;
}

/** Stop a timer number tmr.
 *
 * Should be called when the timer is no more needed.
 * \param tmr (I) Timer number 0..tmrN-1
 * \return no
 * \header sys\\sys.h
 * \source sys\\i32.c sys\\tlm.c
 * \test tcab0001.c
 */
void tmrStop(byte tmr) {
    VERIFY(tmr < 4);
    timer[tmr].hi = 0;
    timer[tmr].lo = 0;
}

static word FunctionPtr(void) {
    return 0;
}

void tmrSleep(card dly) {
    int ret;
    word delay;
    doubleword time, period;

    delay = GetDelayUser();
    ret = StartLDelay(delay, dly, 0, FunctionPtr);

    switch (ret) {
      case cOK:
          trcS("Delay OK\n");
          break;
      case cBadDelayNumber:
          trcS("Bad Delay Number\n");
          break;
      case cDelaySlotAlreadyUsed:
          trcS("Delay Slot Already Used\n");
          break;
      default:
          trcS("Delay KO\n");
          break;
    }

    StopDelay(delay, &time, &period);
    FreeDelayUser(delay);
    return;

}

/** Make a beep.
 * \return no
 * \header sys\\sys.h
 * \source sys\\i32.c sys\\tlm.c
 */
void utaBeep(void) {
    trcFS("%s\n", "Beep");
    buzzer(10);
}

/** Make a click.
 * \return no
 * \header sys\\sys.h
 * \source sys\\i32.c sys\\tlm.c
 */
void Click(void) {
    trcFS("%s\n", "Beep");
    buzzer(1);
}

static FILE *hMag13 = NULL;
static FILE *hMag2 = NULL;
static FILE *hMag3 = NULL;      //Joshua _a for ISO3

/** Open the associated channel.
 * Start waiting the magnetic stripe card.
 * Normally this function should be called just before magnetic card acquisition.
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\i32.c sys\\tlm.c
 * \test tcab0006.c
 */
int magStart(void) {
    trcFS("%s\n", "magStart");

    hMag13 = fopen("SWIPE31", "r*");
    CHECK(hMag13, lblKO);

    hMag2 = fopen("SWIPE2", "r*");
    CHECK(hMag2, lblKO);

//Joshua _a for ISO3
    hMag3 = fopen("SWIPE3", "r*");
    CHECK(hMag3, lblKO);
    return 1;
  lblKO:
    return -1;
}

static int magTrack1(char *trk1, char *src, char *dst) {
    int ret = 0;                //to be returned: number of tracks read
    unsigned char len = 0;
    char tmp[128];

    if(trk1) {
        memset(tmp, 0, 128);
        ret = is_iso1(hMag13, &len, tmp);
        trcFS("iso1 : %s\n", tmp);
        CHECK(ret >= ISO_OK, lblEnd);
        CHECK(ret < DEF_SEP, lblEnd);
        VERIFY(strlen(tmp) < 128);
        src = tmp;
        dst = trk1;
        while(*src) {           //find start sentinel
            if(*src++ == '%')
                break;
        }
        while(*src) {           //copy all data between start and end sentinels
            if(*src == '?')
                break;
            *dst++ = *src++;
        }
        trcFS("Track 1 : %s\n", trk1);
    }

  lblEnd:
    return ret;
}

static int magTrack2(char *trk2, char *src, char *dst) {
    int ret = 0;                //to be returned: number of tracks read
    unsigned char len = 0;
    char tmp[128];

    if(trk2) {
        memset(tmp, 0, 128);
        ret = is_iso2(hMag2, &len, tmp);
        trcFS("iso2 : %s\n", tmp);
        CHECK(ret >= ISO_OK, lblKO);

        switch (ret) {
          case DEF_SEP:
          case DEF_PAR:
          case DEF_LRC:
#ifndef __NON_PAYMENT_CARD__
          case DEF_LUH:
#endif
          case DEF_NUM:
          case NO_DATA:
              goto lblKO;
              break;
        };
#ifndef __NON_PAYMENT_CARD__
        CHECK(ret < DEF_SEP, lblEnd);
#else
        if(ret) {
            CHECK(ret == DEF_LUH, lblEnd);
        } else {
            CHECK(ret < DEF_SEP, lblEnd);
        }
#endif
        VERIFY(strlen(tmp) < 128);
        src = tmp;
        dst = trk2;
        while(*src) {           //find start sentinel
            if(*src++ == 'B')
                break;
        }
        while(*src) {           //copy all data between start and end sentinels
            if(*src == 'F')
                break;
            if(*src == 'D')
                *src = '=';
            *dst++ = *src++;
        }
        trcFS("Track 2 : %s\n", trk2);
        CHECK(ret >= 0, lblKO);
    }

    goto lblEnd;

  lblKO:
    return -1;
  lblEnd:
    return ret;
}

static int magTrack3(char *trk3, char *src, char *dst) {
    int ret = 0;                //to be returned: number of tracks read
    unsigned char len = 0;
    char tmp[128];

    if(trk3) {
        memset(tmp, 0, 128);
        ret = is_iso3(hMag3, &len, tmp);
        trcFS("iso3 : %s\n", tmp);
        CHECK(ret >= ISO_OK, lblEnd);
        CHECK(ret < DEF_SEP, lblEnd);
        VERIFY(strlen(tmp) < 128);
        src = tmp;
        dst = trk3;
        while(*src) {           //find start sentinel
            if(*src++ == 'B')
                break;
        }
        while(*src) {           //copy all data between start and end sentinels
            if(*src == '?')
                break;
            *dst++ = *src++;
        }
        trcFS("Track 3 : %s\n", trk3);
    }

  lblEnd:
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
 * \source sys\\i32.c sys\\tlm.c
 * \test tcab0006.c
 */
int magGet(char *trk1, char *trk2, char *trk3) {
    int ret;                    //to be returned: number of tracks read
    int ctr;
    int sta;
    int num = 0;
    char *src = 0;
    char *dst = 0;

    VERIFY(hMag13);
    VERIFY(hMag2);
    VERIFY(hMag3);              //Joshua _a for ISO3
    reset_buf(hMag13, _receive_id);
    reset_buf(hMag2, _receive_id);
    reset_buf(hMag3, _receive_id);  //Joshua -a for ISO3

    ctr = 0;
    while(ctr < 1) {
        /*
           sta = ttestall(SWIPE2 | SWIPE31, 100);  // wait for the first event
         */
        //Joshua _c for ISO3
        sta = ttestall(SWIPE2 | SWIPE31 | SWIPE3, 100); // wait for the first event
        if(sta) {
            trcFN("sta1=%x\n", sta);
        }

        if(sta & SWIPE31) {
            sta |= ttestall(SWIPE2, 100);   //wait for the second event
            trcFN("sta2=%x\n", sta);
        } else if(sta & SWIPE2) {
            sta |= ttestall(SWIPE31, 100);  //wait for the second event
            trcFN("sta31=%x\n", sta);
        } else if(sta & SWIPE3) //Joshua _a for ISO3
        {
            sta |= ttestall(SWIPE3, 100);   //wait for the second event
            trcFN("sta31=%x\n", sta);
        }
        ctr++;                  //retry if first event retruns timeout / unable to read track data
    }

    //zero pointer means that the info is not needed
    if(trk1)
        *trk1 = 0;
    if(trk2)
        *trk2 = 0;
    if(trk3)
        *trk3 = 0;

    if(sta & SWIPE31) {
        ret = magTrack1(trk1, src, dst);
        if((ret >= ISO_OK) && (ret < DEF_SEP)) {
            num++;
        }
/*  Joshua _d for EPS
        if(ret >= ISO_OK) {
            ret = magTrack3(trk3, src, dst);
            if((ret < DEF_SEP) && (ret >= ISO_OK)) {
                num++;
            }
        }
*/
    }

    if(sta & SWIPE2) {
        ret = magTrack2(trk2, src, dst);
        CHECK(ret >= ISO_OK, lblKO);
        num++;
    }
    //Joshua _a for ISO3
    if(sta & SWIPE3) {
        ret = magTrack3(trk3, src, dst);
        if((ret < DEF_SEP) && (ret >= ISO_OK)) {
            num++;
        }
    }

    goto lblEnd;
  lblKO:
    return -1;
  lblEnd:
    return num;
}

/** Stop waiting the magnetic stripe card.
 * Close the associated channel.
 * Normally this function should be called just after magnetic card acquisition.
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\i32.c sys\\tlm.c
 * \test tcab0006.c
 */
int magStop(void) {
    int ret;                    //to be returned: normally zero

    trcFS("%s\n", "magStop");
    VERIFY(hMag13);
    VERIFY(hMag2);
    VERIFY(hMag3);              //Joshua _c for ISO3

    ret = fclose(hMag13);
    hMag13 = 0;
    CHECK(ret == 0, lblKO);

    ret = fclose(hMag2);
    hMag2 = 0;
    CHECK(ret == 0, lblKO);

    //Joshua _a for ISO3
    ret = fclose(hMag3);
    hMag3 = 0;
    CHECK(ret == 0, lblKO);
    return 0;
  lblKO:
    return -1;
}

static int getMag(char *trk1, char *trk2, char *trk3, int sta) {
    int ret;                    //to be returned: number of tracks read
    int num = 0;
    char *src = 0;
    char *dst = 0;

    VERIFY(hMag13);
    VERIFY(hMag2);
    VERIFY(hMag3);

    //zero pointer means that the info is not needed
    if(trk1)
        *trk1 = 0;
    if(trk2)
        *trk2 = 0;
    if(trk3)
        *trk3 = 0;

    if(sta & SWIPE31) {
        ret = magTrack1(trk1, src, dst);
        if((ret >= ISO_OK) && (ret < DEF_SEP)) {
            num++;
        }
/*  Joshua _d for ISO3
        if(ret >= ISO_OK) {
            ret = magTrack3(trk3, src, dst);
            if((ret < DEF_SEP) && (ret >= ISO_OK)) {
                num++;
            }
        }
        */
    }

    if(sta & SWIPE2) {
        ret = magTrack2(trk2, src, dst);
        CHECK(ret >= ISO_OK, lblKO);
        num++;
    }
    //Joshua _a for ISO3
    if(sta & SWIPE3) {
        ret = magTrack3(trk3, src, dst);
        if((ret < DEF_SEP) && (ret >= ISO_OK)) {
            num++;
        }
    }

    goto lblEnd;
  lblKO:
    return -1;
  lblEnd:
    return num;
}

/** Process card input. The application waits for an external event: chip card inserted
 * or magnetic stripe card swiped or a key pressed or 30 sec timeout 
 *
 * The control string contains characters 'c', 'm' or 'k' to indicate what type of input is accepted.
 *
 * If a chip card is inserted the ATR is saved into buf and the value �c?is returned.
 *
 * If a magnetic stripe card is swiped the track1 is saved into buf+0,
 * the track2 into buf+128 and the track3 into buf+256 and the value 'm' is returned.
 * 
 * If a key is pressed it is saved into buf and the value 'k' is returned
 * Timeout of 30 seconds is set.
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
 * \sa Source: sys\\tlm.c
 * \test tcmp0002.c
*/
int getCard(byte * buf, const char *ctl) {
    int ret = 0, mag, icc, kbd;
    int sta;

    VERIFY(buf);
    VERIFY(ctl);

    memset(buf, 0, 128 * 3);
    mag = -1;
    icc = -1;
    kbd = -1;

    while(*ctl) {
        switch (*ctl++) {
          case 'm':
              mag = magStart();
              break;
          case 'c':
              icc = iccStart(0);
              break;
          case 'k':
              kbd = kbdStart(1);
              break;
          default:
              break;
        }
    }

    while(1) {
        reset_buf(hMag13, _receive_id);
        reset_buf(hMag2, _receive_id);
        reset_buf(hMag3, _receive_id);  //Joshua _a for ISO3
/*        
        sta = ttestall(KEYBOARD | CAM0 | SWIPE31 | SWIPE2, 30 * 100);   // Wait for an event
        sta |= ttestall(sta ^ (SWIPE31 | SWIPE2 | CAM0), 10);   // Wait for the second event
        sta |= ttestall(sta ^ (SWIPE31 | SWIPE2 | CAM0), 10);   // Wait for the third event
*/
        //Joshua _c for ISO3
        sta = ttestall(KEYBOARD | CAM0 | SWIPE31 | SWIPE2 | SWIPE3, 30 * 100);  // Wait for an event  
        sta |= ttestall(sta ^ (SWIPE31 | SWIPE2 | CAM0 | SWIPE3), 10);  // Wait for the second event
        sta |= ttestall(sta ^ (SWIPE31 | SWIPE2 | CAM0 | SWIPE3), 10);  // Wait for the third event
        sta |= ttestall(sta ^ (SWIPE31 | SWIPE2 | CAM0 | SWIPE3), 10);  // Wait for the third event

        CHECK(sta != 0, lblEnd);    // Exit if no event occured after 30s        

        if(icc >= 0) {          //we process chip?
            ret = iccCommand(0, (byte *) 0, (byte *) 0, buf);   //try reset

            if(ret == -iccCardRemoved) {

                ret = 0;        //removed=inserted
            }
            if(ret == -iccDriverPb) {   //driver works fine?

                ret = 0;
                icc = -1;       //don't ask for icc any more
            }
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
                trcFN("chip OK : %d\n", ret);
                goto lblEnd;
            }
        }

        if(mag >= 0) {          //we process magstripe?                            
            ret =
                getMag((char *) buf, (char *) buf + 128, (char *) buf + 256,
                       sta);
            if(ret) {
                ret = 'm';      //yes, finished
                trcFN("mag OK : %d\n", ret);
                goto lblEnd;
            }
        }

        if(kbd >= 0) {          //we process keyboard?
            ret = kbdKey();
            CHECK(ret != '\xFF', lblKO);    //errors are not acceptable        
            if(ret) {           //key pressed
                switch (ret) {
                  case kbdVAL :
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
                          utaBeep();
                          ret = kbdStart(1);
                          CHECK(ret, lblKO);
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
                      utaBeep();
                      kbdStop();
                      ret = kbdStart(1);
                      CHECK(ret, lblKO);
                      continue;
                }
                goto lblEnd;
            }
        }
    }

  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    if(mag >= 0)
        magStop();
    if(icc >= 0)
        iccStop(0);
    kbdStop();
    return ret;
}

/** Get Random Number 0..9999999
 * \return RANDOM VALUE
 * \header sys\\sys.h
 * \source sys\\i32.c sys\\tlm.c
 */
card sysRand(void) {
    return rand();
}

static FILE *hSmc[5] = { 0, 0, 0, 0, 0 };
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
    char tmp[4 + 1];            //used to generate channel name

    trcFN("iccStart rdr=%d\n", (int) rdr);

    iccType = rdr >> 4;
    rdr &= 0x0F;
    VERIFY(rdr < 5);

    strcpy(tmp, "CAM0");
    tmp[3] = '0' + rdr;

    hSmc[rdr] = 0;
    switch (iccType) {
      case 0:
      case 1:
          hSmc[rdr] = fopen(tmp, "rw*");
          break;
          //case 1:
          //  hSmc[rdr] = stdperif(tmp, 0);
          //  break;
      default:
          VERIFY(iccType <= 1);
          break;
    }
    CHECK(hSmc[rdr], lblKO);

    trcS("iccStart OK\n");
    return 1;
  lblKO:
    return -1;
}

/** Close the associated channel.
 * \param rdr (I) Reader number, 0..4
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\i32.c sys\\tlm.c
 * \test tcab0007.c
 */
int iccStop(byte rdr) {
    VERIFY(rdr < 5);
    trcFN("iccStop rdr=%d\n", (int) rdr);

    if(hSmc[rdr])
        fclose(hSmc[rdr]);
    hSmc[rdr] = 0;
    return 0;
}

int iccDetect(byte rdr) {
    byte stat;

    VERIFY(rdr < 5);
    trcFN("iccDetect rdr=%d\n", (int) rdr);

    CHECK(hSmc[rdr], lblKO);

    status(hSmc[rdr], &stat);
    CHECK(stat & CAM_PRESENT, lblKO);

    trcS("ICC PRESENT\n");
    return 1;

  lblKO:
    trcS("ICC NOT PRESENT\n");
    return 0;
}

static int iccCmdEmv(byte rdr, const byte * cmd, const byte * dat, byte * rsp) {
    int ret;
    T_APDU apduC;               // Command APDU
    T_APDU apduR;               // Response APDU
    byte snd[262];
    byte rcv[262];
    HISTORIC atr;

    trcS("iccCmdEmv\n");
    VERIFY(rdr < 5);

    memset(&apduC, 0, sizeof(apduC));
    memset(&apduR, 0, sizeof(apduR));
    if(rsp)
        memset(rsp, 0, 260);
    if(cmd) {                   //there is a command to send
        memcpy(snd, cmd + 1, 5);
        memcpy(snd + 5, dat + 1, *dat);
        memset(rcv, 0, sizeof(rcv));
        apduC.data = snd;
        apduC.data[4] = *dat;
        apduC.length = 5 + dat[0];
        apduR.data = rcv;

        ret = EMV_apdu(hSmc[rdr], &apduC, &apduR);
        if(ret == 0) {
            trcS("rcv: ");
            trcBuf(rcv, apduR.length);
            trcS("\n");

            if(*rcv == 0x61) {  //retrieve response data
                memcpy(snd, "\x00\xC0\x00\x00", 4);
                snd[4] = rcv[1];
                apduC.length = 5;
                ret = EMV_apdu(hSmc[rdr], &apduC, &apduR);
                if(ret == 0) {
                    ret = apduR.length;
                    if(rsp)
                        memcpy(rsp, rcv, ret);
                } else
                    ret = -ret;
            } else {
                if(rsp)
                    memcpy(rsp, rcv, apduR.length);
            }
        } else
            ret = -ret;
    } else {                    //no command, POWERON assumed
        trcS("Power On\n");
        ret = EMV_power_on(hSmc[rdr], &atr);
        if(ret == 5)            //try once more: the first time usually it does not work
            ret = EMV_power_on(hSmc[rdr], &atr);
        if(ret == 0) {
            trcS("atr: ");
            trcBuf(atr.historic, atr.length);
            trcS("\n");
            ret = 4 + atr.length;   //compatibility issue with ingenico: the first 4 bytes are Answer-To-Reset - TBD
            if(rsp)
                memcpy(rsp + 4, atr.historic, ret);
        } else
            ret = -ret;
    }
    if(ret < 0) {
        switch (-ret) {
          case 1:
          case 7:
              ret = -iccCardRemoved;
              break;
          case 2:
          case 4:
          case 5:
              ret = -iccCardPb;
              break;
          case 3:
              ret = -iccCardMute;
              break;
          case 6:
              ret = -iccReadFailure;
              break;
          default:
              ret = -iccDriverPb;
              break;
        }
    }
    return ret;
}

static int iccCmd7816(byte rdr, const byte * cmd, const byte * dat, byte * rsp) {
    int ret = 0;
    COMMAND_CAM cam;
    HISTORIC atr;

    trcS("iccCmd7816\n");
    VERIFY(rdr < 5);

    memset(&cam, 0, sizeof(cam));
    memset(&atr, 0, sizeof(atr));
    if(rsp)
        memset(rsp, 0, 260);
    if(cmd) {                   //there is a command to send
        cam.header = (byte *) cmd + 1;
        switch (*cmd) {         //command type
          case 0:              //without any protocol
          case 1:              //no input, no output
          case 3:              //some input, no output
              trcS("SV_input_command\n");
              if(dat)
                  cam.data = (byte *) dat + 1;
              ret = SV_input_command(hSmc[rdr], &cam);
              if(ret)
                  ret = -ret;
              break;
          case 2:              //no input, some output
              //case 4: //some input, some output
              trcS("SV_output_command\n");
              VERIFY(rsp);
              cam.data = rsp;
              ret = SV_output_command(hSmc[rdr], &cam);
              if(!ret)          //OK
                  ret = cmd[5]; //length of data to read
              else
                  ret = -ret;
              break;
          default:
              VERIFY(*cmd < 4);
              break;

        }

        if(ret >= 0) {
            if(rsp) {
                rsp[ret++] = cam.statusword1;
                rsp[ret++] = cam.statusword2;
            }
        }
    } else {                    //no command, POWERON assumed
        trcS("Power On\n");
        ret = SV_power_on(hSmc[rdr], &atr);
        if(ret == 0) {
            trcS("atr: ");
            trcBuf(atr.historic, atr.length);
            trcS("\n");
            ret = 4 + atr.length;   //compatibility issue with ingenico: the first 4 bytes are Answer-To-Reset - TBD
            if(rsp)
                memcpy(rsp + 4, atr.historic, ret);
        } else
            ret = -ret;
    }
    if(ret < 0) {
        switch (-ret) {
          case 1:
          case 7:
              ret = -iccCardRemoved;
              break;
          case 2:
          case 4:
          case 5:
              ret = -iccCardPb;
              break;
          case 3:
              ret = -iccCardMute;
              break;
          case 6:
              ret = -iccReadFailure;
              break;
          default:
              ret = -iccDriverPb;
              break;
        }
    }
    return ret;
}

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
 *
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
 * \source sys\\tlm.c
 * \test tcab0007.c
 */
int iccCommand(byte rdr, const byte * cmd, const byte * dat, byte * rsp) {
    int ret;

    VERIFY(rdr < 5);
    if(!hSmc[rdr]) {
        ret = 0;
        goto lblEnd;
    }
    trcS("iccCommand ");
    if(cmd) {
        trcS("cmd: ");
        trcBuf(cmd, 6);
        trcS("\n");
    }
    if(dat) {
        trcS("dat: ");
        trcBuf(dat, *dat + 1);
        trcS("\n");
    }
    switch (iccType) {
      case 0:
          ret = iccCmdEmv(rdr, cmd, dat, rsp);
          break;
      default:
          VERIFY(iccType == 1);
          ret = iccCmd7816(rdr, cmd, dat, rsp);
          break;
    }
    if(rsp && (ret > 0)) {
        trcS("rsp: ");
        trcBuf(rsp, ret);
        trcS("\n");
    }
  lblEnd:
    trcFN("iccCommand ret=%d\n", ret);
    return ret;
}

/** Get system date and time in format YYMMDDhhmmss.
 * \param YYMMDDhhmmss (O) Buffer[12+1] to accept date and time retrieved.
 * \return non-negative value if OK; negative otherwise, Sagem terminals will not fail this function
 * \header sys\\sys.h
 * \source sys\\i32.c sys\\tlm.c
 * \test tcab0050.c
 */
int getDateTime(char *YYMMDDhhmmss) {
    unsigned long ldate;
    DATE Date;
    int yr, mth, dy;
    int hr, min, sec;
    int ret;

    VERIFY(YYMMDDhhmmss);

    ret = read_date(&Date);
    ldate = d_tolong(&Date);

    dy = (Date.day[0] - '0') * 10 + Date.day[1] - '0';
    mth = (Date.month[0] - '0') * 10 + Date.month[1] - '0';
    yr = (Date.year[0] - '0') * 10 + Date.year[1] - '0';

    hr = (Date.hour[0] - '0') * 10 + Date.hour[1] - '0';
    min = (Date.minute[0] - '0') * 10 + Date.minute[1] - '0';
    sec = (Date.second[0] - '0') * 10 + Date.second[1] - '0';

    sprintf(YYMMDDhhmmss, "%02d%02d%02d%02d%02d%02d", yr, mth, dy, hr, min,
            sec);

    trcFS("getDateTime : %s\n", YYMMDDhhmmss);

    return (1);
}

/** Set system date and time in format YYMMDDhhmmss.
 * \param       YYMMDDhhmmss (I) Buffer containing the new date and time
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\i32.c sys\\tlm.c
 * \test tcab0050.c
 */
int setDateTime(const char *YYMMDDhhmmss) {
    DATE Date;
    int ret;

    VERIFY(YYMMDDhhmmss);
    trcFS("setDateTime YYMMDDhhmmss=%s\n", YYMMDDhhmmss);

    read_date(&Date);

    Date.year[0] = YYMMDDhhmmss[0];
    Date.year[1] = YYMMDDhhmmss[1];
    Date.month[0] = YYMMDDhhmmss[2];
    Date.month[1] = YYMMDDhhmmss[3];
    Date.day[0] = YYMMDDhhmmss[4];
    Date.day[1] = YYMMDDhhmmss[5];
    Date.hour[0] = YYMMDDhhmmss[6];
    Date.hour[1] = YYMMDDhhmmss[7];
    Date.minute[0] = YYMMDDhhmmss[8];
    Date.minute[1] = YYMMDDhhmmss[9];
    Date.second[0] = YYMMDDhhmmss[10];
    Date.second[1] = YYMMDDhhmmss[11];

    ret = ctrl_date(&Date);
    CHECK(ret == 0, lblKO);     // Returns 0 if function has been correctly executed or -1 if date is incoherent

    ret = write_date(&Date);
    CHECK(ret == 0, lblKO);     // Returns 0 if function has been correctly executed or -1 if date is incoherent

    ret = 1;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    return ret;
}

/** Get Time Stamp.
 * \param unit:
 *  - 'C': return number of centiseconds since 01/01/1970 (the last 31 bits)
 *  - 'c': return number of centiseconds since first call
 *  - 'S': return number of seconds since 01/01/1970 (the last 31 bits)
 *  - 's': return number of seconds since first call
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
 * \source sys\\tlm.c
 * \test tcab0050.c
 *
 * This function is designed for performance testing and timeout management.
*/
int getTS(char unit) {
    int ret = 0;
    unsigned long t;
    static unsigned long t0 = 0;
    card m;

    if(unit == 0)               //compatibility with old implementation
        unit = 'C';
    if(unit == 1)               //compatibility with old implementation
        unit = 'c';

    t = get_tick_counter();
    trcFN("timestamp: %x\n", t);

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
          goto lblKO;
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

static void (*tskFunction) (card) = 0;
static word tskPatch(card num) {
    VERIFY(tskFunction);
    tskFunction(num);
    return 0;
}

static int taskCreate(char taskName[2], void (*tskFct) (card), int *taskID) {
    task_t task;
    int index;                  //this will be the task identifier        

    tskFunction = tskFct;
    memcpy(task.taskName, taskName, 2);
    task.taskID = fork(tskPatch, 0, 0);
    task.taskNumber = GiveNoTask(task.taskID);
    trcFN("TNC: %d\n", task.taskNumber);

    index = 0;
    while(index < 20 && taskList[index].taskName[0] != 0)
        index++;
    if(index < 20) {
        memcpy(&taskList[index], (task_t *) & task, sizeof(task_t));
        *((int *) (taskID)) = index;
        return 0;
    } else
        return -1;
}

/** Synchronise Tasks Processing
 * Suspend task in progress and wait for task available for task synchronisation
 * \header sys\\sys.h
 * \source sys\\tlm.c
 * \test tcab0056.c
*/
void tskSync(void) {
    ttestall(0, 8);             //timeout = 8 milliseconds
}

/** A wrapper around semaphore acquiring function.
 * Puts the current task into waiting state until a parallel task calls semDec function.
 * \param idx (I) Semaphore index, 0..2
 * \return
 *    - negative if failure.
 *    - otherwise semaphore identifier.
 * \header sys\\sys.h
 * \source sys\\tlm.c
 * \test tcab0056.c
*/
int semInc(byte idx) {
    int ret;

    trcFN("semInc Beg idx= %d\n", (int) idx);
    VERIFY(idx < 3);
    /*
       Request a semaphore unit: P

       The calling task is requesting allocation of a unit in the semaphore
       identified by <SemaphoreNumber>. If no unit is available, the calling
       task is inserted at the end of the task queue associated with
       semaphore <SemaphoreNumber> and waits until one unit is
       available. The unit count associated with the semaphore is
       decremented.
       In order to limit the waiting time, a maximum value <TimeOut> can be
       specified. <Timeout> is specified in terms of number of 10
       milliseconds. If the time-out occurs before the granting of the unit, the
       task is reactivated and will resume with a status cTime-out. A value of
       zero means that no limit is requested.

       Source: TELIUM Real Time Kernel User Guide rev A
     */
    ret = P(semaphore, 0);
    CHECK(ret == cOK, lblKO);
    ret = 1;
    goto lblEnd;
  lblKO:
    ret = -1;
    goto lblEnd;
  lblEnd:
    trcFN("semInc ret= %d\n", ret);
    return (int) semaphore;
}

/** A wrapper around semaphore releasing function.
 * Allows parallel task to continue processing.
 * \param idx (I) Semaphore index, 0..2
 * \return
 *    - negative if failure.
 *    - otherwise semaphore identifier.
 * \header sys\\sys.h
 * \source sys\\tlm.c
 * \test tcab0056.c
*/
int semDec(byte idx) {
    int ret;

    trcFN("semDec Beg idx= %d\n", (int) idx);
    VERIFY(idx < 3);
    /*
       Release a semaphore unit: V

       The calling task is releasing a unit of the semaphore identified by
       <SemaphoreNumber>. The unit count associated with the semaphore
       is incremented and if any tasks are waiting, the first is dequeued from
       the task queue associated with the semaphore, put back in the
       READY or CURRENT state and the unit is granted to it.
       Should the maximum value of the semaphore count be exceeded, the
       unit count remains unchanged and a cSemOverflow status returned.

       Source: TELIUM Real Time Kernel User Guide rev A 
     */
    ret = V(semaphore);
    CHECK(ret == cOK, lblKO);
    ret = 1;
    goto lblEnd;
  lblKO:
    ret = -1;
    goto lblEnd;
  lblEnd:
    trcFN("semDec ret= %d\n", ret);
    return (int) semaphore;
}

/** Start a secondary task performed by the function fun with the argument arg.
 * \param fun (I) Function to be performed as a secondary task 
 * \param arg (I) An argument to be transferred to the function fun
 * \return
 *    - negative if failure.
 *    - otherwise secondary task identifier.
 * \header sys\\sys.h
 * \source sys\\tlm.c
 * \test tcab0056.c
*/
int tskStart(void (*fun) (card), card arg) {
    int ret;
    int maxSemaphore = 3;

    VERIFY(fun);
    trcS("tskStart Beg\n");

    mainTaskNumber = CurrentTask();
    semaphore = GetSemaphoreUser();
    InitSemaphore(semaphore, maxSemaphore);

    ret = taskCreate(UTA_TASK_NAME, fun, &tskS);
    CHECK(ret == cOK, lblKO);

    goto lblEnd;
  lblKO:
    return -1;
    goto lblEnd;
  lblEnd:
    trcS("tskStart End\n");
    trcFN("tskStart ret= %d\n", ret);
    return (int) tskS;
}

/** Stop the secondary task. Should be called at the end of secondary task function.
 * \return no
 * \header sys\\sys.h
 * \source sys\\tlm.c
 * \test tcab0056.c
*/
void tskStop(void) {
    int index;                  //this will be the task identifier    

    trcS("tskStop Beg\n");
    FreeSemaphoreUser(semaphore);
    mainTaskNumber = CurrentTask();
    index = 0;
    for (index = 0;
         index < 20 && taskList[index].taskNumber != ((int) mainTaskNumber);
         index++)
        if(index < 20) {
            kill(taskList[index].taskID, "-*");
            memset(&taskList[index], 0, sizeof(task_t));
        }
    trcS("tskStop End\n");
}

/** Initiates a terminal reset.
 * \header sys\\sys.h
 * \source sys\\tlm.c
*/
void reboot(void) {
    exit(0x2000);
}

/** Get Terminal Serial Number and SAP code
 * This function is for retrieving terminal serial number
 * SAP is not implemented yet
 * \param sap (O) char[16+1] S.A.P. code/prod reference of the terminal
 * \param ser (O) char[12+1] Serial number of the terminal
 * \param dvc (I) device code (not implemented yet for telium):
 *  - 't' or 0: terminal (zero means 'default')
 *  - 'p': pinpad
 * \return non-negative value if OK; negative otherwise
 *
 * If one of the input pointer is zero, the value is not retrieved
 * \header sys\\sys.h
 * \source sys\\tlmcry.c
 * \test tcjl0000.c
 */
int getSapSer(char *sap, char *ser, char dvc) {
    //unsigned char prod_ser_num[4 + 1];
	unsigned char prod_ser_num[20 + 1];
    int ret;

    memset(prod_ser_num, 0, sizeof(prod_ser_num));

    if(ser) {
    	/*
    	//@@ADE 2014-08-07 COMMENT
        ret = SystemFioctl(SYS_FIOCTL_GET_PRODUCT_SERIAL_NUMBER, prod_ser_num);
        CHECK(ret >= 0, lblKO);
        memcpy(ser, "00", 2);
        ret = bin2hex(ser + 2, (byte *) prod_ser_num, 4);
        CHECK(ret == 8, lblKO);
        */
        PSQ_Give_Serial_Number(prod_ser_num);
        memcpy(ser, prod_ser_num, strlen(prod_ser_num));

    }

    if(sap) {
        ret = SystemFioctl(SYS_FIOCTL_GET_PRODUCT_REFERENCE, sap);
        CHECK(ret >= 0, lblKO);
    }
    goto lblEnd;

  lblKO:
    return -1;
  lblEnd:
    return 1;
}

int getAppName(char *name) {
    int ret;
    object_info_t ObjectInfo;

    VERIFY(name);

    //Get Application ID
    ret =
        ObjectGetInfo(OBJECT_TYPE_APPLI, ApplicationGetCurrent(), &ObjectInfo);
    CHECK(ret >= 0, lblKO);
    memcpy(name, &ObjectInfo.name, sizeof(ObjectInfo.name));
    goto lblEnd;

  lblKO:
    return -1;
  lblEnd:
    return ret;
}

/** Get the configuration files and code files and Crcs of the code files in the application 
 * \param fDetails (O) Structure to accept the file names and the Crcs.
 * \param cFile (I) code file name
 * \return number of files
 * \header sys\\sys.h
 * \source sys\\tlm.c
 * \test tcjl0004.c
*/
int getCodeFileList(tFileDetails * fDetails, char *cFile) {
    int ret = 0;
    object_info_t ObjectInfo;
    int num_of_app = 0, cntr;

    //get number of app in terminal
    num_of_app = num_seg();

    for (cntr = 0; cntr < num_of_app; cntr++) {
        //Get Application ID
        ret = ObjectGetInfo(OBJECT_TYPE_APPLI, cntr, &ObjectInfo);
        CHECK(ret >= 0, lblKO);
        memcpy(fDetails[cntr].fileName, &ObjectInfo.file_name,
               sizeof(ObjectInfo.file_name));
        fDetails[cntr].fCrc = ObjectInfo.crc;
    }
    goto lblEnd;

  lblKO:
    return -1;
  lblEnd:
    return ret;

}

/** Retrieve the application version. 
 * \param appVer (O) Application version in ASCII.
 * \param appName (I) Application name.
 * \return
 *    - -1 if failure.
 *    - otherwise return 1.
 * \header sys\\sys.h
 * \source sys\\tlm.c
 * \test tcfa0006.c
 */
int getAppVer(char *appVer, const char *appName) {
    int ret, len;
    char name[15];
    object_info_t ObjectInfo;

    VERIFY(appName);

    //Get Application ID
    ret =
        ObjectGetInfo(OBJECT_TYPE_APPLI, ApplicationGetCurrent(), &ObjectInfo);
    CHECK(ret >= 0, lblKO);
    memcpy(name, &ObjectInfo.name, sizeof(ObjectInfo.name));

    if(memcmp(name, appName, (strlen(name) - 3)) == 0) {
        len = strlen(name);
        memcpy(appVer, name + (len - 3), 3);
        ret = 1;
        goto lblEnd;
    }

  lblKO:
    return -1;
  lblEnd:
    return ret;
}

