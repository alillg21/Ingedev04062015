/** \file
 * Unitary test case tcab0173.
 *
 * tHash performance testing:
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0173.c $
 *
 * $Id: tcab0173.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "pri.h"
#include "tst.h"

#define DIMHSH 128
#define DIMBLK 128
#define DIMOVF 512
#define LEN 8
static tHash hsh;
static int init(void) {
    int ret;

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "init...");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "hshInit...");
    CHECK(ret >= 0, lblKO);
    ret = hshInit(&hsh, 4, 0, LEN, DIMHSH, DIMBLK, DIMOVF); //initialize the hash descriptor
    //in section 4, offset 0
    //key length is LEN
    //number of hash blocks is DIHSH
    //number of cells in a hash block is DIMBLK
    //number of cells in overflow area is DIMOVF
    //It returns the size of non-volatile memory occupied by the hash index
    //It consists of
    // - (DIMHSH*DIMBLK+DIMOVF)(LEN+sizeof(word)): DIMHSH hash blocks and one overflow area
    //Each cell has a size of LEN+sizeof(word): key of length LEN and word containing the index in target multirecord
    //Hash index consists of DIMHSH blocks and one overflow area
    //Each block contains DIMBLK cells and the index of the first vacant cell
    //Overflow area contains DIMOVF cells and the index of the first vacant cell
    CHECK(ret > 0, lblKO);
    ret = dspLS(1, "hshInit OK");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, "hshReset...");
    CHECK(ret >= 0, lblKO);
    ret = hshReset(&hsh);       //fill by zeroes the non volatile memory reserved for hash index
    CHECK(ret > 0, lblKO);
    ret = dspLS(2, "hshReset OK");
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    ret = -1;
  lblEnd:
    tmrPause(1);
    return ret;
}

static card prtTS(const char *msg, card beg) {
    char tmp[prtW + 1];
    char buf[prtW + 1];
    card ts;

    strcpy(buf, msg);
    ts = getTS(1);
    ts -= beg;
    num2dec(tmp, ts, 0);
    strcat(buf, tmp);
    prtS(buf);
    return ts + beg;
}

static int insdel(void) {
    int ret;
    word num, max;
    char buf[256];
    card ts;

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "insdel...");
    CHECK(ret >= 0, lblKO);

    ts = prtTS("ins beg ", 0);
    num = 0;
    while(42) {                 //insert until saturation
        num2hex(buf, num, LEN);
        ret = dspLS(1, buf);
        CHECK(ret >= 0, lblKO);
        ret = hshIns(&hsh, (byte *) buf, num);
        if(ret < 0)
            break;
        num++;
    }
    prtTS("ins end ", 0);

    max = num - 1;
    num2dec(buf, max, 0);
    strcat(buf, " ins: ");
    //Unicapt results: 16272 insertions per 3315 sec: one insertion takes 0.2037 sec
    //Telium results: 16272 insertions per 146 sec: one insertion takes 0.0090 sec
    prtTS(buf, ts);
    prtS("= = = ");

    ts = prtTS("del beg ", 0);
    num = 0;
    while(42) {                 //remove half of keys
        num2hex(buf, num, LEN);
        ret = dspLS(2, buf);
        CHECK(ret >= 0, lblKO);
        ret = hshDel(&hsh, (byte *) buf);
        CHECK(ret >= 0, lblKO);
        if(ret == 0)
            break;
        num += 2;
    }
    prtTS("del end ", 0);
    prtTS("del: ", ts);
    //Unicapt Results: 16272/2=8136 deletions per 2837 sec: one deletion takes 0.3487 sec
    //Telium Results: 16272/2=8136 deletions per 354 sec: one deletion takes 0.0435 sec
    prtS("= = = ");

    ret = max;
    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    ret = -1;
  lblEnd:
    tmrPause(1);
    return ret;
}

static int test(word max) {
    int ret;
    word num;
    char buf[LEN + 1];
    card ts;

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "test...");
    CHECK(ret >= 0, lblKO);
    prtS("=== test ===");

    ts = prtTS("fnd odd beg ", 0);
    num = max;
    while(num--) {              //odds should be found; evens are removed
        if(num % 2) {
            num2hex(buf, num, LEN);
            ret = dspLS(3, buf);
            CHECK(ret >= 0, lblKO);
            ret = hshFind(&hsh, (byte *) buf);
            CHECK(ret == num, lblKO);
        } else {
            //CHECK(ret < 0, lblKO);
        }
    }
    prtTS("fnd odd end ", 0);
    prtTS("fnd odd: ", ts);
    //Unicapt Results: 8136 finds in 1655 sec before clean and 1005 after clean: 0.2034 sec and 0.1235
    //Telium Results: 8136 finds in 320 sec before clean and 158 after clean: 0.0393 sec and 0.0194    
    ret = 1;

    ret = dspLS(3, "...");
    CHECK(ret >= 0, lblKO);
    ts = prtTS("fnd ODD beg ", 0);  //without display
    num = max;
    while(num--) {              //odds should be found; evens are removed
        if(num % 2) {
            num2hex(buf, num, LEN);
            //ret= dspLS(3,buf); CHECK(ret >= 0, lblKO);
            ret = hshFind(&hsh, (byte *) buf);
            CHECK(ret == num, lblKO);
        } else {
            //CHECK(ret < 0, lblKO);
        }
    }
    prtTS("fnd ODD end ", 0);
    prtTS("fnd ODD: ", ts);
    //Unicapt Results: 8136 finds in 1337 sec before clean and 687 after clean: 0.1643 sec and 0.0844
    //Telium Results: 8136 finds in 293 sec before clean and 133 after clean: 0.0360 sec and 0.0163
    ret = 1;

    ts = prtTS("fnd evn beg ", 0);
    num = max;
    while(num--) {              //odds should be found; evens are removed
        if(num % 2) {
            //CHECK(ret == num, lblKO);
        } else {
            num2hex(buf, num, LEN);
            ret = dspLS(3, buf);
            CHECK(ret >= 0, lblKO);
            ret = hshFind(&hsh, (byte *) buf);
            CHECK(ret < 0, lblKO);
        }
    }
    prtTS("fnd evn end ", 0);
    prtTS("fnd evn: ", ts);
    //Unicapt Results:8136 finds in 10843 sec before clean and 1519 after clean: 1.332 sec and 0.18679
    //Telium Results:8136 finds in 3069 sec before clean and 276 after clean: 0.3772 sec and 0.0339
    ret = 1;

    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    ret = -1;
  lblEnd:
    tmrPause(1);
    return ret;
}

static int clean(void) {
    int ret;
    card ts;

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "clean...");
    CHECK(ret >= 0, lblKO);
    prtS("=== clean ===");

    ret = dspLS(1, "hshClean...");
    CHECK(ret >= 0, lblKO);
    ts = prtTS("clean beg ", 0);
    ret = hshClean(&hsh);
    CHECK(ret > 0, lblKO);
    prtTS("clean end ", 0);
    prtTS("clean: ", ts);
    ret = dspLS(1, "hshClean OK");
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    ret = -1;
  lblEnd:
    tmrPause(1);
    return ret;
}

void tcab0173(void) {
    int ret;
    word num;
    byte vol[256];

    nvmStart();
    dbaSetPtr(vol);
    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);
    ret = init();
    CHECK(ret >= 0, lblKO);
    ret = insdel();
    CHECK(ret >= 0, lblKO);
    num = (word) ret;
    ret = test(num);
    CHECK(ret >= 0, lblKO);
    ret = clean();
    CHECK(ret >= 0, lblKO);
    ret = test(num);
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    dspClear();
    dspLS(0, "KO!");
    trcErr(ret);
    tmrPause(3);
  lblEnd:
    dspStop();
    prtStop();
}
