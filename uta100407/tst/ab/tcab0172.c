/** \file
 * Unitary test case tcab0172.
 * tHash methods testing:
 * \sa
 *  - hshInit
 *  - hshReset
 *  - hshOpen
 *  - hshIns
 *  - hshFlush
 *  - hshFind
 *  - hshDel
 *  - hshScanBeg
 *  - hshScan
 *  - hshScanEnd
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0172.c $
 *
 * $Id: tcab0172.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <stdio.h>
#include <string.h>
#include "pri.h"
#include "tst.h"

//#define CFG 4564
#define CFG 5324

#if(CFG==4564)
#define DIMHSH 4
#define DIMBLK 5
#define DIMOVF 6
#define LEN 4
#endif

#if(CFG==5324)
#define DIMHSH 5
#define DIMBLK 3
#define DIMOVF 2
#define LEN 4
#endif

static tHash hsh;

static int trace(void) {
    int ret;
    word blk, idx, lim, vac, dim, val;
    card ofs;
    byte dat[4096];
    tBuffer buf;
    char tmp[256];

    bufInit(&buf, dat, 4096);
    ofs = hsh.ofs;
    for (blk = 0; blk <= hsh.dimHsh; blk++) {
        dim = (word) ((blk == hsh.dimHsh) ? hsh.dimOvf : hsh.dimBlk);

        ret = nvmLoad(hsh.sec, &lim, ofs, sizeof(word));
        CHECK(ret >= 0, lblKO);
        CHECK(lim <= dim, lblKO);
        ofs += sizeof(word);

        ret = nvmLoad(hsh.sec, &vac, ofs, sizeof(word));
        CHECK(ret >= 0, lblKO);
        CHECK(vac <= dim, lblKO);
        ofs += sizeof(word);

        if(blk == hsh.dimHsh) {
            CHECK(lim == hsh.limOvf || lim == hsh.dimOvf + 1, lblKO);
            CHECK(vac == hsh.vacOvf || vac == hsh.dimOvf + 1, lblKO);
        }

        sprintf(tmp, " blk= %d", blk);
        bufAppStr(&buf, tmp);
        sprintf(tmp, " lim= %d", lim);
        bufAppStr(&buf, tmp);
        sprintf(tmp, " vac= %d", vac);
        bufAppStr(&buf, tmp);

        for (idx = 0; idx < dim; idx++) {
            memset(tmp, 0, 256);
            ret = nvmLoad(hsh.sec, tmp, ofs, hsh.len);
            CHECK(ret >= 0, lblKO);
            ofs += hsh.len;
            ret = nvmLoad(hsh.sec, &val, ofs, sizeof(word));
            CHECK(ret >= 0, lblKO);
            ofs += sizeof(word);

            bufAppStr(&buf, " (");
            if(*tmp) {
                CHECK(val & 0x8000, lblKO);
                bufAppStr(&buf, tmp);
                val &= 0x7FFF;
            } else {
                CHECK((val & 0x8000) == 0, lblKO);
                bufAppStr(&buf, "    ");
            }
            bufAppStr(&buf, ",");

            num2hex(tmp, val, 2);
            bufAppStr(&buf, tmp);
            bufAppStr(&buf, ")");
        }
        bufAppStr(&buf, "\n");
    }
    trcFS("trace hash:\n%s", (char *) bufPtr(&buf));
    return 1;
  lblKO:
    return -1;
}

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
    //number of hash blocks is DIMHSH
    //number of cells in a hash block is DIMBLK
    //number of cells in overflow area is DIMOVF
    //It returns the size of non-volatile memory occupied by the hash index
    //It consists of:
    // - (DIMHSH*(DIMBLK+sizeof(word))+DIMOVF)(LEN+sizeof(word)): DIMHSH hash blocks and one overflow area
    //Each cell has a size of LEN+sizeof(word): key of length LEN and word containing the index in target multirecord
    //Hash index consists of DIMHSH blocks and one overflow area
    //Each block contains DIMBLK cells and the index of the first vacant cell
    //Overflow area contains DIMOVF cells, the index of the lase filled cell and the index of the first vacant cell
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

static int insfind(void) {
    int ret;

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "insfind...");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "hshIns...");
    CHECK(ret >= 0, lblKO);
    ret = hshIns(&hsh, (byte *) "0000", 0);
    CHECK(ret >= 0, lblKO);
    ret = trace();
    CHECK(ret >= 0, lblKO);
    ret = hshIns(&hsh, (byte *) "1111", 1);
    CHECK(ret >= 0, lblKO);
    ret = trace();
    CHECK(ret >= 0, lblKO);
    ret = hshIns(&hsh, (byte *) "2222", 2);
    CHECK(ret >= 0, lblKO);
    ret = trace();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(1, "hshIns OK");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(3, "hshFind...");
    CHECK(ret >= 0, lblKO);
    ret = hshFind(&hsh, (byte *) "0000");
    CHECK(ret == 0, lblKO);
    ret = hshFind(&hsh, (byte *) "1111");
    CHECK(ret == 1, lblKO);
    ret = hshFind(&hsh, (byte *) "2222");
    CHECK(ret == 2, lblKO);
    ret = hshFind(&hsh, (byte *) "3333");
    CHECK(ret < 0, lblKO);      //not found
    ret = dspLS(3, "hshFind OK");
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    ret = -1;
  lblEnd:
    tmrPause(1);
    return ret;
}

static int delfind(void) {
    int ret;

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "delfind...");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "hshDel...");
    CHECK(ret >= 0, lblKO);
    ret = hshDel(&hsh, (byte *) "1111");
    CHECK(ret == 1, lblKO);     //deleted 
    ret = trace();
    CHECK(ret >= 0, lblKO);
    ret = hshDel(&hsh, (byte *) "4444");
    CHECK(ret == 0, lblKO);     //not found
    ret = trace();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(1, "hshDel OK");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(3, "hshFind...");
    CHECK(ret >= 0, lblKO);
    ret = hshFind(&hsh, (byte *) "0000");
    CHECK(ret == 0, lblKO);
    ret = hshFind(&hsh, (byte *) "1111");
    CHECK(ret < 0, lblKO);      //it was deleted
    ret = hshFind(&hsh, (byte *) "2222");
    CHECK(ret == 2, lblKO);
    ret = hshFind(&hsh, (byte *) "3333");
    CHECK(ret < 0, lblKO);      //not found
    ret = dspLS(3, "hshFind OK");
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    ret = -1;
  lblEnd:
    tmrPause(1);
    return ret;
}

static int overflow(void) {
    int ret;
    word num, max;
    char buf[LEN + 1];

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "overflow...");
    CHECK(ret >= 0, lblKO);

    ret = hshReset(&hsh);       //fill by zeroes the non volatile memory reserved for hash index
    CHECK(ret > 0, lblKO);

    num = 0;
    while(42) {                 //insert until saturation
        num2hex(buf, num, LEN);
        ret = dspLS(1, buf);
        CHECK(ret >= 0, lblKO);
        ret = hshIns(&hsh, (byte *) buf, num);
        if(ret < 0)
            break;
        ret = trace();
        CHECK(ret >= 0, lblKO);
        num++;
    }

    num = 0;
    while(42) {                 //remove half of keys
        num2hex(buf, num, LEN);
        ret = dspLS(2, buf);
        CHECK(ret >= 0, lblKO);
        ret = hshDel(&hsh, (byte *) buf);
        CHECK(ret >= 0, lblKO);
        if(ret == 0)
            break;
        ret = trace();
        CHECK(ret >= 0, lblKO);
        num += 2;
    }

    num--;
    max = num;
    while(num--) {              //odds should be found; evens are removed
        num2hex(buf, num, LEN);
        ret = dspLS(3, buf);
        CHECK(ret >= 0, lblKO);
        ret = hshFind(&hsh, (byte *) buf);
        if(num % 2) {
            CHECK(ret == num, lblKO);
        } else {
            CHECK(ret < 0, lblKO);
        }
    }
    ret = max;

    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    ret = -1;
  lblEnd:
    tmrPause(1);
    return ret;
}

static int scan(void) {
    int ret;
    char tmp[256];
    word idx;
    word num = 0;

    ret = hshScanBeg(&hsh);
    CHECK(ret > 0, lblKO);

    while(42) {
        num++;
        memset(tmp, 0, sizeof(tmp));
        ret = hshScan(&hsh, tmp, &idx);
        if(!ret)
            continue;
        if(ret < 0)
            break;
        trcFN("scan[%d]: ", num);
        trcFS("%s ", tmp);
        trcFN("%d\n", idx);
    }
    hshScanEnd(&hsh);
    return 1;
  lblKO:
    return -1;
}

static int clean(word num) {
    int ret;
    char buf[LEN + 1];

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "clean...");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "hshClean...");
    CHECK(ret >= 0, lblKO);
    ret = trace();
    CHECK(ret >= 0, lblKO);
    ret = hshClean(&hsh);
    CHECK(ret > 0, lblKO);
    ret = trace();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(1, "hshClean OK");
    CHECK(ret >= 0, lblKO);

    num--;
    while(num--) {              //odds should be found; evens are removed
        num2hex(buf, num, LEN);
        ret = dspLS(2, buf);
        CHECK(ret >= 0, lblKO);
        ret = hshFind(&hsh, (byte *) buf);
        if(num % 2) {
            CHECK(ret == num, lblKO);
        } else {
            CHECK(ret < 0, lblKO);
        }
    }
    ret = dspLS(3, "hshFind OK");
    CHECK(ret >= 0, lblKO);
    ret = 1;

    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    ret = -1;
  lblEnd:
    tmrPause(1);
    return ret;
}

void tcab0172(void) {
    int ret;
    byte vol[256];

    trcS("tcab0172 Beg\n");

    nvmStart();
    dbaSetPtr(vol);
    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = init();
    CHECK(ret >= 0, lblKO);
    ret = insfind();
    CHECK(ret >= 0, lblKO);
    ret = delfind();
    CHECK(ret >= 0, lblKO);
    ret = overflow();
    CHECK(ret >= 0, lblKO);
    ret = scan();
    CHECK(ret >= 0, lblKO);
    ret = clean((word) ret);
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
    trcS("tcab0172 End\n");
}

//----------------------------
//#define DIMHSH 4
//#define DIMBLK 5
//#define DIMOVF 6
//#define LEN 4

//ins:
//0 [5] (0002,02)(0004,04)(0006,06)(0007,07)(0009,09)
//1 [2] (0008,08)(000C,0C)
//2 [5] (0001,01)(0011,11)(0012,12)(0013,13)(0015,15)
//3 [5] (0000,00)(0003,03)(0005,05)(000E,0E)(0010,10)
//4 [6] (000A,0A)(000B,0B)(000D,0D)(000F,0F)(0014,14)(0016,16)

//del:
//0 [2] (    ,05)(    ,00)(    ,01)(0007,07)(0009,09)
//1 [1] (    ,02)(    ,00)
//2 [2] (0001,01)(0011,11)(    ,05)(0013,13)(0015,15)
//3 [3] (    ,05)(0003,03)(0005,05)(    ,04)(    ,00)
//4 [5] (    ,06)(000B,0B)(000D,0D)(000F,0F)(    ,00)(    ,04)

//----------------------------
//#define DIMHSH 5
//#define DIMBLK 3
//#define DIMOVF 2
//#define LEN 4

//ins:
//0 [0] (    ,00)(    ,00)(    ,00)
//1 [2] (000A,0A)(000C,0C)(    ,00)
//2 [3] (0007,07)(0008,08)(0009,09)
//3 [3] (0000,00)(0004,04)(0005,05)
//4 [3] (0001,01)(0002,02)(0003,03)
//5 [2] (0006,06)(000B,0B)

/* -------------------------------------
 blk= 0 lim= 0 vac= 0 (    ,00) (    ,00) (    ,00)
 blk= 1 lim= 0 vac= 1 (    ,03) (    ,01) (    ,00)
 blk= 2 lim= 2 vac= 1 (0007,07) (    ,04) (0009,09)
 blk= 3 lim= 1 vac= 1 (    ,04) (    ,01) (0005,05)
 blk= 4 lim= 2 vac= 1 (0001,01) (    ,04) (0003,03)
 blk= 5 lim= 1 vac= 0 (    ,03) (000B,0B)

 blk= 0 lim= 0 vac= 0 (    ,00) (    ,00) (    ,00)
 blk= 1 lim= 0 vac= 1 (    ,03) (    ,01) (    ,00)
 blk= 2 lim= 2 vac= 1 (0007,07) (    ,04) (0009,09)
 blk= 3 lim= 1 vac= 1 (    ,04) (    ,01) (0005,05)
 blk= 4 lim= 3 vac= 3 (0001,01) (000B,0B) (0003,03)
 blk= 5 lim= 0 vac= 1 (    ,03) (    ,01)

*/
