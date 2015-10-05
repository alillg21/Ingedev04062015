/** \file
 * Unitary non UTA test case for ...
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/bkw/tcbkw004.c $
 *
 * $Id: tcbkw004.c 2571 2010-02-11 22:46:30Z bkwok $
 */

#include "SDK30.H"
#include "WGUI.h"
#include "tst.h"
#include "fom.h"
#include "cgu.h"

#ifdef __cplusplus
extern "C"
{
#endif
	
	
void tcbkw004(void)
{
	
	Form_Handle_t * form;
	
	StartCGUI();	
	
	loadForm("/HOST/SWIPE.HTML", FORM_TYPE_PLAIN, TRUE, FALSE);
	
	form = getForm("/HOST/SWIPE.HTML");
	
	if (form != NULL)
	{
		displayForm(form);
	}
	else
	{
		printf("/HOST/SWIPE.HTML not found");
		ttestall(0,200);
	}
	EndCGUI();

//	font_size(4,8);        
//    putchar('\x1B');         
//    printf("Track1=%s\n", tcTrk1);
//    printf("Track2=%s\n", tcTrk2);
//    printf("Track3=%s\n", tcTrk3);
}

#ifdef __cplusplus
}
#endif
