/** \file
 * Unitary test case tcmp0017.
 *
 * nvmHold and NvmRelease performance testing:
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcmp0017.c $
 *
 * $Id: tcmp0017.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#ifdef __TELIUM__
#include <sdk30.h>
#endif
#include <string.h>
#include "str.h"
#include "tst.h"

#ifdef __TEST__
#define CHK CHECK(ret>=0,lblKO)
static int iterations = 1000;

static card prtTS(const char *msg, card beg) {
    char tmp[prtW + 1];
    char buf[prtW + 1];
    card ts;

    strcpy(buf, msg);
    ts = getTS(1);
    ts -= beg;
    num2dec(tmp, ts, 0);
    strcat(buf, tmp);
    prtS(buf);
    return ts + beg;
}

//Make N writings of the same byte to the same place, without nvmHold/nvmRelease.
static int test1(void) {
	int ret;
	int idx;
	card ts;	

	ts = prtTS("test1 beg ", 0);
	
    for(idx = 0; idx < iterations; idx++){ 
        ret = nvmSave(0, "1", 0, 1);  CHK;
    }

	prtTS("test1 end ", 0);	
	prtTS("test1: ", ts);			
	//Unicapt Results: 1000 insertions per 6450 centiseconds: 15.50 insertions per sec
    //Telium Results: 1000 insertions per 148 centiseconds: 675.68 insertions per sec
	return ret;
	lblKO:
		return -1;	
}

//Make N writings of the same byte to the same place, with nvmHold/nvmRelease.
static int test2(void) {
	int ret;
	int idx;
	card ts;

	ts = prtTS("test2 beg ", 0);
	
    ret= nvmHold(0);
    for(idx = 0; idx < iterations; idx++){ 
		ret = nvmSave(0, "1", 0, 1);  CHK;
    }
    nvmRelease(0);

	prtTS("test2 end ", 0);	
	prtTS("test2: ", ts);		
	//Unicapt Results: 1000 insertions per 4983 centiseconds: 20.07 insertions per sec
    //Telium Results: 1000 insertions per 125 centiseconds: 800 insertions per sec
	return ret;
	lblKO:
		return -1;
}

//Make N readings of the same byte to the same place, without nvmHold/nvmRelease.
static int test3(void) {
	int ret;
	int idx;
	card ts;	
	char buf[256];

	ts = prtTS("test3 beg ", 0);
	
    for(idx = 0; idx < iterations; idx++){ 
        memset(buf, 0, 256);
        ret = nvmLoad(0, buf, 0, 1); CHK;
    }

	prtTS("test3 end ", 0);	
	prtTS("test3: ", ts);				
	//Unicapt Results: 1000 readings per 2431 centiseconds: 41.14 readings per sec
    //Telium Results: 1000 readings per 68 centiseconds: 1470.59 readings per sec	
	return ret;
	lblKO:
		return -1;	
}

//Make N readings of the same byte to the same place, with nvmHold/nvmRelease.
static int test4(void) {
	int ret;
	int idx;
	card ts;	
	char buf[256];

	ts = prtTS("test4 beg ", 0);
	
    ret= nvmHold(0);
    for(idx = 0; idx < iterations; idx++){
        memset(buf, 0, 256);			
        ret = nvmLoad(0, buf, 0, 1); CHK;
    }
    nvmRelease(0);

	prtTS("test4 end ", 0);	
	prtTS("test4: ", ts);		
	//Unicapt Results: 1000 readings per 555 centiseconds: 180.18 readings per sec
    //Telium Results: 1000 readings per 47 centiseconds: 2127.66 readings per sec
	return ret;
	lblKO:
		return -1;	
}

//Write N bytes to flash sequentially one by one using nvmSeqPut function
static int test5(void) {
    int ret;
    word idx;
    card hdl = 0;
	card ts;			

    hdl = nvmSeqOpen("tcmp0017.tst", 'w');
    CHECK(hdl, lblKO);

	ts = prtTS("test5 beg ", 0);
	
    for (idx = 0; idx < iterations; idx++) {
        ret = nvmSeqPut(hdl, "1", 1);
        CHECK(ret >= 0, lblKO);
    }

	prtTS("test5 end ", 0);	
	prtTS("test5: ", ts);			
	//Unicapt Results: 1000 insertions per 4897 centiseconds: 20.42 insertions per sec
    //Telium Results: 1000 insertions per 37 centiseconds: 2702.70 insertions per sec	
    nvmSeqClose(hdl);
    tmrPause(1);

 	goto lblEnd;
	
  lblKO:
	ret = -1;
  lblEnd:
    if(hdl)
        nvmSeqClose(hdl);	
	return ret;
}

//Read N bytes to flash sequentially one by one using nvmSeqGet function
static int test6(void) {
	int ret;
	word idx;
	char buf[256];
	card hdl = 0;
	card ts;			

    hdl = nvmSeqOpen("tcmp0017.tst", 'r');
    CHECK(hdl, lblKO);

	ts = prtTS("test6 beg ", 0);	
	
    for (idx = 0; idx < iterations; idx++) {
        memset(buf, 0, 256);
        ret = nvmSeqGet(hdl, buf, 1);
        CHECK(ret >= 0, lblKO);
    }		

	prtTS("test6 end ", 0);	
	prtTS("test6: ", ts);			
	//Unicapt Results: 1000 readings per 495 centiseconds: 202.02 readings per sec
    //Telium Results: 1000 readings per 41 centiseconds: 2439.02 readings per sec		
    nvmSeqClose(hdl);
    tmrPause(1);

 	goto lblEnd;
	
  lblKO:
	ret = -1;
  lblEnd:
    if(hdl)
        nvmSeqClose(hdl);	
	return ret;
}

//Write these bytes sequentially using nvmSave without nvmHold/nvmRelease
static int test7(void) {
	int ret;
	word idx;
	card ts;			

	ts = prtTS("test7 beg ", 0);	

    for (idx = 0; idx < iterations; idx++) {               
		 ret = nvmSave(0, "1", idx, 1);
        CHECK(ret >= 0, lblKO);
    }		

	prtTS("test7 end ", 0);	
	prtTS("test7: ", ts);			
	//Unicapt Results: 1000 insertions per 6807 centiseconds: 14.69 insertions per sec
    //Telium Results: 1000 insertions per 147 centiseconds: 680.27 insertions per sec	
 	goto lblEnd;
	
  lblKO:
	ret = -1;
  lblEnd:
	return ret;
}

//Write these bytes sequentially using nvmSave with nvmHold/nvmRelease
static int test8(void) {
	int ret;
	word idx;
	card ts;			

	ts = prtTS("test8 beg ", 0);	

	nvmHold(0);
    for (idx = 0; idx < iterations; idx++) {               
		 ret = nvmSave(0, "1", idx, 1);
        CHECK(ret >= 0, lblKO);
    }		
	nvmRelease(0);
	
	prtTS("test8 end ", 0);	
	prtTS("test8: ", ts);			
	//Unicapt Results: 1000 insertions per 5226 centiseconds: 19.14 insertions per sec
    //Telium Results: 1000 insertions per 126 centiseconds: 2083.33 insertions per sec	
 	goto lblEnd;
	
  lblKO:
	ret = -1;
  lblEnd:
	return ret;
}

//Read these bytes sequentially using nvmLoad without nvmHold/nvmRelease
static int test9(void) {
	int ret;
	word idx;
	char buf[256];
	card ts;			


	ts = prtTS("test9 beg ", 0);	
	
    for (idx = 0; idx < iterations; idx++) {
        memset(buf, 0, 256);
        ret = nvmLoad(0, buf, idx, 1);
        CHECK(ret >= 0, lblKO);
    }		

	prtTS("test9 end ", 0);	
	prtTS("test9: ", ts);			
	//Unicapt Results: 1000 readings per 2434 centiseconds: 41.08 readings per sec
    //Telium Results: 1000 readings per 68 centiseconds: 1470.59 readings per sec	
 	goto lblEnd;
	
  lblKO:
	ret = -1;
  lblEnd:
	return ret;
}

//Read these bytes sequentially using nvmLoad with nvmHold/nvmRelease
static int test10(void) {
	int ret;
	word idx;
	char buf[256];
	card ts;			

	ts = prtTS("test10 beg ", 0);	

	nvmHold(0);
    for (idx = 0; idx < iterations; idx++) {
        memset(buf, 0, 256);
        ret = nvmLoad(0, buf, idx, 1);
        CHECK(ret >= 0, lblKO);
    }		
	nvmRelease(0);

	prtTS("test10 end ", 0);	
	prtTS("test10: ", ts);			
	//Unicapt Results: 1000 readings per 558 centiseconds: 179.21 readings per sec
    //Telium Results: 1000 readings per 48 centiseconds: 2083.33 readings per sec	
 	goto lblEnd;
	
  lblKO:
	ret = -1;
  lblEnd:
	return ret;
}

//Read these bytes randomly using nvmLoad without nvmHold/nvmRelease
static int test11(void) {
	int ret;
	word idx;
	char buf[256];
	card ts;			
	card loc;

	ts = prtTS("test11 beg ", 0);	

    for (idx = 0; idx < iterations; idx++) {
        memset(buf, 0, 256);
    	 loc = sysRand() % iterations;
        ret = nvmLoad(0, buf, loc, 1);
        CHECK(ret >= 0, lblKO);
    }		

	prtTS("test11 end ", 0);	
	prtTS("test11: ", ts);			
	//Unicapt Results: 1000 readings per 2580 centiseconds: 38.76 readings per sec
    //Telium Results: 1000 readings per 70 centiseconds: 1428.57 readings per sec	
 	goto lblEnd;
	
  lblKO:
	ret = -1;
  lblEnd:
	return ret;
}

//Read these bytes randomly using nvmLoad with nvmHold/nvmRelease
static int test12(void) {
	int ret;
	word idx;
	char buf[256];
	card ts;			
	card loc;

	ts = prtTS("test11 beg ", 0);	

	nvmHold(0);	
    for (idx = 0; idx < iterations; idx++) {
        memset(buf, 0, 256);
    	 loc = sysRand() % iterations;
        ret = nvmLoad(0, buf, loc, 1);
        CHECK(ret >= 0, lblKO);
    }		
	nvmRelease(0);

	prtTS("test11 end ", 0);	
	prtTS("test11: ", ts);			
	//Unicapt Results: 1000 readings per 711 centiseconds: 140.65 readings per sec
    //Telium Results: 1000 readings per 48 centiseconds: 2083.33 readings per sec	
 	goto lblEnd;
	
  lblKO:
	ret = -1;
  lblEnd:
	return ret;
}
void tcmp0017(void) {
	int ret;

	ret = dspStart();
	CHECK(ret >= 0, lblKO);	

	ret = prtStart();
	CHECK(ret >= 0, lblKO);

    nvmStart();                 //initialise non-volatile memory
    
	dspClear();
	dspLS(1, "Write w/o");
	dspLS(2, "NvmHold");
	prtS("Write w/o Nvm Hold:");
	ret = test1(); CHK;	

	dspClear();
	dspLS(1, "Write w/");
	dspLS(2, "NvmHold");
	prtS("Write with Nvm Hold:");
	ret = test2(); CHK;	

	dspClear();
	dspLS(1, "Read w/o");
	dspLS(2, "NvmHold");
	prtS("Read w/o Nvm Hold:");
	ret = test3(); CHK;	

	dspClear();
	dspLS(1, "Read w/");
	dspLS(2, "NvmHold");
	prtS("Read with Nvm Hold:");
	ret = test4();CHK;

	dspClear();
	dspLS(1, "Write Seq");
	dspLS(2, "using nvmSeqPut");
	prtS("Write Seq (nvmSeqPut):");
	ret = test5(); CHK;

	dspClear();
	dspLS(1, "Read Seq w/");
	dspLS(2, "using nvmSeqGet");
	prtS("Read Seq (nvmSeqGet):");
	ret = test6(); CHK;	

	dspClear();
	dspLS(1, "Write Seq w/o");
	dspLS(2, "NvmHold");
	prtS("Write Seq w/o Nvm Hold:");
	ret = test7(); CHK;

	dspClear();
	dspLS(1, "Write Seq w/");
	dspLS(2, "NvmHold");
	prtS("Write Seq with Nvm Hold:");
	ret = test8(); CHK;	

	dspClear();
	dspLS(1, "Read Seq w/o");
	dspLS(2, "NvmHold");
	prtS("Read Seq w/o Nvm Hold:");
	ret = test9(); CHK;

	dspClear();
	dspLS(1, "Read Seq w/");
	dspLS(2, "NvmHold");
	prtS("Read Seq with Nvm Hold:");
	ret = test10(); CHK;		

	dspClear();
	dspLS(1, "Read Random w/o");
	dspLS(2, "NvmHold");
	prtS("Read Rdm w/o Nvm Hold:");
	ret = test11(); CHK;		

	dspClear();
	dspLS(1, "Read Random w/");
	dspLS(2, "NvmHold");
	prtS("Read Rdm with Nvm Hold:");
	ret = test12(); CHK;			
	
	goto lblEnd;
  lblKO:
    dspClear();
    dspLS(0, "KO!");
    trcErr(ret);
    tmrPause(3);
  lblEnd:
		ret = 1;
    dspStop();
    prtStop();
}
#endif

