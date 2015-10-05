#include <string.h>
#include "pri.h"
#include "tc.h"

// Unitary test case tcjd0011
// Tested function :
//   ecrRcvSale

typedef struct
{
	word ref;
	char name[15];
	byte tax;
	char amount[20];
	char price[10];
	char qty1[10];
	char qty2[10];
	char cash[20];
	char check[20];
	char credit[20];
	char cash2[20];
} tSale;

void tcjd0011(void) {
	char *fnc = "RcvSale";
	int ret, saleIdx;
	int orgMode;
	tSale sale;
		  
	orgMode = tcDspModeSet(-SCR_BY_SCR);
	tcDspModeSet(TRC_2_HMI);
//	tcDspModeSet(MSG_2_PRT);
	tcDspS(fnc);
	tcDspTitle(fnc, "");

	ret = ecrStart();
	CHECK(ret == 1, lblErrDecode);

//	ret = ecr2pcEnable(0);
//	CHECK(ret == 1, lblKO);

	ret = ecr2pcEnable(1);
	CHECK(ret == 1, lblKO);

	for (saleIdx = 1; 42 ; saleIdx++)
	{
		tcPushLn();
		tcDspS("Make a sale on the ECR... (or press Cancel)");
		tcDspS("");
		tcPopLn();
		
		ret = ecrRcvSale(&sale);

		if (ecrGetAck() == -7)  // Cancelled by user
			break;
		
		CHECK(ret == 1, lblKO);

		tcDspF2N("Sale %d for PLU ref = %d", saleIdx, sale.ref);
		tcDspFS("Name = %s", sale.name);
		tcDspFN("Tax = %d", sale.tax);
		tcDspFS("Total price = %s", sale.amount);
		tcDspFS("Unit price = %s", sale.price);
		tcDspFS("Quantity1 = %s", sale.qty1);
//		tcDspFS("Quantity2 = %s", sale.qty2);
//		tcDspFS("Cash = %s", sale.cash);
//		tcDspFS("Check = %s", sale.check);
//		tcDspFS("Credit = %s", sale.credit);
//		tcDspFS("Cash2 = %s", sale.cash2);
		tcDspS("-------------");
	}

	tcDspS(" OK");
	goto lblEnd;

lblErrDecode:
	tcDecodeComError(ret);
	tcSetRet(-1);
	
lblKO:
	tcDspFN(" KO! (NACK = %d)", ecrGetAck()); tcPrt(0);

lblEnd:
	ret = ecr2pcEnable(0);

	if (ret < 0)
	{
		tcWait("Finish the sale on ECR!", 0);
		ret = ecr2pcEnable(0);
	}
	
	if (ret < 0)
	{
		tcDspFN("ecr2pcEnable(0) KO! (NACK = %d)", ecrGetAck()); tcPrt(0);
		tcWait("", 0);
	}
	else
		tcDspFS("%s OK", fnc); tcPrt(0);
	
	ecrStop();
}
