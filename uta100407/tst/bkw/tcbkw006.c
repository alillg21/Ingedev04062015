#include "SDK30.H"
#include "WGUI.h"
#include "WGUI_Plugin.h"
#include "fom.h"
#include "cgu.h"





void tcbkw006(void) {
	
	Form_Handle_t * form;
	
	StartCGUI();
	
	loadForm("/HOST/PINENTRY.HTML", FORM_TYPE_PINENTRY, FALSE, FALSE);
	
	form = getForm("/HOST/PINENTRY.HTML");
	
	if (form != NULL)
	{
		displayForm(form);
	}
	else
	{
		printf("/HOST/PINENTRY.HTML not found");
		ttestall(0,200);
	}

	EndCGUI();
	
}

