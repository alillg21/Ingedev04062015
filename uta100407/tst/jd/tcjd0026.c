/// \tcjd0026 file.c

/** UTA Unitary test case of barcode reader usage.
 * \sa
 *  - bcrStart()
 *  - bcrRead()
 *  - bcrStop()
 */

#include <unicapt.h>
#include <string.h>
#include <stdio.h>
#include "sys.h"

#define CHK CHECK(ret>=0,lblKO)

void tcjd0026(void) {	
#ifdef __BCR__
	int ret;
	char buf[256];
	char dateTime[12 + 1];
	char *ptr;

	memset(buf,'\0',256);
	nvmStart();
//	prtStart();
//	dspStart();
//	cnvStart();
	tcDspS("Start TCJD0026"); //tcPrt(0);

	ret = getDateTime(dateTime);
	//tcPrt(dateTime);
	
	ret = 0;
		
	do
	{
		switch (ret) {
			case 0:
				tcDspS("0: Read normal BC...");
				ptr = NULL;
				break;
			case 1:
				tcDspS("1: Read  code 39...");
				ptr = "PRECA2;PREBK201;C39ENA1.";
				break;
			case 2:
				tcDspS("2: Read PDF417...");
				ptr = "PRECA2;PREBK202;PDFDFT;MPDDFT.";
				break;
			case 3:
				tcDspS("3: Read CBRENA1...");
				ptr = "PRECA2;PREBK201;CBRENA1.";
				break;
			case 4:
				tcDspS("4: Read Code 49...");
				ptr = "PRECA2;PREBK201;C49DFT.";
				break;
			case 5:
				tcDspS("5: Read Aztec...");
				ptr = "PRECA2;PREBK201;AZTDFT.";
				break;
			case 6:
				tcDspS("6: Read QR Code...");
				ptr = "PRECA2;PREBK201;QRCENA1.";
				break;
			default:
				tcDspS("Done !");
				goto lblEnd;
		}	
		
		ret = bcrStart();
		
		if (ret != RET_OK)
		{
			tcDspS("BCR start error"); //tcPrt(0);
			goto lblEnd;
		}

		ret = bcrRead(ptr, 50, buf);

		if (ret == 0)
		{
			tcDspS("No bar code"); //tcPrt(0);
		}
		else if (ret > 0)
		{
			tcDspFS("Bar code = %s", buf); //tcPrt(0);
		}
		else
		{
			tcDspS("Bar code reading error"); //tcPrt(0);
		}

		bcrStop();

		tcDspS("-----------");

		ret = tcWait("Next decoding [0-6]\n", 0) - '0';
	} while (ret >= 0 && ret <= 6);

lblEnd:
	tmrPause(2);
//	bcrStop();
//	cnvStop();
//	dspStop();
	//tcPrt(" ");
	//tcPrt(" ");
	//tcPrt("____fin test____");
	//prtStop();
#endif
}
