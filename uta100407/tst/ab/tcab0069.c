#include <string.h>
#include "pri.h"
#include "tst.h"

//Unitary test case tcab0069.
//Functions testing:
// a40ABCD

static int example51(void) {
    int ret;
    char abcd[8 * 4 + 1];
    char *trk2 = ";123456789012345678=9912123456789012345678?";
    const char *ptrA = abcd + 0;
    const char *ptrB = abcd + 8;
    const char *ptrC = abcd + 16;
    const char *ptrD = abcd + 24;

    ret = a40ABCD(abcd, trk2);
    CHECK(memcmp(ptrA, "34567890", 8) == 0, lblKO);
    CHECK(memcmp(ptrB, "12345678", 8) == 0, lblKO);
    CHECK(memcmp(ptrC, "12345678", 8) == 0, lblKO);
    CHECK(memcmp(ptrD, "90123456", 8) == 0, lblKO);

    return 1;
  lblKO:
    return -1;
}

static int example52(void) {
    int ret;
    char abcd[8 * 4 + 1];
    char *trk2 = ";1234567890=99121234?";
    const char *ptrA = abcd + 0;
    const char *ptrB = abcd + 8;
    const char *ptrC = abcd + 16;
    const char *ptrD = abcd + 24;

    ret = a40ABCD(abcd, trk2);
    CHECK(memcmp(ptrA, "00000012", 8) == 0, lblKO);
    CHECK(memcmp(ptrB, "34567890", 8) == 0, lblKO);
    CHECK(memcmp(ptrC, "1234?000", 8) == 0, lblKO);
    CHECK(memcmp(ptrD, "00000000", 8) == 0, lblKO);

    return 1;
  lblKO:
    return -1;
}

static int example61(void) {
    int ret;
    char abcd[8 * 4 + 1];
    char *trk2 = ";1234567890123456789012345678?";
    const char *ptrA = abcd + 0;
    const char *ptrB = abcd + 8;
    const char *ptrC = abcd + 16;
    const char *ptrD = abcd + 24;

    ret = a40ABCD(abcd, trk2);
    CHECK(memcmp(ptrA, "12345678", 8) == 0, lblKO);
    CHECK(memcmp(ptrB, "90123456", 8) == 0, lblKO);
    CHECK(memcmp(ptrC, "34567890", 8) == 0, lblKO);
    CHECK(memcmp(ptrD, "12345678", 8) == 0, lblKO);

    return 1;
  lblKO:
    return -1;
}

static int example62(void) {
    int ret;
    char abcd[8 * 4 + 1];
    char *trk2 = ";1234567890?";
    const char *ptrA = abcd + 0;
    const char *ptrB = abcd + 8;
    const char *ptrC = abcd + 16;
    const char *ptrD = abcd + 24;

    ret = a40ABCD(abcd, trk2);
    CHECK(memcmp(ptrA, "00000012", 8) == 0, lblKO);
    CHECK(memcmp(ptrB, "34567890", 8) == 0, lblKO);
    CHECK(memcmp(ptrC, "12345678", 8) == 0, lblKO);
    CHECK(memcmp(ptrD, "90?00000", 8) == 0, lblKO);

    return 1;
  lblKO:
    return -1;
}

static int exampleTT(void) {
    int ret;
    char abcd[8 * 4 + 1];
    char *trk2 = ";6007932005000000027=49121200000000019?";
    const char *ptrA = abcd + 0;
    const char *ptrB = abcd + 8;
    const char *ptrC = abcd + 16;
    const char *ptrD = abcd + 24;

    ret = a40ABCD(abcd, trk2);
    CHECK(memcmp(ptrA, "79320050", 8) == 0, lblKO);
    CHECK(memcmp(ptrB, "00000027", 8) == 0, lblKO);
    CHECK(memcmp(ptrC, "12000000", 8) == 0, lblKO);
    CHECK(memcmp(ptrD, "00019F00", 8) == 0, lblKO);

    return 1;
  lblKO:
    return -1;
}

void tcab0069(void) {
    int ret;

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = dspClear();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "example51");
    CHECK(ret >= 0, lblKO);
    ret = example51();
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspLS(1, "example52");
    CHECK(ret >= 0, lblKO);
    ret = example52();
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspLS(2, "example61");
    CHECK(ret >= 0, lblKO);
    ret = example61();
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspLS(3, "example62");
    CHECK(ret >= 0, lblKO);
    ret = example62();
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspClear();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "exampleTT");
    CHECK(ret >= 0, lblKO);
    ret = exampleTT();
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
  lblEnd:
    tmrPause(3);
    dspStop();
}
