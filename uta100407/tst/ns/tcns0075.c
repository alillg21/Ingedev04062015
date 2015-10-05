/** \file
 * Unitary test case for sys component: printer perypheral, based on tcab0000.c
 * demonstrates adhesive printing using the special characters string "\x1b\x03" (on TELIUM, takes no effect on UNICAPT)
 *  - prtES()
 */

#include "sys.h"
#include "tst.h"

void tcns0075(void) {
	int ret = 0;
#ifdef __TELIUM__
		
    char *dft, *currFont;
	 	 
#endif
    ret = prtStart();           //open the associated channel
    CHECK(ret >= 0, lblKO);

#ifdef __TELIUM__

	currFont = GetCurrentFont(); // save default font into the variable
	 		 	 
	dft = LoadFont("/SYSTEM/ISO6.SGN"); // your ISO6 font should be loaded to SWAP disk first!!!
        DefCurrentFont(dft); // Set current font
#endif
    
    /*!
     * \par prtES(const char *  esc, const char *  str )
     * this function print formated by special characters string
     *
     * \htmlonly
     <table border=1 cellspacing=0 cellpadding=0>
     <tr> <td>INGENICO Control chars</td>   <td>SAGEM Control chars</td> <td>Print effect</td>                  </tr>
     <tr> <td>\x1b\x03 - takes no effect</td>  <td>\x1b\x03</td> <td>adhesive printing (no gaps between letters, usually needs for arabic fonc printing properly)</td>      </tr>
     </table>
     \endhtmlonly
     */

 		 prtS("bold: ");
		 prtES("\x1b\x1a","\x31\x32\x34\xE1\xC5\xDA\xC7\xCF\xC9\x20\xC7\xE1\xD4\xCD\xE4");
		 prtES("\x1b\x1a","test");

		 prtS("bold adhesive: ");
 		 prtES("\x1b\x1a\x1b\x03","\x31\x32\x34\xE1\xC5\xDA\xC7\xCF\xC9\x20\xC7\xE1\xD4\xCD\xE4");
 		 prtES("\x1b\x1a\x1b\x03","test");
 		 
 		 prtS("");
		 
		 prtS("high");
		 prtES("\x1b\x1d","\xE1\xC5\xDA\xC7\xCF\xC9\x20\xC7\xE1\xD4\xCD\xE4");
		 prtES("\x1b\x1d","test");

		 prtS("high adhesive");
		 prtES("\x1b\x1d\x1b\x03","\xE1\xC5\xDA\xC7\xCF\xC9\x20\xC7\xE1\xD4\xCD\xE4");
		 prtES("\x1b\x1d\x1b\x03","test");
		 
		 prtS("");
		 
		 prtS("big");
		 prtES("\x1b\x1e","\xE1\xC5\xDA\xC7\xCF\xC9\x20\xC7\xE1\xD4\xCD\xE4");
		 prtES("\x1b\x1e","test");

		 prtS("big adhesive");
		 prtES("\x1b\x1e\x1b\x03","\xE1\xC5\xDA\xC7\xCF\xC9\x20\xC7\xE1\xD4\xCD\xE4");
		 prtES("\x1b\x1e\x1b\x03","test");
		 
		 prtS("");
		 
		 prtS("condenced");
		 prtES("\x1b\x20","\xE1\xC5\xDA\xC7\xCF\xC9\x20\xC7\xE1\xD4\xCD\xE4");
		 prtES("\x1b\x20","test");

		 prtS("condenced adhesive");
 		 prtES("\x1b\x20\x1b\x03","\xE1\xC5\xDA\xC7\xCF\xC9\x20\xC7\xE1\xD4\xCD\xE4");
 		 prtES("\x1b\x20\x1b\x03","test");
 		 
 		 prtS("");
		 
		 prtS("medium");
		 prtES("\x1b\x2a","\xE1\xC5\xDA\xC7\xCF\xC9\x20\xC7\xE1\xD4\xCD\xE4");
		 prtES("\x1b\x2a","test");

		 prtS("medium adhesive");
 		 prtES("\x1b\x2a\x1b\x03","\xE1\xC5\xDA\xC7\xCF\xC9\x20\xC7\xE1\xD4\xCD\xE4");
 		 prtES("\x1b\x2a\x1b\x03","test");
		 
		 prtS("");
		 
		 prtS("centered");
		 prtES("\x1b\x17","\xE1\xC5\xDA\xC7\xCF\xC9\x20\xC7\xE1\xD4\xCD\xE4");
		 prtES("\x1b\x17","test");

		 prtS("centered adhesive");
		 prtES("\x1b\x17\x1b\x03","\xE1\xC5\xDA\xC7\xCF\xC9\x20\xC7\xE1\xD4\xCD\xE4");
		 prtES("\x1b\x17\x1b\x03","test");
		 
		 prtS("");
		 
		 prtS("reversed");
		 prtES("\x1b\x22","\xE1\xC5\xDA\xC7\xCF\xC9\x20\xC7\xE1\xD4\xCD\xE4");
		 prtES("\x1b\x22","test");

		 prtS("reversed adhesive");
 		 prtES("\x1b\x22\x1b\x03","\xE1\xC5\xDA\xC7\xCF\xC9\x20\xC7\xE1\xD4\xCD\xE4");
 		 prtES("\x1b\x22\x1b\x03","test");
		 
		 prtS("");
		 
		 prtS("righ justified");
		 prtES("\x1b\x18","\xE1\xC5\xDA\xC7\xCF\xC9\x20\xC7\xE1\xD4\xCD\xE4");
		 prtES("\x1b\x18","test");

		 prtS("righ adhesive");
		 prtES("\x1b\x18\x1b\x03","\xE1\xC5\xDA\xC7\xCF\xC9\x20\xC7\xE1\xD4\xCD\xE4");
 		 prtES("\x1b\x18\x1b\x03","test");
		 
	#ifdef __TELIUM__	
 	 	 DefCurrentFont(currFont); // restore the default font
	#endif
  

    goto lblEnd;
  lblKO:
    trcErr(ret);
  lblEnd:
    prtStop();                  //close resource
    trcS("tcns0075 End\n");
}
