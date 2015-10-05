#include <string.h>
#include "pri.h"
#include "tst.h"

static code const char *srcFile = __FILE__;

//Unitary test case tcjk0001.
//Functions testing:
//  fmtTok

#define MAX_TOK_LEN 32
void tcjk0001()
{
	int ret;
	char src[] = "ABC|DE^F";
	char dst[MAX_TOK_LEN];
	
	char *T1 = "094-123456"; //Tel with countrycode
	char *T2 = "123456"; //Tel with no countrycode
	char CC[3+1]; //3 digit country codes
	
	//Basic test
	ret = fmtTok(dst, src, "|");
	CHECK(ret==3, lblKO);
	CHECK(strcmp(dst, "ABC") == 0, lblKO);
	
	//Length test when token found
	ret = fmtTok(0, src, "$^*");
	CHECK(ret==6, lblKO);
	
	//Length test when token not found
	ret = fmtTok(0, src, "$#*");
	CHECK(ret==8, lblKO); 
	
	//Some boundary tests
	ret = fmtTok(dst, "^ABC*", "^|*");
	CHECK(ret==0, lblKO);
	CHECK(strlen(dst) == 0, lblKO);
	ret = fmtTok(0, "^ABC*", "^|*");
	CHECK(ret==0, lblKO);
	ret = fmtTok(dst, "ABC^", "^|*");
	CHECK(ret==3, lblKO);
	CHECK(strcmp(dst, "ABC") == 0, lblKO);
	ret = fmtTok(0, "ABC^", "^|*");
	CHECK(ret==3, lblKO);
	
	//Failure Test showing improper usage
	ret = fmtTok(0, T1, "-");
	CHECK(ret <= 3, lblKO); //OK
	ret = fmtTok(CC, T1, "-"); //OK
	
	ret = fmtTok(0, T2, "-"); //should return 0 but dosen't
	CHECK(ret <=3, lblKO); //country code is null, not "123456" and should not fail here.
	ret = fmtTok(CC, T2, "-"); //will write "56" past CC buffer if above CHECK is removed.

	dspLS(1, "  !!TEST OK!!  ");
	goto lblEnd;
lblKO:
	Beep();
	dspLS(1, "  ??TEST KO??  ");
lblEnd:
	tmrPause(2);
	
}
