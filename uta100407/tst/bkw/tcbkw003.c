/** \file
 * Unitary non UTA test case for ...
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/bkw/tcbkw003.c $
 *
 * $Id: tcbkw003.c 2571 2010-02-11 22:46:30Z bkwok $
 */

#include "SDK30.H"
#include "WGUI.h"
#include "WGUI_PlugIn.h"
#include "WGUI_Javascript.h"
#include "fom.h"
#include "cgu.h"

#ifdef __cplusplus
extern "C"
{
#endif
	

void tcbkw003(void)
{
	Form_Handle_t * form;
	
	StartCGUI();
	
	loadForm("/HOST/ALPHANUM.HTML", FORM_TYPE_PLAIN, FALSE, FALSE);
	
	form = getForm("/HOST/ALPHANUM.HTML");
	
	if (form != NULL)
	{
		displayForm(form);
	}
	else
	{
		printf("/HOST/ALPHANUM.HTML not found");
		ttestall(0,200);
	}
	EndCGUI();
		
		
}


#ifdef __cplusplus
}
#endif
