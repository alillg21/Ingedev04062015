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
	
	
void tcbkw020(void)
{
	Form_Handle_t * form;
	
	StartCGUI();

	loadForm("/HOST/PHO_ENTRY.HTML", FORM_TYPE_PLAIN, TRUE, TRUE);
	
	form = getForm("/HOST/PHO_ENTRY.HTML");
	
	if (form != NULL)
	{
		displayForm(form);
	}
	else
	{
		printf("/HOST/PHO_ENTRY.HTML not found");
		ttestall(0,200);
	}
	
	EndCGUI();

}

#ifdef __cplusplus
}
#endif
