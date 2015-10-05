#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pri.h"
#include "tc.h"

// Unitary test case tcjd0001.
// Tested functions :
//  ecrLoadArticle, ecrRcvArticle
//  ecrGetAck

//
// PLU structure
// for rcv article function
//
typedef struct
{
	char   bar[15];
	char   dsc[15];
	byte   tax;
	char   amt[10];
} pluRecord;


extern int ecrRcvArticle(word idx, char *bar, char *amt, char *nam, byte *tax);


void tcjd0001(void) {
	int ret;
	char *fnc = "LoadArticle";
	pluRecord plu;
	
	tcDspTitle(fnc, "");

	ret = ecrStart();
    CHECK(ret == 1, lblKO);

	// Program the "064 elektronsk" phone recharge (plu = 7) with unitary amount 100 -> 1.00
	// -----------------------------
	ret = ecrLoadArticle(7, "0000000000007", "100", "064 elektronsk", 4);
	CHECK(ret == 1, lblKO);

	// Retreive the record and check it is OK
	memset(&plu, 0, sizeof(plu));
	ret = ecrRcvArticle(7, plu.bar, plu.amt, plu.dsc, &plu.tax);
	CHECK(ret == 1, lblKO);
	CHECK(atoi(plu.bar) == atoi("00000000000007"), lblKO);
	CHECK(atoi(plu.amt) == 1, lblKO);
	CHECK(strcmp(plu.dsc, "064 elektronsk") == 0, lblKO);
	CHECK(plu.tax == 4, lblKO);
	tcDspS("7: \"064 elektronsk\" set OK");
	
	// Program the "Star  013" phone recharge (plu = 55) with unitary amount 100 -> 1.00
	// -----------------------------
	ret = ecrLoadArticle(55, "9771452199000", "100", "Star  013", 5);
	CHECK(ret == 1, lblKO);

	// Retreive the record and check it is OK
	memset(&plu, 0, sizeof(plu));
	ret = ecrRcvArticle(55, plu.bar, plu.amt, plu.dsc, &plu.tax);
	CHECK(ret == 1, lblKO);
	CHECK(atoi(plu.bar) == atoi("9771452199000"), lblKO);
	CHECK(atoi(plu.amt) == 1, lblKO);
	CHECK(strcmp(plu.dsc, "Star  013") == 0, lblKO);
	CHECK(plu.tax == 5, lblKO);
	tcDspS("55: \"Star  013\" set OK");

	// Program the "Vesti" phone recharge (plu = 57) with unitary amount 100 -> 1.00
	// -----------------------------
	ret = ecrLoadArticle(57, "0230000000506", "100", "Vesti", 5);
	CHECK(ret == 1, lblKO);

	// Retreive the record and check it is OK
	memset(&plu, 0, sizeof(plu));
	ret = ecrRcvArticle(57, plu.bar, plu.amt, plu.dsc, &plu.tax);
	CHECK(ret == 1, lblKO);
	CHECK(atoi(plu.bar) == atoi("0230000000506"), lblKO);
	CHECK(atoi(plu.amt) == 1, lblKO);
	CHECK(strcmp(plu.dsc, "Vesti") == 0, lblKO);
	CHECK(plu.tax == 5, lblKO);
	tcDspS("57: \"Vesti\" set OK");

#if 0
	// Program the "065 mts" phone recharge (plu = 55) with unitary amount 100 -> 1.00
	ret = ecrLoadArticle(55, "9771452199000", "100", "065 mts", 5);
	CHECK(ret == 1, lblKO);

	// Retreive the record and check it is OK
	memset(&plu, 0, sizeof(plu));
	ret = ecrRcvArticle(55, plu.bar, plu.amt, plu.dsc, &plu.tax);
	CHECK(ret == 1, lblKO);
	CHECK(atoi(plu.bar) == atoi("9771452199000"), lblKO);
	CHECK(atoi(plu.amt) == 1, lblKO);
	CHECK(strcmp(plu.dsc, "065 mts") == 0, lblKO);
	// ALWAYS RETURNS "Star  013"
	CHECK(plu.tax == 5, lblKO);
	tcDspS("065 mts set OK");

	// Program the "061 mobilcom" phone recharge (plu = 57) with unitary amount 100 -> 1.00
	ret = ecrLoadArticle(57, "0230000000506", "100", "061 mobilcom", 5);
	CHECK(ret == 1, lblKO);

	// Retreive the record and check it is OK
	memset(&plu, 0, sizeof(plu));
	ret = ecrRcvArticle(57, plu.bar, plu.amt, plu.dsc, &plu.tax);
	CHECK(ret == 1, lblKO);
	CHECK(atoi(plu.bar) == atoi("0230000000506"), lblKO);
	CHECK(atoi(plu.amt) == 1, lblKO);
	CHECK(strcmp(plu.dsc, "061 mobilcom") == 0, lblKO);
	// ALWAYS RETURNS "Vesti"
	CHECK(plu.tax == 5, lblKO);
	tcDspS("061 mobilcom set OK");

	// upload article 4	
	ret = ecrLoadArticle(4, "8606102514029", "5000", "Lut.Blago Tebi", 2);
	CHECK(ret == 1, lblKO);

	// Retreive the record and check it is OK
	memset(&plu, 0, sizeof(plu));
	ret = ecrRcvArticle(4, plu.bar, plu.amt, plu.dsc, &plu.tax);
	CHECK(ret == 1, lblKO);
	CHECK(atoi(plu.bar) == atoi("8606102514029"), lblKO);
	CHECK(atoi(plu.amt) == 50, lblKO);
	CHECK(strcmp(plu.dsc, "Lut.Blago Tebi") == 0, lblKO);
	CHECK(plu.tax == 2, lblKO);

	// upload article 1	
	ret = ecrLoadArticle(1, "8606101120016", "2700", "Karta 1.zona", 5);
	CHECK(ret == 1, lblKO);

	// Retry with a wrong tax level
	ret = ecrLoadArticle(1, "8606101120016", "2700", "Karta 1.zona", 4);

	// NACK value should be 2
	CHECK(ecrGetAck() == 2, lblKO);
#endif

	tcDspS("OK");
	goto lblEnd;
	
  lblKO:
	tcPrt(fnc);
	tcDspFN("KO : ACK = %d", ecrGetAck());
	tcPrt(0);  // prints the last displayed message
  lblEnd:	
	ecrStop();
}

