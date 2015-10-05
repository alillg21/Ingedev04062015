#include <string.h>
#include <stdio.h>
#include "pri.h"
#include "tst.h"

static code const char *srcFile = __FILE__;

// Unitary test case tcar0000.
// Tested function :
//  ecrLoadArticle
//  ecrAckVal


void tcar0000(void) {
	char tmp[20];
	int ret;
	char *fnc = "LoadArticle";
	char *ok = " OK";
	char *ko = " KO!";
	
	dspStart();
	dspLS(0, fnc);

	ret = prtStart();
	CHECK(ret >= 0,lblKO);

	ret = ecrStart();
    CHECK(ret == 0,lblKO);

	// Program the phone recharge with unitary amount 1 
	//					plu, bar code,   amount,   desc,   tax level
	ret = ecrLoadArticle(7,"0000000000007","1","064 elektronsk",4);
	CHECK(ret == 0, lblKO);

	//					plu, bar code,       amount,   desc,   tax level
	ret = ecrLoadArticle(1, "8606101120016", "2700", "Karta 1.zona", 5);
	CHECK(ret == 0, lblKO);

	ecrPause(200); // 2s pause

	// Retry with a wrong tax level
	(void) ecrLoadArticle(1, "8606101120016", "2700", "Karta 1.zona", 4);

	// NACK value should be 2
	sprintf(tmp, "%s ACK = %d", fnc, ecrAckVal());
	prtS(tmp);
	dspLS(1, ok);

	goto lblEnd;

  lblKO:
    prtS(fnc); prtS(ko);
	dspLS(1, ko);
	
  lblEnd:
  	ecrStop();
  	prtStop();
	dspStop();
}

