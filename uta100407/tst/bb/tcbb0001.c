/** \file
 * Unitary test case for printer fonts
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/bb/tcbb0001.c $
 *
 * $Id: tcbb0001.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include "sdk30.h"
#include "sys.h"

void printWithEscapes(void){
	int ret;
	
	ret = prtES("\x1B\x20\x1B\x17","CONDENSED AND CENTERED");
	ret = prtES("\x1B\x20\x1B\x18","CONDENSED AND RIGHT");
	ret = prtES("\x1B\x17\x1B\x20","CENTERED AND CONDENSED");
	ret = prtES("\x1B\x18\x1B\x20","RIGHT AND CONDENSED");
	ret = prtES("\x1B\x20\x1B\x22","CONDENSED+REVERSED");
	ret = prtES("\x1B\x20\x1B\x18","CONDENSED+RIGHT");	
	ret = prtES("\x1B\x20\x1B\x17\x1B\x22","CONDENSED+CENTERED+REVERSED");	
	
	ret = prtES("","NORMAL");
	ret = prtES("\x1B\x17","NORM+CENTERED");
	ret = prtES("\x1B\x22","NORM+REVERSED");
	ret = prtES("\x1B\x18","NORMAL+RIGHT");	
	ret = prtES("\x1B\x17\x1B\x22","NORM+CENT+REV");	

	ret = prtES("\x1B\x1A\x1B\x17","BOLD AND CENTERED");
	ret = prtES("\x1B\x1A\x1B\x18","BOLD AND RIGHT");
	ret = prtES("\x1B\x17\x1B\x1A","CENTERED AND BOLD");
	ret = prtES("\x1B\x18\x1B\x1A","RIGHT AND BOLD");
	ret = prtES("\x1B\x1A\x1B\x18\x1B\x22","BOLD+RIGHT+REV");

	ret = prtES("\x1B\x1D\x1B\x17","HIGH AND CENTERED");
	ret = prtES("\x1B\x1D\x1B\x18","HIGH AND RIGHT");
	ret = prtES("\x1B\x17\x1B\x1D","CENTERED AND HIGH");
	ret = prtES("\x1B\x18\x1B\x1D","RIGHT AND HIGH");
	ret = prtES("\x1B\x1D\x1B\x18\x1B\x22","HIGH+RIGHT+REV");
	
	ret = prtES("\x1B\x2A","MEDIUM");	
	ret = prtES("\x1B\x2A\x1B\x17","MED+CENTERED");
	ret = prtES("\x1B\x2A\x1B\x18","MED+RIGHT");
	ret = prtES("\x1B\x17\x1B\x2A","CENTERED+MED");
	ret = prtES("\x1B\x18\x1B\x2A","RIGHT+MED");
	ret = prtES("\x1B\x2A\x1B\x18\x1B\x22","MED+RIGHT+REV");
	
	ret = prtES("\x1B\x1E","BIG");	
	ret = prtES("\x1B\x1E\x1B\x17","BIG+CENT");
	ret = prtES("\x1B\x17\x1B\x1E","CENT+BIG");
	ret = prtES("\x1B\x18\x1B\x1E","RIGHT+BIG");
	ret = prtES("\x1B\x1E\x1B\x18\x1B\x22","B+R+R");
	
 	prtS(" "); 	prtS(" "); prtS(" ");	
	return;
}

void printSome(void){
	int ret;
	
	ret = prtES("\x1B\x20","!#$%&ﬂ()*+,-/0123456789:<=>ABCDEFGHIJKLMNOPQRSTU");
	ret = prtES("\x1B\x20","VWXYZ[]_3¥^;abcdefghijklmnopqrstuvwxyzÄ§…¡Õ”ˆ¸ı˚");
	
	ret = prtES("","!#$%&ﬂ()*+,-/0123456789:");
	ret = prtES("","<=>ABCDEFGHIJKLMNOPQRSTU");
	ret = prtES("","abcdefghijklmnÄ§…¡Õ”ˆ¸ı˚");

	ret = prtES("\x1B\x1A","!#$%&ﬂ()*+,-/0123456789:");
	ret = prtES("\x1B\x1A","<=>ABCDEFGHIJKLMNOPQRSTU");
	ret = prtES("\x1B\x1A","abcdefghijklmnÄ§…¡Õ”ˆ¸ı˚");

	ret = prtES("\x1B\x1D","!#$%&ﬂ()*+,-/0123456789:");
	prtES("\x1B\x1D","<=>ABCDEFGHIJKLMNOPQRSTU");
	prtES("\x1B\x1D","abcdefghijklmnÄ§…¡Õ”ˆ¸ı˚");
	
	prtES("\x1B\x2A","!#$%&ﬂ()*+,-/012");
	prtES("\x1B\x2A","3456789:<=>ABCDE");
	prtES("\x1B\x2A","FGHIJKLMNOPQRSTU");
	prtES("\x1B\x2A","abcdefÄ§…¡Õ”ˆ¸ı˚");
	
	prtES("\x1B\x1E","0123456789+-");	
	prtES("\x1B\x1E","ABCDEFGHIJKL");	
	prtES("\x1B\x1E","abcdefghijkl");	
	prtES("\x1B\x1E","yzÄ§…¡Õ”ˆ¸ı˚");	
	
 	prtS(" "); 	prtS(" "); prtS(" ");	
	return;
}

void printSlip(void){

	prtES("\x1B\x1D","   KIS MACK” J¡T…KBOLT");
	prtS("        BUDAPEST");
	prtS("   VIOLA UTCA 12. 1045");
	prtS("________________________");    
	prtS("D¡TUM: 2009/01/30  09:49");    
	prtS("TERMIN¡L:       POS00111");    
 	prtS("TRANZAKCI”SZ¡M:     0003");
 	prtS("K¡RTYASZ¡M / CARD NUMBER");
 	prtS("        ************0358");
 	prtES("\x1B\x1A","MAESTRO");
 	prtS("________________________");
 	prtES("\x1B\x2A","   ELUTASÕTVA!");
 	
 	prtS(" "); 	prtS(" "); prtS(" ");
 	return;
} 


void displayFonts(void){
	int key, ret;

    ret = dspXYFS(0, 21*0, 2,"01234567");
    ret = dspXYFS(0, 21*1, 2,"89ABCDEF");
    ret = dspXYFS(0, 21*2, 2,"GHIJKLMN");
    
    kbdStart(1);
    key= 0;	
    while(!key){
        key= kbdKey();
        if(key)
            break;
    }
    kbdStop();
       
    
    ret = dspXYFS(0, 21*0, 2,"OPQRSTUV");
    ret = dspXYFS(0, 21*1, 2,"XYZabcde");
    ret = dspXYFS(0, 21*2, 2,"fghijklm");
    
    kbdStart(1);
    key= 0;	
    while(!key){
        key= kbdKey();
        if(key)
            break;
    }
    kbdStop();
        
    
    ret = dspXYFS(0, 21*0, 2,"nopqrstu");
    ret = dspXYFS(0, 21*1, 2,"ˆ¸ı˚Û˙ÌÈ");
    ret = dspXYFS(0, 21*2, 2,"÷‹’€”⁄Õ¡");
    
    kbdStart(1);
    key= 0;	
    while(!key){
        key= kbdKey();
        if(key)
            break;
    }
    kbdStop();
            
    
    
    ret = dspLS(0,"ABCDEFGHIJKLMNOP");
    ret = dspLS(1,"QRSTUVWXYZ¡…Õ”⁄÷");
    ret = dspLS(2,"’‹€abcdefghijklm");
    ret = dspLS(3,"nopqrstuvwxyz·ÈÌ");
    
    kbdStart(1);
    key= 0;	
    while(!key){
        key= kbdKey();
        if(key)
            break;
    }
    kbdStop();
    
    
    ret = dspLS(0,"Û˙ˆı¸˚1234567890");
    ret = dspLS(1,"!#$%&'()*+,-./:;");
    ret = dspLS(2,"<=>?@[ ]^_¥{|}~£");
    ret = dspLS(3,"Äﬂ§&–˜◊∏___5®∞ ");
    
    kbdStart(1);
    key= 0;	
    while(!key){
        key= kbdKey();
        if(key)
            break;
    }
    kbdStop();
        
    
    ret = dspLS(BLD(0),"ABCDEFGHIJKLMNOP");
    ret = dspLS(BLD(1),"QRSTUVWXYZ¡…Õ”⁄÷");
    ret = dspLS(BLD(2),"’‹€abcdefghijklm");
    ret = dspLS(BLD(3),"nopqrstuvwxyz·ÈÌ");
        
    kbdStart(1);
    key= 0;	
    while(!key){
        key= kbdKey();
        if(key)
            break;
    }
    kbdStop();
        

    ret = dspLS(BLD(0),"Û˙ˆı¸˚1234567890");
    ret = dspLS(BLD(1),"!#$%&'()*+,-./:;");
    ret = dspLS(BLD(2),"<=>?@[ ]^_¥{|}~£");
    ret = dspLS(BLD(3),"Äﬂ§&–˜◊∏___5®∞ ");
        
    kbdStart(1);
    key= 0;	
    while(!key){
        key= kbdKey();
        if(key)
            break;
    }
    kbdStop();
        
    
    ret = dspLS(INV(0),"ABCDEFGHIJKLMNOP");
    ret = dspLS(INV(1),"QRSTUVWXYZ¡…Õ”⁄÷");
    ret = dspLS(INV(2),"’‹€abcdefghijklm");
    ret = dspLS(INV(3),"nopqrstuvwxyz·ÈÌ");
        
    kbdStart(1);
    key= 0;	
    while(!key){
        key= kbdKey();
        if(key)
            break;
    }
    kbdStop();
        

    ret = dspLS(INV(0),"Û˙ˆı¸˚1234567890");
    ret = dspLS(INV(1),"!#$%&'()*+,-./:;");
    ret = dspLS(INV(2),"<=>?@[ ]^_¥{|}~£");
    ret = dspLS(INV(3),"Äﬂ§&–˜◊∏___5®∞ ");
        
    kbdStart(1);
    key= 0;	
    while(!key){
        key= kbdKey();
        if(key)
            break;
    }
    kbdStop();
        
    
    ret = dspLS(BIG(0),"INGENICO");
    ret = dspLS(BIG(1),"TERMIN¡L");
    ret = dspLS(BIG(2),"12345678");
    
    kbdStart(1);
    key= 0;	
    while(!key){
        key= kbdKey();
        if(key)
            break;
    }
    kbdStop();

	
	
	return;
}

void tcbb0001(void) {
	int ret;
    int key= 0;	

    dspStart();
    prtStart();
    
    ret = dspLS(BLD(0),"PRINT  1:NORMAL");
    ret = dspLS(BLD(1),"2:BOLD 3:HIGH  ");
    ret = dspLS(BLD(2),"4:BIG  5:MEDIUM");
    ret = dspLS(BLD(3),"6:CONDENSED    ");
    
    do{
    	kbdStart(1);
    	key= 0;	
 	   	while(!key){
 		   key= kbdKey();
 		   if(key)
 			   break;
 	   	}
 	   	kbdStop();
    
 	   	switch (key){
 	   	case 49: 
 	   		PrintPolice("_pNORMAL_",_FIXED_WIDTH_);		//24 characters per row
 	   		break;
 	   	case 50: 
 	   		PrintPolice("_pBOLD_",_FIXED_WIDTH_);		    //24 characters per row    
 	   		break;
 	   	case 51: 
 	   		PrintPolice("_pHIGH_",_FIXED_WIDTH_);			//24 characters per row, height is 2 rows
 	   		break;
 	   	case 52:
 	   		PrintPolice("_pBIG_",_FIXED_WIDTH_);			//12 characters per row, height is 2 rows 	   		
 	   		break;
 	   	case 53:
 	   		PrintPolice("_pMEDIUM_",_FIXED_WIDTH_);		//16 characters per row, height is 1.5 rows 	   		
 	   		break; 
 	   	case 54:
 	   		PrintPolice("_pCONDENSED_",_FIXED_WIDTH_);	//48 characters per row
 	   		break; 
 	   	default:
 	   		break;
 	   	}
    } while (key > 48 && key < 55);
    
    ret = dspLS(BLD(0)," PRINTING WITH  ");
    ret = dspLS(BLD(1),"     prtES()    ");
    ret = dspLS(BLD(2),"  WITH KEY '5'. ");
    ret = dspLS(BLD(3),"                ");
    
    kbdStart(1);
    key= 0;	
    while(!key){
       key= kbdKey();
        if(key)
        break;
    }
    kbdStop();
    
    if (key == 53){
    	 printSome();
    }
    
    ret = dspLS(BLD(0)," PRINTING WITH  ");
    ret = dspLS(BLD(1),"    ESCAPES     ");
    ret = dspLS(BLD(2),"  WITH KEY '0'. ");
    ret = dspLS(BLD(3),"                ");
    
    kbdStart(1);
    key= 0;	
    while(!key){
       key= kbdKey();
        if(key)
        break;
    }
    kbdStop();
    
    if (key == 48){
    	printWithEscapes();
    }    
    
    ret = dspLS(0," PRINTING A SLIP");
    ret = dspLS(1,"  WITH KEY '1'. ");
    ret = dspLS(2,"                ");
    ret = dspLS(3,"                ");
    
    kbdStart(1);
    key= 0;	
    while(!key){
       key= kbdKey();
        if(key)
        break;
    }
    kbdStop();
    
    if (key == 49){  
    	printSlip();
    } 
    
    displayFonts();

    prtStop();    
    dspStop();
    return;
}
