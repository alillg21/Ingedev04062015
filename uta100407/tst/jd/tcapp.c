#include "tc.h"

const char *tcIdleMsg(void)
{
	return "UTA 2.3\ntest cases\nJean-Philippe DUBOIS";
}

const char *tcIdleApp(void)
{
	return "tcapp_jp";
}

// Program here your test sequence
// (see tc.h for structure details)
//
mnuCfg_t *tcMenu(void)
{
	static mnuCfg_t tcEcrMnu1[] =
	{
		{ "TC_ECR 1 - SELECT [1-8]" },
		{ "TC LoadArticle",     '1', tcjd0001 },
		{ "TC OnlineSale",      '2', tcjd0002 },
		{ "TC CloseSale",       '3', tcjd0003 },
		{ "TC ProgramTax",      '4', tcjd0004 },
		{ "TC LoadList",        '5', tcjd0005 },
		{ "TC OnLineSale loop", '6', tcjd0006 },
		{ "TC HandShake",       '7', tcjd0007 },
		{ "TC OnLineVoid",      '8', tcjd0008 },
		{ "" }
	};

	static mnuCfg_t tcEcrMnu2[] =
	{
		{ "TC_ECR 2 - SELECT [1-2]" },
		{ "TC Sales report",    '1', tcjd0010 },
		{ "TC Sales from ECR",  '2', tcjd0011 },
		{ "TC X and Z reports", '3', tcjd0012 },
		{ "" }
	};

/*************
	// Sequence of 2 gprs tests, 50 iterations each
	static tcCfg_t tcGprsAuto50[] =
	{
		{EOT_WAIT,  2, tcjd0177, 50, {"1", "1"}}, // bml handshake
		{NO_WAIT,   2, tcjd0040, 50},  // Send GPRS msg using chn
//		{NO_WAIT,   2, tcjd0183, 1, {"tcjd0183", "BAT. LEVEL"}},
		{ 0 }
	};

	// Sequence of 2 gprs tests, 1 iteration each
	static tcCfg_t tcGprsAuto1[] =
	{
		{EOT_WAIT,  2, tcjd0177, 1, {"1111", "", "2"}}, // ctl = 2 : bml handshake
		{NO_WAIT,   2, tcjd0040 },  // Send GPRS msg using chn
		{ 0 }
	};
************/

	static mnuCfg_t tcGprsMnu[] =
	{
		{ "TC_GPRS MENU : TCJD177 uses GMA plugin" },
		{ "TCJD177 (conn, discon.)",    '1', tcjd0177, 1, {"0", "0"}},
		{ "TCJD177 (receive)",          '2', tcjd0177, 1, {"0", "1"}},
		{ "TCJD177 (send, receive)",    '3', tcjd0177, 1, {"1", "1"}},
//		{ "TCIK99  (with GMA)",         '2', tcik0099 },
//		{ "TCIK041 (without GMA)",      '4', tcik0041 },
		{ "TCJD177 (send,receive) x50", '4', tcjd0177, 50, {"1", "1"}},
//		{ "TCJD041 (without GMA)  x50", '6', tcjd0041, 50, {"1", "1"}},
//		{ "TCIK041 (without GMA)  x50", '7', tcik0041, 50, {"1", "1"}},
		{ "TCJD041 (without GMA)",      '5', tcjd0041 },
		{ "TCJD040 (without GMA)",      '6', tcjd0040 },
//		{ "TCJD040 (with CHN) x50",     '9', tcjd0040, 50 },
//		{ "LST(TCJD177(x50), 040(x50))",'?', tcGprsAuto50 },
//		{ "LST(TCJD(177, 040)x50)",     '?', tcGprsAuto1, 50 },  // 50 iterations
		{ "" }
	};

/*****
	static tcCfg_t tcAutoDemo[] =
	{
		{NO_WAIT,   2, tcjd0177, 1, {"1", "1"}},
		{NO_WAIT,   2, tcjd0183, 1, {"tcjd0183", "BAT. LEVEL"}},
		{ITER_WAIT, 2, tcjd0046, 2 }, // (for type1 tc, title is set
											   // internally by tcDspTitle())
		{ITER_WAIT, 2, tcab0009, 1, {"tcab0009"}},
		{NO_WAIT,   2, tcjd0007 },
		{ 0 }
	};

	static tcCfg_t tcBcrTst[] =
	{
		{NO_WAIT, 2, tcjd0025, 1},
		{NO_WAIT, 2, tcjd0026, 1},
		{ 0 }
	};
******/
	
	static mnuCfg_t tcDemoMnu[] =
	{
		{ "TC_DEMO MENU :" },
		{ "TCLIB DEMO-46",             '1', tcjd0046 },
		{ "TC BARCODE PRINTING",       '2', tcjd0025 },
		{ "TC BARCODE READING",        '3', tcjd0026 },
//		{ "LST BCR: tcjs001, tcik108", '3', tcBcrTst },
		{ "" }
	};
/******
	static mnuCfg_t tcAbMnu[] =
	{
		{ "TCAB - 120-123, 143, 150 :" },
		{ "TCAB 120", '1', tcab0120 },
		{ "TCAB 121", '2', tcab0121 },
		{ "TCAB 122", '3', tcab0122 },
		{ "TCAB 123", '4', tcab0123 },
		{ "TCAB 143", '5', tcab0143 },
		{ "TCAB 150", '6', tcab0150 },
		{ "" }
	};
*****/
	
	static mnuCfg_t mainMnu[] =
	{
		{ "MNU - TCAPP :" },
	 	{ "MNU GPRS",  '1', tcGprsMnu },
		{ "MNU ECR 1", '2', tcEcrMnu1 },
		{ "MNU ECR 2", '3', tcEcrMnu2 },
//	 	{ "MNU TC_AB", '3', tcAbMnu   },
		{ "MNU DEMO",  '4', tcDemoMnu },
		{ "" }
	};

	return mainMnu;
}

