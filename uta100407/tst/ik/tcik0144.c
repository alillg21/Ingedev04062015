/** \file 
 * Unitary test case for sys component(tcik0144.c): printer perypheral using FONT TOOL.
 * \sa
 *  - prtStart()
 *  - prtStop()
 *  - prtS()
 *  - prtES()
 */

#include <sdk30.h>
#include "sys.h"
#include "tst.h"
#include <string.h>

void tcik0144(void) {
    int ret;                    //*< integer value for return code */
    unsigned char *defFont = 0, *arabFont = 0, *cyrilFont = 0, *chinaFont =
        0, *symbolFont = 0;
    char buffer[300], bufftemp[10];
    unsigned char lettre;
    int i;
    const char *fnt = "/SYSTEM/COUR.SGN";

    //const char *fnt= "/SYSTEM/FAZING.SGN";

    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    dspClear();
    dspStop();

    defFont = LoadFont((char *) fnt);
    if(defFont == NULL) {
        printf("file not found : %s", fnt);
        trcFS("file not found : %s\n", fnt);
    } else {
        // Register the font used by the application
        trcS("Set default Font\n");
        DefCurrentFont(defFont);

        //DrawExtendedString8859(0,0,"ABCDEFGHIJKLMNOPQR",_OFF_,"_dMEDIUM_",_FIXED_WIDTH_);
        DrawExtendedString8859(0, 0, "0123456789012345", _OFF_, "_dMEDIUM_",
                               _FIXED_WIDTH_);
        DrawExtendedString8859(0, 16, "ABCDEFGHabcdefgh", _OFF_, "_dMEDIUM_",
                               _FIXED_WIDTH_);
        DrawExtendedString8859(0, 32, "0123456789012345", _OFF_, "_dBOLD_",
                               _FIXED_WIDTH_);
        DrawExtendedString8859(0, 48, "ABCDEFGHabcdefgh", _OFF_, "_dBOLD_",
                               _FIXED_WIDTH_);

        DrawExtendedString8859(0, 32, "ABab", _OFF_, "_dXLARGE_",
                               _FIXED_WIDTH_);
/*
		pprintf("Terminal font:\n");
		pprintf("0123456789012345\n");
		pprintf("ABCDEFGHIJKLMNOP\n");
		
		pprintf8859("Courrier new normal:\n",_OFF_,_pNORMAL_,_FIXED_WIDTH_);
		pprintf8859("0123456789012345\n",_OFF_,_pNORMAL_,_FIXED_WIDTH_);
		pprintf8859("ABCDEFGHIJKLMNOP\n",_OFF_,_pNORMAL_,_FIXED_WIDTH_);
		
		pprintf8859("Courrier new bold:\n",_OFF_,_pBOLD_,_FIXED_WIDTH_);
		pprintf8859("0123456789012345\n",_OFF_,_pBOLD_,_FIXED_WIDTH_);
		pprintf8859("ABCDEFGHIJKLMNOP\n",_OFF_,_pBOLD_,_FIXED_WIDTH_);
		
		pprintf8859("\x49\xFE\n",_OFF_,_pNORMAL_,_FIXED_WIDTH_);
		//pprintfUnicode("\x04\x49\x0\n\x0\n\x0",_OFF_,_pNORMAL_,_FIXED_WIDTH_);
	*/
    }

    cyrilFont = LoadFont("/SYSTEM/ISO5.SGN");   //application type 0105
    if(cyrilFont == NULL) {
        trcS("file not found : /SYSTEM/ISO5.SGN\n");
    } else {
        // Register the font used by the application
        DefCurrentFont(cyrilFont);
        // Print the demo cyrilic message
        lettre = 176;
        memset(buffer, 0, sizeof(buffer));
        for (i = 1; i <= 64; i++) {
            //strcat (buffer,"\x");
            sprintf(bufftemp, "%c", lettre);
            strcat(buffer, bufftemp);
            lettre++;
            if(!(i % 16))
                strcat(buffer, "\n");
        }
        strcat(buffer, "\n");
        pprintf8859(buffer, _OFF_, _pNORMAL_, _PROP_WIDTH_);
    }

    arabFont = LoadFont("/SYSTEM/ISO6.SGN");
    if(arabFont == NULL) {
        printf("file not found :\n /SYSTEM/ISO6.SGN");
        trcS("file not found : /SYSTEM/ISO6.SGN\n");
    } else {
        // Register the font used by the application
        DefCurrentFont(arabFont);
        // Print the demo arabic message
        pprintf8859
            ("\xC7\xE4\xD3\xE4\xC7\xE5\x20\xD9\xE4\xEA\xE3\xE5 : 6-9588 OSI\n",
             _OFF_, _pBOLD_, _PROP_WIDTH_);
        // print Arabic message  UNICODE coded
        pprintfUnicode
            ("\xFE\x8D\xFE\xDF\xFE\xB4\xFE\xFC\xFE\xE1\x00\x20\xFE\xCB\xFE\xE0\xFE\xF4\xFE\xDC\xFE\xE2\x0\n\x0\n\x0",
             _OFF_, _pBOLD_, _PROP_WIDTH_);
    }

    chinaFont = LoadFont("/SYSTEM/CHINA.SGN");
    if(chinaFont == NULL) {
        trcS("file not found : /SYSTEM/CHINA.SGN\n");
    } else {
        trcS("CHINE.SGN found\n");
        // Register the font used by the application
        pprintf("test chinois\n");

        DefCurrentFont(chinaFont);
        /*
           lettre = 800;
           memset(buffer, 0, sizeof(buffer));
           for (i = 1; i <= 64; i++)
           {
           //strcat (buffer,"\x");
           sprintf (bufftemp,"%c", lettre);
           strcat (buffer, bufftemp);
           lettre++;
           if(!(i % 16))
           strcat (buffer, "\x0\n\x0");
           }
           strcat (buffer, "\n");
           pprintfUnicode(buffer,_OFF_,_pNORMAL_,_PROP_WIDTH_);
         */
        //PrintPolice(_pNORMAL_,_PROP_WIDTH_);
        pprintfUnicode("\xFE\x8D\xFE\xDF\x34\xC5\x34\x34\x31\x0\n\x0", _OFF_,
                       _pNORMAL_, _PROP_WIDTH_);
        /*
           pprintfUnicode("\x01\xF4\x01\xF5\n\x0\n\x0",_OFF_,_pNORMAL_,_PROP_WIDTH_);
           pprintfUnicode("\x01\xF4\x01\xF5\n\x0\n\x0",_OFF_,_pNORMAL_,_PROP_WIDTH_);
           trcS("printed message:\n");
           trcS("\x01\xF4\x01\xF5\x0\n\x0\n\x0\n");
           //pprintfUnicode("\x0\n\x0\n\x0",_OFF_,_pNORMAL_,_PROP_WIDTH_);
           pprintf8859("1234567890\n",_OFF_,_pBNORMAL_,_PROP_WIDTH_);
           // Print the demo chinaFont message
           pprintf8859("\x01\x11\x23\x34\x47\x55\x60\x79\x84\x9A\xA3\xB3\xC4\xD5\xE5\n",_OFF_,_pNORMAL_,_PROP_WIDTH_);
           // print chinaFont message  UNICODE coded
           pprintfUnicode("\x01\x11\x23\x34\x47\x55\x60\x79\x84\x9A\xA3\xB3\xC4\xD5\xE5\n\x0",_OFF_,_pNORMAL_,_PROP_WIDTH_);
         */
        pprintf("fin test chinois\n");
    }

    //symbolFont
    symbolFont = LoadFont("/SYSTEM/SYMBOL.SGN");
    if(symbolFont == NULL) {
        trcS("file not found : /SYSTEM/SYMBOL.SGN\n");
    } else {
        trcS("Symbols !\n");
        // Register the font used by the application
        DefCurrentFont(symbolFont);
        PrintPolice(_pNORMAL_, _PROP_WIDTH_);

        //PrintPolice(_pBOLD_,_PROP_WIDTH_);
        //PrintPolice(_pLARGE_,_PROP_WIDTH_);
        /*
           // Print the demo chinaFont message
           pprintf8859("test 1\n",_OFF_,_pNORMAL_,_PROP_WIDTH_);
           pprintf8859("\x20\x30\x58\xFE\n",_OFF_,_pNORMAL_,_PROP_WIDTH_);
           // print chinaFont message  UNICODE coded
           pprintfUnicode("\x20\x30\x58\xFE\x0\n\x0\n\x0",_OFF_,_pNORMAL_,_PROP_WIDTH_);

           // Print the demo chinaFont message
           pprintf8859("test 2\n",_OFF_,_pBOLD_,_PROP_WIDTH_);
           // print chinaFont message  UNICODE coded
           //pprintfUnicode("\x01\x11\x23\x34\x47\x55\x60\x79\x84\x9A\xA3\xB3\xC4\xD5\xE5\x0\n\x0\n\x0",_OFF_,_pBOLD_,_PROP_WIDTH_);

           // Print the demo chinaFont message
           pprintf8859("test 3\n",_OFF_,_pLARGE_,_PROP_WIDTH_);
           // print chinaFont message  UNICODE coded
           pprintfUnicode("\x01\x11\x23\x34\x47\x55\x60\x79\x84\x9A\xA3\xB3\xC4\xD5\xE5\x0\n\x0\n\x0",_OFF_,_pLARGE_,_PROP_WIDTH_);
         */
    }
    goto lblEnd;
  lblKO:
    trcErr(ret);
  lblEnd:
    tmrPause(3);
    dspStart();
    dspClear();
    prtStop();                  //close resource
}
