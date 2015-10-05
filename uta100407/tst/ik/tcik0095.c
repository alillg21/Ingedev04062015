#include <string.h>
#include "sys.h"
#include "tst.h"
#include "ImgBgd.c"
static code const char *srcFile = __FILE__;

//Unitary test case tcik0095.
//Testing for TOP5VRS application of background image drawning
extern const unsigned char BMP_Bgd[];
void tcik0095(void) {
    int ret;

    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "Test of BckGnd image");
    CHECK(ret >= 0, lblKO);

    cnvStart();
    //ret = scnDraw(shpImgBgd, 0, 0);
    //CHECK(ret >= 0, lblKO);
    dspLight(100);
    ret = cnvDraw();
    CHECK(ret >= 0, lblKO);

    tmrPause(1);
    dspLS(0, "OK");
    goto lblEnd;
  lblKO:
    trcErr(ret);
    dspLS(0, "KO!");
  lblEnd:
    tmrPause(1);
    prtStop();
    cnvStop();
    dspStop();
}
