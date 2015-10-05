#include <string.h>
#include "pri.h"
#include "tc.h"

// Unitary test case tcjd0012
// Tested functions :
//   ecrRcvXreport, ecrReqZreport

typedef struct
{
/* 0 */ int taxableVal;   //  141360  151071
	int ReceiptsNum;  //           2       3
	int netto;        //      123968  132274
	int taxes;        //       17391   18796
	int voidAmt;      //      -17890  -17900
	int voidQty;
	int subTotAmt;    //         401     401
	int subTotQty;
	int refundAmt;
	int refundQty;
/* 10 */ int RAcashAmt;
	int RAcashQty;
	int RAcashEuroAmt;
	int RAcashEuroQty;
	int POcashAmt;
	int POcashQty;
	int POcashEuroAmt;
	int POcashEuroQty;
	int RAchkAmt;
	int RAchkQty;
/* 20 */ int POchkAmt;
	int POchkQty;
	int discAmt;
	int discQty;
	int addOnAmt;
	int addOnQty;
	int roundDiff;
	int totSalCashAmt;      // 141360  151071
	int totSalCashQty;      //      2       2
	int totSalCashEuroAmt;
/* 30 */ int totSalCashEuroQty;
	int totSalChkAmt;
	int totSalChkQty;
	int totSalCredAmt;
	int totSalCredQty;
	int drawCash;           // 141360  151071
	int drawEuroCash;
	int drawChk;
	int drawCred;
	int totRecpt;           //      9       9
/* 40 */ int fiscRecpt;
} XReport_t;

void tcjd0012(void) {
	char *fnc = "RcvXreport, ReqZreport";
	int ret, idx;
	int orgMode;
	XReport_t xRep;
	int *pVal;
			
	orgMode = tcDspModeSet(-SCR_BY_SCR);
	tcDspModeSet(TRC_2_HMI);
//	tcDspModeSet(MSG_2_PRT);
	tcDspS(fnc);
	tcDspTitle(fnc, "");

	ret = ecrStart();
	CHECK(ret == 1, lblErrDecode);

	ret = ecrRcvXreport(&xRep);
	CHECK(ret == 1, lblKO);

	pVal = (int *) &xRep;
	for (idx = 0 ; idx < sizeof(xRep) / sizeof(card) ; idx++)
	{
		if (pVal[idx])
			tcDspF2N("val[%d] = %d", idx, pVal[idx]);
	}

	tcWait("RcvXreport OK", 0);
	
	ret = ecrReqZreport(0);
	CHECK(ret == 1, lblKO);
	tcDspS("ReqZreport OK");
	goto lblEnd;

lblErrDecode:
	tcDecodeComError(ret);
	tcSetRet(-1);
	
lblKO:
	tcDspFN(" KO! (NACK = %d)", ecrGetAck()); tcPrt(0);

lblEnd:
	ecrStop();
}
