/** \file
 * Unitary non UTA test case for Displaying a form at Idle Screen
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/bkw/tcbkw007.c $
 *
 * $Id: tcbkw007.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include "SDK30.H"
#include "WGUI.h"
#include "tst.h"

#ifdef __cplusplus
extern "C"
{
#endif
	
extern void (* idleForm)();

	
#define USER2_APPLI_TYPE	   0x6e


void launchSwipe(void);
int ManageMenu( const char *szTitle, int nItems, const char* Items[]);

static const char *tzMenuIdleScreen[] =
{
	"Signature Capture",	
	"Multi Media",
	"Alphanumeric",
	"Swipe Card",
	"Pin Entry with CGUI",
	0
};

static const char *tzMenuIdleDisplay[] =
{
	"Select Idle Display",	
	"Turn Off Idle Display",
	0
};

static void SelectIdleScreen(void)
{
	switch (ManageMenu("Select Idle Screen", 5, tzMenuIdleScreen))
	{
		case 0:  idleForm = tcbkw001;            break; 
		case 1:  idleForm = tcbkw002;            break; 
		case 2:  idleForm = tcbkw003;            break; 
		case 3:  idleForm = tcbkw004;            break; 
		case 4:  idleForm = tcbkw006;            break; 
		default: 				         break; // Abort key pressed
	}
}

static void TurnOffIdleScreen(void)
{
	idleForm = NULL;
}

void tcbkw007(void)
{
	switch (ManageMenu("Idle Screen", 2, tzMenuIdleDisplay))
	{
		case 0:  SelectIdleScreen();            break; 
		case 1:  TurnOffIdleScreen();            break;
		default: 				         break; // Abort key pressed
	}
}

#ifdef __cplusplus
}
#endif
