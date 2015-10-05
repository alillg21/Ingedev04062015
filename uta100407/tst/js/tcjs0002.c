#include <string.h>
#include "pri.h"
#include "tst.h"

static code const char *srcFile= __FILE__;

int ICCCard_SelectFil(int FileId);
int ICCCard_VerifySecretCode(byte apSCN);

byte gSecretCode[3][9] = {
							{"CSPIGMY1"},
							{"CSPIGMY2"},
							{"CSPIGMY3"}
						};	

#define ICCCARD_DATAFILE			0x01
#define ICCCARD_TRANFILE			0x02

#ifdef jai
        hmiOpDrawLine(pCanv, pGC, (Coord)xPos, (Coord)(yPos+1), (Coord)xPos, (Coord)(yPos+height-2));                    // left line
        hmiOpDrawLine(pCanv, pGC, (Coord)(xPos+width-1), (Coord)(yPos+1), (Coord)(xPos+width-1), (Coord)(yPos+height-2));    // right line
        hmiOpDrawLine(pCanv, pGC, (Coord)(xPos+1), (Coord)yPos, (Coord)(xPos+width-2), (Coord)yPos);                    // top line
        hmiOpDrawLine(pCanv, pGC, (Coord)(xPos+1), (Coord)(yPos+height-1), (Coord)(xPos+width-2), (Coord)(yPos+height-1));    // bottom line
#endif
static int prtH(const byte *buf,byte len){
    int ret;
    char hex[512+1];
    char *ptr;
    word idx;    
    VERIFY(buf);
    
    memset(hex,0,512+1);
    bin2hex(hex,buf,len);
    idx= (word)len+len;
    ptr= hex;
    while(idx){
        ret= prtS(ptr);
        CHECK(ret>=0,lblKO);
        if(idx<prtW) break;
        idx-= prtW;
        ptr+= prtW;
    }
    return len;
lblKO:
    return -1;
}
//Test Case Enter the IP address
void tcjs0002(void){
    int ret;
    int len;
    byte rsp[256];
    char *ptr;

    ret= dspStart();
    CHECK(ret>=0,lblKO);
    
    ret= prtStart();
    CHECK(ret>=0,lblKO);
    
    ret= dspLS(0,"Insert a card...");
    CHECK(ret>=0,lblKO);

    ret= tmrStart(0,60*100); //60 seconds timeout
    CHECK(ret>=0,lblKO); 

    ret= iccStart(0); //Open associated channel and start waiting for a card
    CHECK(ret>=0,lblKO);
    
    while(tmrGet(0)){
        ret= iccCommand(0,(byte *)0,(byte *)0,rsp); //look for a result; if a card is inserted, capture ATR
        if(ret==-iccCardRemoved) continue;
        CHECK(ret>=0,lblKO);
        if(!ret) continue;
        len= ret;
        break;
    }
    
    if(!tmrGet(0)) goto lblEnd; //card is not inserted
    tmrStop(0);
    
    ret= prtS("ATR:");
    CHECK(ret>=0,lblKO);
    
    ret= prtH(rsp,(byte)len);
    CHECK(ret>=0,lblKO);

    ret= dspLS(0,"ATR printed");
    CHECK(ret>=0,lblKO);
    
    ret= dspLS(1,"Wait...");
    CHECK(ret>=0,lblKO);

    tmrPause(5);

    ret= iccCommand(0,(byte *)"\x04\x00\xA4\x04\x00 ",(byte *)"\x0E""1PAY.SYS.DDF01",rsp); //APPLICATION SELECT
    if(ret<=0){
        switch(-ret){
            case 0: case iccCardRemoved: ptr= "Card removed"; break;
            case iccCardMute: ptr= "Card mute"; break;
            case iccCardPb: ptr= "Card Pb"; break;
            case iccDriverPb: ptr= "Dirver Pb"; break;
            case iccKO:
            default: ptr= "Unknown Pb"; break;
        }
        dspLS(1,ptr);
    }else{
        len= ret;
        ret= prtS("APPLICATION SELECT:");
        CHECK(ret>=0,lblKO);
        
        ret= prtH(rsp,(byte)len);
        CHECK(ret>=0,lblKO);
        
        ret= dspLS(1,"APP SELECT OK");
        CHECK(ret>=0,lblKO);
    }
	

	//select file
	ICCCard_SelectFil(ICCCARD_DATAFILE);
	tmrPause(2);
	ICCCard_VerifySecretCode(0);
	

    ret= dspLS(2,"Remove card...");
    CHECK(ret>=0,lblKO);

    tmrPause(5);
    goto lblEnd;

lblKO:
    trcErr(ret);
    dspLS(0,"KO!");
    tmrPause(3);
lblEnd:
    iccStop(0); //stop waiting card and close associated channel
    prtStop();
    dspStop();
}

int ICCCard_SelectFil(int FileId)
{
	int ret;
    int len;
    byte rsp[256];
    char *ptr;

	ret= iccCommand(0,(byte *)"\x04\x00\xA4\x01\x0C ",(byte *)"\x02""\x02\x00",rsp); //APPLICATION SELECT
    if(ret<=0){
        switch(-ret){
            case 0: case iccCardRemoved: ptr= "Card removed"; break;
            case iccCardMute: ptr= "Card mute"; break;
            case iccCardPb: ptr= "Card Pb"; break;
            case iccDriverPb: ptr= "Dirver Pb"; break;
            case iccKO:
            default: ptr= "Unknown Pb"; break;
        }
        dspLS(1,ptr);
    }else{
        len= ret;
        ret= prtS("1st FILE SELECTED");
        CHECK(ret>=0,lblKO);
        
        ret= prtH(rsp,(byte)len);
        CHECK(ret>=0,lblKO);
        
        ret= dspLS(1,"FIRST FILE SEL OK");
        CHECK(ret>=0,lblKO);


		if (FileId == ICCCARD_DATAFILE)
			ret= iccCommand(0,(byte *)"\x04\x00\xA4\x02\x0C ",(byte *)"\x02""\x02\x03",rsp); //DATA FILE SELECT
		else if (FileId == ICCCARD_TRANFILE)
			ret= iccCommand(0,(byte *)"\x04\x00\xA4\x02\x0C ",(byte *)"\x02""\x02\x05",rsp); //TRAN FILE SELECT

		if(ret<=0){
			switch(-ret){
				case 0: case iccCardRemoved: ptr= "Card removed"; break;
				case iccCardMute: ptr= "Card mute"; break;
				case iccCardPb: ptr= "Card Pb"; break;
				case iccDriverPb: ptr= "Dirver Pb"; break;
				case iccKO:
				default: ptr= "Unknown Pb"; break;
			}
			dspLS(1,ptr);
			}else{
				len= ret;
				ret= prtS("DATA FILE SELECTED");
				CHECK(ret>=0,lblKO);
        
				ret= prtH(rsp,(byte)len);
				CHECK(ret>=0,lblKO);
        
				ret= dspLS(1,"DF/TF SEL OK");
				CHECK(ret>=0,lblKO);
			}
    }

goto lblEnd;
lblKO:
    trcErr(ret);
    dspLS(0,"KO!");
    tmrPause(3);
lblEnd:
    iccStop(0); //stop waiting card and close associated channel
    prtStop();
    dspStop();
	return ret;
	
}

//int ICCCard_ReadBinary(byte *apBuf)
int ICCCard_VerifySecretCode(byte apSCN)
{
	int ret;
    int len;
	int i;
	byte req[100];
	byte cmd[10];
    byte rsp[256];
    char *ptr;

	
	cmd[0] = '\x04'; //TYPE
	cmd[1] = '\x00'; //CLA
	cmd[2] = '\x20'; //INS
	cmd[3] = '\x00'; //P1
	cmd[4] = apSCN; //P2

	

	req[0] = 0x08;
	
	for (i=1;i<=8 ; i++)
	{
		req[i] = gSecretCode[apSCN][i];
	}


	ret= iccCommand(0,cmd,req,rsp); //verify
	//ret= iccCommand(0,(byte *)"\x04\x00\x20\x00\x00 ",(byte *)"\x08""CSPIGMY1",rsp); 


	if(ret<=0){
		switch(-ret){
			case 0: case iccCardRemoved: ptr= "Card removed"; break;
			case iccCardMute: ptr= "Card mute"; break;
			case iccCardPb: ptr= "Card Pb"; break;
			case iccDriverPb: ptr= "Dirver Pb"; break;
			case iccKO:
			default: ptr= "Unknown Pb"; break;
		}
		dspLS(1,ptr);
		}else{
			len= ret;
			ret= prtS("PASSWORD VERIFIED");
			CHECK(ret>=0,lblKO);
    
			ret= prtH(rsp,(byte)len);
			CHECK(ret>=0,lblKO);
    
			ret= dspLS(1,"PASSWORD OK");
			CHECK(ret>=0,lblKO);
		}
		tmrPause(2);

goto lblEnd;
lblKO:
    trcErr(ret);
    dspLS(0,"KO!");
    tmrPause(3);
lblEnd:
    iccStop(0); //stop waiting card and close associated channel
    prtStop();
    dspStop();
	return ret;
	
}
