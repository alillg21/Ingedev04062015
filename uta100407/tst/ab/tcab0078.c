/** \file
 * Unitary test case tcab0078.
 * Functions testing for GCL Internal modem communication:
 * \sa
 *  - utaGclStart()
 *  - utaGclDial()
 *  - utaGclRecv()
 *  - utaGclSend()
 *  - utaGclStop()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0078.c $
 *
 * $Id: tcab0078.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "sys.h"
#include "tst.h"

static byte patternFound(const tBuffer * buf, const tBuffer * pat) {
    const byte *ptr1;
    const byte *ptr2;
    word len1, len2, idx1, idx2;

    ptr1 = bufPtr(buf);
    len1 = bufLen(buf);
    ptr2 = bufPtr(pat);
    len2 = bufLen(pat);

    idx1 = len1;
    idx2 = len2;
    while(idx2--) {
        if(!idx1)
            return 0;
        idx1--;
        if(ptr1[idx1] != ptr2[idx2])
            return 0;
    }
    VERIFY(memcmp(ptr1 + len1 - len2, ptr2, len2) == 0);
    return 1;
}

void tcab0078(void) {
    int ret, sav;
    int idx;
    word len;
    byte b;
    char *ptr;
    char msg[dspW + 1];
    tBuffer buf;
    byte dat[256];

    //byte trm[] = { ('C' | 0x80), 0 };
    //byte trm[] = { (0x03|0x0D), 0 };
    byte trm[] = { 0x0A, 0 };
    char key;
    tBuffer bPat;
    byte dPat[16];
    byte pat[16];

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);
    memset(msg, 0, dspW + 1);

    //ptr = "8N11200|0-0836065555";
    //ptr= "7E11200|18280";
    //ptr= "?7E12400|0-0836062424";
    ptr = "?8N11200|0-0836062424";
    ret = dspLS(0, "utaGclStart");
    CHECK(ret >= 0, lblKO);
    ret = dspLS(1, ptr);
    CHECK(ret >= 0, lblKO);
    ret = utaGclStart(gclAsyn, ptr);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, "utaGclDial...");
    CHECK(ret >= 0, lblKO);
    ret = utaGclDial();
    CHECK(ret >= 0, lblKO);

    ret = kbdStart(1);
    CHECK(ret >= 0, lblKO);
    sav = 0;
    key = 0;
    trcS("Start");
    do {
        ret = utaGclDialInfo();
        if(sav == ret)
            continue;
        sav = ret;
        switch (ret) {
          case gclStaPreDial:
              ptr = "PreDial";
              break;
          case gclStaDial:
              ptr = "Dial";
              break;
          case gclStaConnect:
              ptr = "Connect";
              break;
          case gclStaLogin:
              ptr = "Login";
              break;
          case gclStaFinished:
              ptr = "Done";
              break;
          default:
              goto lblKO;
        }
        dspLS(2, ptr);
        key = kbdKey();
        trcFN("key=%02x\n", (word) key);
        CHECK(key >= 0, lblKO);
        if(key == 0)
            continue;
        if(key == kbdANN)
            break;
        kbdStop();
        ret = kbdStart(1);
        CHECK(ret >= 0, lblKO);
    } while(ret != gclStaFinished);
    trcS("End\n");
    kbdStop();
    if(key == kbdANN) {
        ret = dspLS(2, "Aborted");
        CHECK(ret >= 0, lblKO);
        goto lblEnd;
    }
    ret = dspLS(2, "utaGclDial OK");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(3, "Receiving per char");
    CHECK(ret >= 0, lblKO);
    for (idx = 0; idx < 6; idx++) {
        ret = utaGclRecv(&b, 20);   //receive characters
        CHECK(ret >= 0, lblKO);
        bin2hex(msg + idx * 2, &b, 1);
        dspLS(2, msg);
    }

    ret = dspLS(3, "Rcv TRANSPAC");
    CHECK(ret >= 0, lblKO);

    bufInit(&buf, dat, 256);
    bufReset(&buf);
    bufInit(&bPat, dPat, 16);
    bufApp(&bPat, (byte *) "\x8D\x0A", 2);
    /*

       for (idx = 0; idx < 64; idx++) {
       ret = utaGclRecv(&b, 20);
       CHECK(ret >= 0, lblKO);
       ret = bufApp(&buf, &b, 1);
       CHECK(ret >= 0, lblKO);
       if(patternFound(&buf, &bPat))
       break;
       //if((b & 0x7F) == 'T')
       //    break;
       }
       prtS(bufPtr(&buf));
     */

    bufReset(&buf);
    ret = utaGclRecvBuf(&buf, trm, 20);
    CHECK(ret >= 0, lblKO);
    prtS((char *) bufPtr(&buf));

    ret = dspLS(1, "Send SET");
    CHECK(ret >= 0, lblKO);
    bufReset(&buf);
    strcpy((char *) pat, "SET 1:0,2:0,3:0,4:8,5:0\x0D");
    ret = utaGclSendBuf(pat, (word) strlen((char *) pat));
    CHECK(ret >= 0, lblKO);

    ret = dspLS(3, "Rcv SET");
    CHECK(ret >= 0, lblKO);
    bufReset(&buf);
    bufReset(&bPat);
    bufApp(&bPat, (byte *) "\x8D\x0A", 2);

    for (idx = 0; idx < 64; idx++) {
        ret = utaGclRecv(&b, 20);
        CHECK(ret >= 0, lblKO);
        ret = bufApp(&buf, &b, 1);
        CHECK(ret >= 0, lblKO);
        if(patternFound(&buf, &bPat))
            break;
    }

    ret = dspLS(1, "Send ParamX25");
    CHECK(ret >= 0, lblKO);
    bufReset(&buf);
    strcpy((char *) pat, "196372527");
    bufApp(&buf, pat, strlen((char *) pat));
    bufSet(&buf, 'D', 1);       //serparator
    bufApp(&buf, (byte *) "\xC2\x09", 2);
    bufApp(&buf, (byte *) "\x05\x04\x12\x01\x11\x30", 6);
    bufApp(&buf, (byte *) "\x08\x01\x01", 3);
    bufApp(&buf, (byte *) "\x0D", 1);   //serparator
    len = bufLen(&buf);
    ret = utaGclSendBuf(bufPtr(&buf), len);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(3, "Rcv COM");
    CHECK(ret >= 0, lblKO);
    bufReset(&buf);
    bufReset(&bPat);
    bufApp(&bPat, (byte *) "\xC3\xCFM", 3);

    for (idx = 0; idx < 64; idx++) {
        ret = utaGclRecv(&b, 20);
        CHECK(ret >= 0, lblKO);
        ret = bufApp(&buf, &b, 1);
        CHECK(ret >= 0, lblKO);
        if(patternFound(&buf, &bPat))
            break;
    }
    bufReset(&buf);
    bufReset(&bPat);
    bufApp(&bPat, (byte *) "\x8D\x0A", 2);

    for (idx = 0; idx < 64; idx++) {
        ret = utaGclRecv(&b, 20);
        CHECK(ret >= 0, lblKO);
        ret = bufApp(&buf, &b, 1);
        CHECK(ret >= 0, lblKO);
        if(patternFound(&buf, &bPat))
            break;
    }

    ret = dspLS(0, "Send ISO 804");
    CHECK(ret >= 0, lblKO);
    bufReset(&buf);
    bufApp(&buf, (byte *) "\xC1\x06\x07\x04\x00\x00\x00\x88", 8);   //entête
    bufApp(&buf, (byte *) "\x08\x04", 2);   //MTI
    bufApp(&buf, (byte *) "\x80\x20\x01\x81\x00\xC6\x00\x00\x20\x00\x00\x00\x00\x00\x00\x00", 16);  //(1)(11)(24)(25,32)()(41,42,46,47)()()(67)()()()()()()()
    bufApp(&buf, (byte *) "\x00\x00\x01", 3);   //11 - numéro d'audit
    bufApp(&buf, (byte *) "\x08\x63", 2);   //24 - ouverture de dialogue + téléparametrage
    bufApp(&buf, (byte *) "\x80\x13", 2);   //25 - premiere initialisation systeme d'acceptation
    bufApp(&buf, (byte *) "\x0B\x00\x00\x00\x00\x00\x10", 7);   //32: bank data
    bufApp(&buf, (byte *) "\x30\x30\x31\x20\x20\x20\x20\x20", 8);   //41 - Identification du systeme d'acceptation
    bufApp(&buf, (byte *) "\x30\x35\x30\x30\x30\x34\x32\x20\x20\x20\x20\x20\x20\x20\x20", 15);  //42 - Identification d'accepteur de la carte
    bufApp(&buf, (byte *) "\x2D", 1);   //46 - Données de configuration du point d'acceptation LLLVAR: length 45
    bufApp(&buf, (byte *) "\xDF\x50\x00\x0C\x31\x30\x31\x35\x32\x30\x35\x31\x30\x30\x37\x30", 16);  //ITP Identifiant de l'appli terminal
    bufApp(&buf, (byte *) "\xDF\x51\x00\x03\x30\x30\x31", 7);   // Numero logique de systeme d'acceptation
    bufApp(&buf, (byte *) "\xDF\x52\x00\x02\x31\x30", 6);   //Type d'architecture de systeme d'acceptation
    bufApp(&buf, (byte *) "\xDF\x54\x00\x01\x30", 5);   //Status d'application
    bufApp(&buf, (byte *) "\xDF\x5F\x00\x07\x30\x35\x30\x30\x30\x34\x32", 11);  //Numero de contrat accepteur
    bufApp(&buf, (byte *) "\x20\x31\x36\x30\x30\x38\x31\x31\x30\x30\x30\x30\x30\x30\x34\x38\x30\x31\x34\x41\x30\x30\x30\x30\x30\x30\x30\x34\x32\x32\x30\x31\x30", 33);  //47 Données complimentaires nationales
    bufApp(&buf, (byte *) "\x00\x00", 2);   //67 - Gestion de transfert 
    len = bufLen(&buf);
    ret = utaGclSendBuf(bufPtr(&buf), len);
    CHECK(ret >= 0, lblKO);
    ret = dspLS(1, "Sending msg804 OK");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(3, "Receiving buf");
    CHECK(ret >= 0, lblKO);
    bufReset(&buf);

    ret = utaGclRecvBuf(&buf, trm, 20);
    CHECK(ret >= 0, lblKO);

    memcpy(msg, bufPtr(&buf), dspW);
    for (idx = 0; idx < dspW; idx++)
        msg[idx] &= 0x7F;       //remove parity bit
    dspLS(2, msg);
    ret = dspLS(3, "Receiving done");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "utaGclStop");
    CHECK(ret >= 0, lblKO);
    ret = utaGclStop();
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = utaGclDialErr();
    switch (ret) {
      case gclErrCancel:
          ptr = "User cancelled";
          break;
      case gclErrDial:
          ptr = "Dial error";
          break;
      case gclErrBusy:
          ptr = "BUSY";
          break;
      case gclErrNoDialTone:
          ptr = "NO DIAL TONE";
          break;
      case gclErrNoCarrier:
          ptr = "NO CARRIER";
          break;
      case gclErrNoAnswer:
          ptr = "NO ANSWER";
          break;
      case gclErrLogin:
          ptr = "LOGIN PBM";
          break;
      default:
          ptr = "KO!";
          break;
    }
    dspLS(0, ptr);
  lblEnd:
    tmrPause(3);
    utaGclStop();
    kbdStop();
    prtStop();
    dspStop();
}
