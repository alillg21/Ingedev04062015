/** \file
 * Unitary non UTA test case for signature capture
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/bkw/tcbkw001.c $
 *
 * $Id: tcbkw001.c 2571 2010-02-11 22:46:30Z bkwok $
 */

#include "SDK30.H"
#include "WGUI.h"
#include "fom.h"
#include "cgu.h"

#ifdef __cplusplus
extern "C"
{
#endif
	
	
void tcbkw001(void)
{
	Form_Handle_t * form;
	
	StartCGUI();

	loadForm("/HOST/SIGNCAPT.HTML", FORM_TYPE_SIGNCAPT, FALSE, FALSE);
	
	form = getForm("/HOST/SIGNCAPT.HTML");
	
	if (form != NULL)
	{
		displayForm(form);
	}
	else
	{
		printf("/HOST/SIGNCAPT.HTML not found");
		ttestall(0,200);
	}
	
	EndCGUI();
}


#ifdef __cplusplus
}
#endif
