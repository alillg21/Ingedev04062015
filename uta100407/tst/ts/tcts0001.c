#include <string.h>
#include "pri.h"
#include "tst.h"

//Integration test case tcst0001.
//Testing combo.

static int testSpr(void){
    int ret;
    tTable tab1;
    tTable tab2;
    tContainer cnt1;
    tContainer cnt2;
    tCombo cmb;
    tLocator loc[3];
    char buf[prtW+1];
	word total;
    char volDba[prtW+sizeof(total)];
	word ofs;

    //Initialize pointer to volatile memory buffer
    memset(volDba,0,sizeof(volDba));
    dbaSetPtr(volDba);

    //Initialize a table at the beginning of volatile memory section 0xFF
	ofs=0;
	//row width is prtW, first row (control string)
    ret= tabInit(&tab1,0xFF,0,prtW,1);
    VERIFY(ret==(int)tabSize(&tab1));

	ofs+=ret;
	//input parameter is binary (word)
    ret= tabInit(&tab2,0xFF,ofs,sizeof(total),1);
    VERIFY(ret==(int)tabSize(&tab2));

    //Create a container around the tables
    ret= cntInit(&cnt1,'t',&tab1); CHECK(ret>0,lblKO); 
    ret= cntInit(&cnt2,'t',&tab2); CHECK(ret>0,lblKO); 

    //Initialize the locator pointing to the operation arguments
    loc[0].cnt= &cnt1;
    loc[0].key= 0; //The first row of the table will be interpreted as format argument of sprintf
    loc[1].cnt= &cnt2;
    loc[1].key= 0; //The second row is the val argument
    loc[2].cnt= 0; //The sup argument is missing
    loc[2].key= 0;

    //Create a combo for sprintf operation
    ret= cmbInit(&cmb,oprSpr,loc,prtW+1); CHECK(ret>0,lblKO); 

    //Fill the tables
    ret= tabPutStr(&tab1,0,"Debit number: %d"); //format argument
    CHECK(ret>0,lblKO);
	total=23;
    ret= tabPutStr(&tab2,0,&total); //val argument
    CHECK(ret>0,lblKO); 

    //Now test it
    ret= cmbGet(&cmb,0,buf,prtW);
    CHECK(ret>0,lblKO); 

    //Show the result
    prtS(buf);
    CHECK(strcmp(buf,"Debit number: 23")==0,lblKO); 

    ret= 1;
    goto lblEnd;
lblKO:
    ret= -1;
lblEnd:
    tmrPause(2);
    return ret;
}
void tcts0001(void){
    int ret;

    nvmStart(); //initialise non-volatile memory
    ret= dspStart(); CHECK(ret>=0,lblKO);
    ret= prtStart(); CHECK(ret>=0,lblKO);
      
    ret= dspClear(); CHECK(ret>=0,lblKO);    
    ret= dspLS(0,"Combo Spr(%d)..."); CHECK(ret>=0,lblKO);    
    ret= testSpr(); CHECK(ret>=0,lblKO);
    tmrPause(1);

    goto lblEnd;
    
lblKO:
    dspLS(0,"KO");
lblEnd:
    tmrPause(3);
    dspStop(); prtStop(); //close resource
}
