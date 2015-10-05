/** \file
 * Unitary non UTA test case for Displaying a scrolling text box
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/bkw/tcbkw0011.c $
 *
 * $Id: tcbkw0011.c 2534 2010-01-06 10:35:08Z abarantsev $
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
	
	
void tcbkw030(void)
{
	Form_Handle_t * form;
	
	StartCGUI();

	loadForm("/HOST/PINENTRY.HTM", FORM_TYPE_PINENTRY, FALSE, FALSE);
	
	form = getForm("/HOST/PINENTRY.HTM");
	
	if (form != NULL)
	{
		displayForm(form);
	}
	else
	{
		printf("/HOST/PINENTRY.HTM not found");
		ttestall(0,200);
	}
	
	EndCGUI();

}

#ifdef __cplusplus
}
#endif
