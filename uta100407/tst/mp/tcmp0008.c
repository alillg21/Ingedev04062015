  /** \file
 * Unitary UTA test case for printing.
 * \sa
 *  - prtES()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/mp/tcmp0008.c $
 *
 * $Id: tcmp0008.c 2534 2010-01-06 10:35:08Z abarantsev $
 */
 
#ifdef __TELIUM__
#include <sdk30.h>
#else
#include <string.h>
#endif
#include "sys.h"
#include "tst.h"

#define ALIGN_CENTER "\x1B\x17"
#define ALIGN_RIGHT "\x1B\x18"
#define FONT_BOLD "\x1B\x1A"
#define FONT_HIGH "\x1B\x1D"
#define FONT_BIG "\x1B\x1E"
#define FONT_CONDENSED "\x1B\x20"
#define FONT_MEDIUM "\x1B\x2A"
#define FONT_REVERSED "\x1B\x22"

#define FONT_BOLD_CENTERED "\x1B\x17\x1B\x1A"
#define FONT_BOLD_CENTERED_REV "\x1B\x17\x1B\x1A\x1B\x22"
#define FONT_BOLD_RIGHT "\x1B\x18\x1B\x1A"
#define FONT_BOLD_RIGHT_REV "\x1B\x18\x1B\x1A\x1B\x22"

#define FONT_HIGH_CENTERED "\x1B\x17\x1B\x1D"
#define FONT_HIGH_CENTERED_REV "\x1B\x17\x1B\x1D\x1B\x22"
#define FONT_HIGH_RIGHT "\x1B\x18\x1B\x1D"
#define FONT_HIGH_RIGHT_REV "\x1B\x18\x1B\x1D\x1B\x22"

#define FONT_BIG_CENTERED "\x1B\x17\x1B\x1E"
#define FONT_BIG_CENTERED_REV "\x1B\x17\x1B\x1E\x1B\x22"
#define FONT_BIG_RIGHT "\x1B\x18\x1B\x1E"
#define FONT_BIG_RIGHT_REV "\x1B\x18\x1B\x1E\x1B\x22"

#define FONT_CONDENSED_CENTERED "\x1B\x17\x1B\x20"
#define FONT_CONDENSED_CENTERED_REV "\x1B\x17\x1B\x20\x1B\x22"
#define FONT_CONDENSED_RIGHT "\x1B\x18\x1B\x20"
#define FONT_CONDENSED_RIGHT_REV "\x1B\x18\x1B\x20\x1B\x22"

#define FONT_MEDIUM_CENTERED "\x1B\x17\x1B\x2A"
#define FONT_MEDIUM_CENTERED_REV "\x1B\x17\x1B\x2A\x1B\x22"
#define FONT_MEDIUM_RIGHT "\x1B\x18\x1B\x2A"
#define FONT_MEDIUM_RIGHT_REV "\x1B\x18\x1B\x2A\x1B\x22"

void tcmp0008(void) {
    int ret;

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    prtES(0, "Left");
    prtES(ALIGN_RIGHT, "Right");
    prtES(ALIGN_CENTER, "Center");
    prtES(FONT_BOLD, "Bold");
    prtES(FONT_HIGH, "High");
    prtES(FONT_BIG, "Big");
    prtES(FONT_CONDENSED, "Condensed");
    prtES(FONT_MEDIUM, "Medium");    
    prtES(FONT_REVERSED, "Reversed");

    prtES(FONT_BOLD_CENTERED, "Center Bold");
    prtES(FONT_BOLD_CENTERED_REV, "Center Bold Rev");
    prtES(FONT_BOLD_RIGHT, "Right Bold");
    prtES(FONT_BOLD_RIGHT_REV, "Right Bold Rev");    

    prtES(FONT_HIGH_CENTERED, "Center High");
    prtES(FONT_HIGH_CENTERED_REV, "Center High Rev");
    prtES(FONT_HIGH_RIGHT, "Right High");
    prtES(FONT_HIGH_RIGHT_REV, "Right High Rev");    

    prtES(FONT_BIG_CENTERED, "C Big");
    prtES(FONT_BIG_CENTERED_REV, "C Big Rev");
    prtES(FONT_BIG_RIGHT, "R Big");
    prtES(FONT_BIG_RIGHT_REV, "R Big Rev");    

    prtES(FONT_CONDENSED_CENTERED, "Center Condensed");
    prtES(FONT_CONDENSED_CENTERED_REV, "Center Condensed Rev");
    prtES(FONT_CONDENSED_RIGHT, "Right Condensed");
    prtES(FONT_CONDENSED_RIGHT_REV, "Right Condensed Rev");        

    prtES(FONT_MEDIUM_CENTERED, "Center Med");
    prtES(FONT_MEDIUM_CENTERED_REV, "Center Med Rev");
    prtES(FONT_MEDIUM_RIGHT, "Right Med");
    prtES(FONT_MEDIUM_RIGHT_REV, "Right Med Rev");          

    goto lblEnd;

  lblKO:
    trcErr(ret);
    prtS("KO!");
    dspLS(0, "KO!");
  lblEnd:    
    prtStop();
    dspStop();
}


