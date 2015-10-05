/** \file
 * Unitary non UTA test case for MultiMedia
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/bkw/tcbkw002.c $
 *
 * $Id: tcbkw002.c 2571 2010-02-11 22:46:30Z bkwok $
 */

#include "SDK30.H"
#include "WGUI.h"
#include "WGUI_Plugin.h"
#include "fom.h"
#include "cgu.h"

#ifdef __cplusplus
extern "C"
{
#endif
	


/** Launch MultiMedia */
void tcbkw002(void)
{
	Form_Handle_t * form;
	
	StartCGUI();
	
	loadForm("/HOST/MMEDIA.HTML", FORM_TYPE_PLAIN, FALSE, FALSE);
	
	form = getForm("/HOST/MMEDIA.HTML");
	
	if (form != NULL)
	{
		displayForm(form);
	}
	else
	{
		printf("/HOST/MMEDIA.HTML not found");
		ttestall(0,200);
	}

	EndCGUI();
}

#ifdef __cplusplus
}
#endif
