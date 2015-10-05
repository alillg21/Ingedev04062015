#include <string.h>
#include "pri.h"
#include "tc.h"

// Unitary test case tcjd0010
// Tested function :
//   ecrRcvSalesReport

typedef struct
{
	word num;
	char bar[15];
	char name[15];
	byte tax;
	char price[10];
	char salesAmt[20];
	char salesNum[10];
	char stock[10];	
} pluRecord;


void tcjd0010(void) {
	int ret, cnt, idx;
	int orgMode;
	char *fnc = "RcvSales";
	pluRecord plu;
	tBuffer sales;
	byte salesB[6000];  // (1 full packet is 1448 bytes long and contains 31 plu records)
	
	orgMode = tcDspModeSet(-SCR_BY_SCR);
	tcDspModeSet(TRC_2_HMI);
//	tcDspModeSet(MSG_2_PRT);
	tcDspS(fnc);
	tcDspTitle(fnc, "");

	ret = ecrStart();
	CHECK(ret == 1, lblKO);

	bufInit(&sales, salesB, sizeof(salesB));

	cnt = ecrRcvSales(&sales);
	tcDspFN("ecrRcvSales = %d", cnt);
	CHECK(cnt >= 0, lblKO);
	tcWait("Parse sales", 0);

	for (idx = 0 ; idx < cnt ; idx++)
	{
		card price, sNum, amt, sAmt;
		
		ret = ecrParseSales(&plu, &sales, idx);
		CHECK(ret == 1, lblKO);
		tcDspInit(0);
		tcDspFN(">> pluTab[%d] :", idx);
		tcDspFN(" num = %d", plu.num);
		tcDspFS(" bar = %s", plu.bar);
		tcDspFS(" name = %s", plu.name);
		tcDspFN(" tax = %d", plu.tax);
		tcDspFS(" price = %s", plu.price);
		tcDspFS(" salesNum = %s", plu.salesNum);
		tcDspFS(" salesAmt = %s", plu.salesAmt);
//		tcDspFS(" stock = %s", plu.stock);

		price = atoi(plu.price);
		sNum = atoi(plu.salesNum);
		amt = price * sNum;
		sAmt = atoi(plu.salesAmt);
		
		if (sAmt != amt)
		{
			tcDspFN("calculated Amt = %d", amt);
			tcWait("Incoherent data", 0);  // Wait for kbd entry for the next plu
		}
	}

	tcDspS(" OK");
	goto lblEnd;

lblKO:
	prtS(fnc);
	tcDspFN(" KO : ACK = %d", ecrGetAck()); tcPrt((char *) 0);

lblEnd:
	tcDspModeReset(orgMode);
	ecrStop();
}


/***
	// Make sales from PC
	// ------------------
	ret = ecrOnLineSale(7, 700); // 7 = "064 elektronsk", 700 * 1 = 700
	CHECK(ret == 1, lblKO);

	ret = ecrOnLineSale(56, 2); // 56 = "Borske novine", 2 * 1000 = 2000
	CHECK(ret == 1, lblKO);

	ret = ecrOnLineSale(57, 10); // 57 = "Vesti", 10 * 20 = 200
	CHECK(ret == 1, lblKO);

	ret = ecrOnLineSale(7, 300); // 7 = "064 elektronsk", 300 * 1 = 300
	CHECK(ret == 1, lblKO);

	ret = ecrOnLineSale(35, 5); // 35 = "Narodne novine", 5 * 20 = 100
	CHECK(ret == 1, lblKO);

	ret = ecrOnLineSale(46, 4); // 46 = "Slobodna rec", 4 * 25 = 100
	CHECK(ret == 1, lblKO);

	ret = ecrOnLineSale(96, 20); // 96 = "Nalog uplata", 20 * 5 = 100
	CHECK(ret == 1, lblKO);

	ret = ecrOnLineSale(155, 2); // 155 = "Karton vozaca", 2 * 300 = 600
	CHECK(ret == 1, lblKO);

	ret = ecrOnLineSale(156, 2); // 156 = "Dosije voz.f.", 2 * 1500 = 3000
	CHECK(ret == 1, lblKO);

	ret = ecrOnLineSale(157, 2); // 157 = "Poreska knjiz.", 2 * 4000 = 8000
	CHECK(ret == 1, lblKO);

	ret = ecrOnLineSale(158, 2); // 158 = "Sanit.knjiz.", 2 * 4000 = 8000
	CHECK(ret == 1, lblKO);

	ret = ecrOnLineSale(159, 2); // 15 = "", 2 * 00 = 00
	CHECK(ret == 1, lblKO);

	ret = ecrOnLineSale(160, 2); // 15 = "", 2 * 00 = 00
	CHECK(ret == 1, lblKO);

	ret = ecrOnLineSale(161, 2); // 15 = "", 2 * 00 = 00
	CHECK(ret == 1, lblKO);

	ret = ecrOnLineSale(162, 2); // 15 = "", 2 * 00 = 00
	CHECK(ret == 1, lblKO);

	ret = ecrOnLineSale(163, 2); // 15 = "", 2 * 00 = 00
	CHECK(ret == 1, lblKO);

	ret = ecrOnLineSale(164, 2); // 15 = "", 2 * 00 = 00
	CHECK(ret == 1, lblKO);

// 33
	ret = ecrOnLineSale(165, 2); // 15 = "", 2 * 00 = 00
	CHECK(ret == 1, lblKO);

	ret = ecrOnLineSale(166, 2); // 15 = "", 2 * 00 = 00
	CHECK(ret == 1, lblKO);

	ret = ecrOnLineSale(167, 2); // 15 = "", 2 * 00 = 00
	CHECK(ret == 1, lblKO);

	ret = ecrOnLineSale(168, 2); // 15 = "", 2 * 00 = 00
	CHECK(ret == 1, lblKO);

	ret = ecrOnLineSale(169, 2); // 15 = "", 2 * 00 = 00
	CHECK(ret == 1, lblKO);

	ret = ecrOnLineSale(170, 2); // 15 = "", 2 * 00 = 00
	CHECK(ret == 1, lblKO);

	ret = ecrOnLineSale(171, 2); // 15 = "", 2 * 00 = 00
	CHECK(ret == 1, lblKO);

	ret = ecrOnLineSale(172, 2); // 15 = "", 2 * 00 = 00
	CHECK(ret == 1, lblKO);

	ret = ecrOnLineSale(173, 2); // 15 = "", 2 * 00 = 00
	CHECK(ret == 1, lblKO);

	ret = ecrOnLineSale(174, 2); // 15 = "", 2 * 00 = 00
	CHECK(ret == 1, lblKO);

	ret = ecrOnLineSale(175, 2); // 15 = "", 2 * 00 = 00
	CHECK(ret == 1, lblKO);

	ret = ecrOnLineSale(176, 2); // 15 = "", 2 * 00 = 00
	CHECK(ret == 1, lblKO);

	ret = ecrOnLineSale(177, 2); // 15 = "", 2 * 00 = 00
	CHECK(ret == 1, lblKO);

	ret = ecrOnLineSale(178, 2); // 15 = "", 2 * 00 = 00
	CHECK(ret == 1, lblKO);

	ret = ecrOnLineSale(179, 2); // 15 = "", 2 * 00 = 00
	CHECK(ret == 1, lblKO);

	ret = ecrFinishSale();
	CHECK(ret == 1, lblKO);
****/
