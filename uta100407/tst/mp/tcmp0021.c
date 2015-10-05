#include "SDK30.H"
#include "sys.h"
#include "TlvTree.h"

#include "oem_cless.h"    // Telium Pass include driver
#include "GTL_SharedExchange.h"
#include "ClessInterface.h"		// Telium Pass include DLL

#define mask8 ((byte)0x80)
#define mask6 ((byte)0x20)
#define mask87 ((byte)0xC0)
#define mask54321 ((byte)0x1F)
#define mask7654321 ((byte)0x7F)

static byte emvIsConstructed(const byte * tag) {    //is it a constructed tag?
    return ((*tag) & mask6) ? 1 : 0;
}

static int emvGetT(byte * tag, const byte * src) {  //extract tag from BER-TLV encoded buffer
    byte ret;

    *tag = *src;
    ret = 1;
    if(((*tag) & mask54321) != mask54321)
        return 1;               //1-byte tag
    do {                        //process multiple-byte tags
        ret++;
        tag++;
        src++;
        *tag = *src;
    } while((*tag) & mask8);
    VERIFY(ret <= 2);           //only 1-byte and 2-byte tags are supported
    return ret;
}

static int emvGetL(card * len, const byte * src) {  //extract length from BER-TLV encoded buffer
    byte ret;

    if(((*src) & mask8) != mask8) { //the easiest case : 1-byte length
        *len = *src;
        ret = 1;
        goto lblOK;
    }
    ret = (*src) & mask7654321;
    src++;
    *len = *src++;
    ret--;

    if(ret == 0) {              //two-byte length
        ret = 2;
        goto lblOK;
    }
    *len *= 0x100;
    *len += *src++;
    ret--;

    if(ret == 0) {              //three-byte length
        ret = 3;
        goto lblOK;
    }
    *len *= 0x100;
    *len += *src++;
    ret--;

    if(ret == 0) {              //four-byte length
        ret = 4;
        goto lblOK;
    }
    *len *= 0x100;
    *len += *src++;
    ret--;
    if(ret == 0) {              //five-byte length
        ret = 5;
        goto lblOK;
    }

    return -1;                  //very long TLVs are not supported
  lblOK:
    return ret;
}

static int emvGetTLV(byte * tag, card * len, byte * val, const byte * src) {    //retrieve TLV from BER-TLV encoded buffer
    word ret;

    ret = emvGetT(tag, src);    //extract tag
    if(*tag == 0)               //It is the case for ETEC 6.0, Interoper.07 MC
        return -1;
    if(ret > 2)
        return -1;

    ret += emvGetL(len, src + ret); //extract length
    src += ret;

    memcpy(val, src, (word) * len); //extract value
    ret += (word) * len;

    return ret;
}

static int trcEmvDbgRsp(const byte * dbg) { //trace reponse APDU
    int ret, idx;
    byte tag[2];                //EMV tag
    card len;                   //EMV tag length
    byte val[256];             //EMV tag value
    char tmp[256];
    byte *ptr;

    //extract debug info portion length
    ptr = (byte *) dbg;
    ret = emvGetT(tag, ptr);    //skip tag
    CHECK(ret > 0, lblKO);
    ptr += ret;
    ret = emvGetL(&len, ptr);
    CHECK(len > 0, lblKO);	
    idx = len;

    trcS("  Response<:");
    trcBN(dbg, idx);            //trace raw binary response
    trcS("\n");

    while(idx > 0) {            //parse EMV TLVs from response using BER-TLV coding rules
        memset(tag, 0, sizeof(tag));
        memset(val, 0, sizeof(val));
        ret = emvGetTLV(tag, &len, val, dbg);   //retrieve (tag,length,value)
        if(ret < 0)
            break;

        memset(tmp, 0, sizeof(tmp));
        sprintf(tmp, "   T=%02X%02X L=%3u V= ", (word) tag[0], (word) tag[1], (word) len);
        trcS(tmp);
        trcBN(val, len);        //value in binary fromat

        if(!emvIsConstructed(tag)) {
            trcS(" [");
            trcAN(val, len);    //value in ASCII format
            trcS("]\n");
            dbg += ret;
            idx -= ret;
            continue;
        }
        //process constructed tags
        trcS("\n");

        if(idx <= 0)
            break;
        ret = emvGetT(tag, dbg);    //skip tag
        dbg += ret;
        idx -= ret;

        if(idx <= 0)
            break;
        ret = emvGetL(&len, dbg);   //skip length
        if(ret < 0)
            break;
        dbg += ret;
        idx -= ret;
    }

    memset(tmp, 0, sizeof(tmp));
    sprintf(tmp, "   SW1SW2=%02X%02X\n", (word) dbg[0], (word) dbg[1]);
    trcS(tmp);

    return 1;
    lblKO:
        return -1;		
}

static void clsErr(int errCode) {
    switch(errCode) {
        case CL_OK: // Card correctly detected
            trcS("CLESS OK\n");
            break;        
        case CL_KO: // An error occurs during the card detection
            trcS("CLESS KO\n");
            break;        
        case CL_NOCARD: // No card has been detected
            trcS("No Card Detected\n");
            break;        
        case CL_TOOMANYCARDS: // Too many cards on the reader
            trcS("Too Many Cards Detected\n");
            break;              
        case CL_INVALIDCARD: // The presented card is not valid
            trcS("Invalid card\n");
            break;              
        default: // Default case
            trcS("Unknown Error\n");
            break;              
    }
}

int tcmp0021(void) {
    #ifdef __CLS__
    int ret;
    int numOfCards = 1;
    int rspLen;
    static const byte SelectPPSE[] =
		{ 0x00, 0xa4, 0x04, 0x00, 0x0e, 0x32, 0x50, 0x41, 0x59, 0x2e, 0x53, 0x59, 0x53, 0x2e, 0x44, 0x44, 0x46, 0x30, 0x31, 0x00};
    byte crdRsp[256];
    
    trcS("tcmp0021 Start\n");

    dspStart();
    ret = clsStart();
    CHECK(ret >= 0, lblKO);

    trcS("Open Driver\n");
    ret = ClessEmv_OpenDriver();
    CHECK(ret == CL_OK, lblKO);	

    prtStart();		

    trcS("Detect Card Start\n");
    ret = ClessEmv_DetectCardsStart(numOfCards, CL_TYPE_AB);
    CHECK(ret == CL_OK, lblKO);

    trcS("Detect Card\n");	
    ret = ClessEmv_DetectCards(CL_TYPE_AB, &numOfCards, 3);
    clsErr	(ret);
    CHECK(ret == CL_OK, lblKO);

    ClessEmv_DetectCardsStop();			

    trcS("Activate Card\n");	
    ret = ClessEmv_ActiveCard(0, CL_ISO14443_4);
    CHECK(ret == CL_OK, lblKO);	

     rspLen = 256;	 

    trcS("APDU Command\n");		 
     // Send the APDU to the card     
     ret = ClessEmv_Apdu(0, SelectPPSE, sizeof(SelectPPSE), crdRsp, &rspLen);
    CHECK(ret == CL_OK, lblKO);

    trcEmvDbgRsp(crdRsp);

    trcS("tcmp0021 OK");
    goto lblEnd;
    
    lblKO:
	 trcS("tcmp0021 KO");
        ret = -1;
    lblEnd:
        prtStop();	        
        ClessEmv_CloseDriver();		
        clsStop();	
        dspStop();        
        return ret;
    #else
    return -1;
    #endif
}

