#include <string.h>
#include "sys.h"
#include "tc.h"

//static code const char *srcFile = __FILE__;

// Test case tcjd0046
// tclib library test demo
// Unitary test.

enum eId {
	idBeg,
	idAut,
	idAutoClose,
	idCode,
	idConf,
	idDayEndTime,
	idDayStartTime,
	idEcr,
	idFloor,
	idFont,
	idFooter,
	idFtr,
	idHdr,
	idHeader,
	idLine,
	idLocation,
	idLogo,
	idMaintenance,
	idMaxAmt,
	idMaxOpr,
	idMaxTrnAmt,
	idMid,
	idName,
	idOper,	
	idPrefixes,
	idPwdAttempts,
	idPwdTimeout,
	idRole,

	idFile,
	idFtp,
	idLogin,
	idPwd,
	idDir,
	idEnd
};

static const char *strTab[] =
{
	"",
	"aut",
	"autoClose",
	"code",
	"configuration",
	"dayEndTime",
	"dayStartTime",
	"ecr",
	"floor",
	"font",
	"footer",
	"ftr",
	"hdr",
	"header",
	"line",
	"location",
	"logo",
	"maintenance",
	"maxAmt",
	"maxOpr",
	"maxTrnAmt",
	"mid",
	"name",
	"operators",
	"prefixes", 
	"pwdAttempts",
	"pwdTimeout",
	"role"
};

static int cmpIdx;

static int getValIdx(const char *val) {
	int min, mid, max;
	int cmp;

	min = 1;  // First list item is "" to keep synchronized with eId enum
	max = sizeof(strTab) / sizeof(char *) - 1;
	cmpIdx = 0;

	// Perform dichotomy access
	while (min <= max)
	{
		cmpIdx++;
		mid = (max + min) / 2;

		cmp = strcmp(val, strTab[mid]);

		if (cmp == 0)
		{	// val found at mid
			return mid;
		}

		// val is not at mid

		if (cmp > 0) // next search in upper half
		{
			min = mid + 1;
		}
		else  // next search in lower half
		{
			max = mid - 1;
		}
	}

	// Not found
	return 0;
}


void tcjd0046(void)
{
	static byte waitStr[40];
	int ret;
	byte buf[128];
	byte tmp[128];
	int idx, oldMode;
	card crd, tcp1, tcp2, tcp3, tcp4;

	tcDspTitle("TCJD46", "tclib demo");
	oldMode = tcDspModeSet(SCR_BY_SCR);

	///////////////////////////////////////////////////

	crd = 10;
	ret = dec2num(&crd, " ", 0);
	tcDspF2N("ret = %d, crd = %d", ret, crd);  // got "ret = 1, crd = 0"
	ret = dec2num(&crd, "2A", 0);            
	tcDspF2N("ret = %d, crd = %d", ret, crd);  // got "ret = 1, crd = 2"
	tcWait("Hit a key", 0);
	return;

	///////////////////////////////////////////////////

	tcDspFN("Max sale = %ld", 0xFFFFFFFF/100);

	// Test floating point implementation 
	buf[0] = buf[1] = buf[2] = buf[3] = buf[4] = buf[5] = 0;
	buf[6] = 'Y';
	buf[7] = '@';

	msg2double(tmp, sizeof(tmp), buf, 0);
	tcDspFS("Int part of double = %s", tmp);
//	return;

	///////////////////////////////////////////////////

	for (idx = 1 ; idx < sizeof(strTab) / sizeof(char *) ; idx++)
	{
		char buf[50];

		sprintf(buf, "%s at %d, in %d steps", strTab[idx], getValIdx(strTab[idx]), cmpIdx);
		tcDspS(buf);
		// expected found "...at 1, 2, 3, 4, ... 26, 27"
	}

	// expected "...at 0..." : not found
	tcDspF2N("ZORRO at %d in %d steps", getValIdx("ZORRO"), cmpIdx);
	tcDspF2N("zorro at %d in %d steps", getValIdx("zorro"), cmpIdx);
	tcDspF2N("hhhh at %d in %d steps", getValIdx("hhhh"), cmpIdx);
	tcDspF2N("mmmm at %d in %d steps", getValIdx("mmmm"), cmpIdx);

	tcDspModeSet(oldMode);
//	return;
	
	///////////////////////////////////////////////////

	buf[0] = '0'; buf[1] = '1';
	for (idx = 2 ; idx <= 8  ; idx *= 2) {
		memcpy(buf + idx, buf, idx);
	}

	tcDspS(buf);  // expected "0101010101010101"

	///////////////////////////////////////////////////

	tmp[0] = '0'; tmp[1] = '1';
	sprintf(buf, "%2.2s%2.2s%2.2s%2.2s%2.2s%2.2s%2.2s%2.2s",
			tmp, tmp, tmp, tmp, tmp, tmp, tmp, tmp);
	tcDspS(buf);  // expected "0101010101010101"
	
	///////////////////////////////////////////////////

	sprintf(buf, "%0*d", 4, 0);
	tcDspS(buf);  // expected "0000"
	
	///////////////////////////////////////////////////

	tcWait("hit a key", 0);
	
	num2hex(tmp, 10, 3);
	sprintf(buf, "num2hex : %s", tmp);
	tcDspS(buf);
	sprintf(buf, "sprintf : %3.3X", 10);
	tcDspS(buf);

	{
		byte ini[] = "012345678901";
		sprintf(buf, "%3.3s.%3.3s.%3.3s.%3.3s|", &ini[0], &ini[3], &ini[6], &ini[9]);
		tcDspS(buf);  // expected "012.345.678.901|"
	
		dec2num(&tcp1, ini + 0, 3);
		dec2num(&tcp2, ini + 3, 3);
		dec2num(&tcp3, ini + 6, 3);
		dec2num(&tcp4, ini + 9, 3);
		sprintf(buf, "%d.%d.%d.%d|", tcp1, tcp2, tcp3, tcp4); 
		tcDspS(buf);  // expected "12.345.678.901|"
	}
	
	tcWait("hit a key", 0);

	for (idx = 0 ; idx < 128 ; idx++)  // 0 1 1 1 1 A A 2 2 2 2
	{		
		if (idx < 1)
			buf[idx] = 0;  // To test repeated byte display
		else if (idx < 5)
			buf[idx] = 1;  // To test repeated byte display
		else if (idx < 7)
			buf[idx] = 'A';  // To test repeated byte display
		else if (idx < 11)
			buf[idx] = 2;  // To test repeated byte display
		else
			buf[idx] = idx;
	}

	tcDspS("Display 128 bytes :");
	tcDspAN((byte *) buf, 128);
	tcWait("hit a key", 0);

	// Display the same msg (x3)	
	// ------------------------
	tcDspS("Connecting...");
	tmrPause(1);
	trcFS("Connecting%s", "...");
	tmrPause(1);
	trcS("Connecting...");
	tmrPause(1);
	// Overwrite the last message
	tcDspPS("Connected !");

	tcDspS("Long display spread on more than one line");

	// Display the same msg (xN)
	tcDspFN("display line %d", 5);
	// Redirect trace to screen
	trcFN("display line %d", 5);
	tcDspS("display line 5");

	// Force line feed displaying the last message
	tcDspS(0);

	tcDspS("display line F");
	tcDspFS("display line %s\n", "F");

	if (tcLoop() == 1)  // Get a string, only at loop 1
	{
		strcpy(waitStr, "12345");
		tcWaitS("Get a string", waitStr, sizeof(waitStr));
		tcDspS("Got string : ");
		sprintf(buf, "\"%s\"", waitStr);
		tcDspS(buf);
		tcWait("hit a key", 0);
	}
/*	
	for (idx = 0 ; idx < 128 ; idx++)
	{
		if (idx < 4)
			buf[idx] = 0;  // To test repeated byte display
		else if (idx < 8)
			buf[idx] = 1;  // To test repeated byte display
		else if (idx < 12)
			buf[idx] = 'A';  // To test repeated byte display
		else if (idx < 16)
			buf[idx] = 2;  // To test repeated byte display
		else
			buf[idx] = idx;
	}

	tcDspS("Display 128 bytes :");
	tcDspAN((byte *) buf, 128);
*/	
	tcSetRet(-2);  // Error code : tclib display an error msg and stop
}
