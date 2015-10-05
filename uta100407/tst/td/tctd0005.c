
// Functional test case tctd0005
//
// author:      Tamas Dani
// date:        28.06.2005
// description: test for beep

#include <string.h>
#include <unicapt.h>
#include "log.h"

#ifdef __TEST__

static code const char *srcFile = __FILE__;
static uint32 hHmi= 0;

static void BeepTest( int param )
{
	int ret;
	int iDebug;

	switch( param )
	{
	case 1 : ret = hmiBeep( hHmi, HMI_BEEP_CLICK, HMI_BEEP_LOW     ); break;
	case 2 : ret = hmiBeep( hHmi, HMI_BEEP_CLICK, HMI_BEEP_MIDTONE ); break;
	case 3 : ret = hmiBeep( hHmi, HMI_BEEP_CLICK, HMI_BEEP_HIGH    ); break;
	case 4 : ret = hmiBeep( hHmi, HMI_BEEP_SHORT, HMI_BEEP_LOW     ); break;
	case 5 : ret = hmiBeep( hHmi, HMI_BEEP_SHORT, HMI_BEEP_MIDTONE ); break;
	case 6 : ret = hmiBeep( hHmi, HMI_BEEP_SHORT, HMI_BEEP_HIGH    ); break;
	case 7 : ret = hmiBeep( hHmi, HMI_BEEP_LONG , HMI_BEEP_LOW     ); break;
	case 8 : ret = hmiBeep( hHmi, HMI_BEEP_LONG , HMI_BEEP_MIDTONE ); break;
	case 9 : ret = hmiBeep( hHmi, HMI_BEEP_LONG , HMI_BEEP_HIGH    ); break;

	case 10: 
		ret = hmiBeep( hHmi, HMI_BEEP_FOREVER, HMI_BEEP_LOW );
		tmrPause( 3 );
		ret = hmiBeepStop( hHmi );
		break;
	case 11: 
		ret = hmiBeep( hHmi, HMI_BEEP_FOREVER, HMI_BEEP_MIDTONE );
		tmrPause( 3 );
		ret = hmiBeepStop( hHmi );
		break;
	case 12: 
		ret = hmiBeep( hHmi, HMI_BEEP_FOREVER, HMI_BEEP_HIGH );
		tmrPause( 3 );
		ret = hmiBeepStop( hHmi );
		break;

	default:
		break;
	}

	switch(ret)
	{
	case RET_OK              : iDebug=0; break; // buzzer started correctly at the indicated frequency.
	case ERR_INVALID_HANDLE  : iDebug=0; break; // some channels are already opened, but that handle was not attributed by the peripheral.
	case ERR_CONFLICT        : iDebug=0; break; // The application that tries to use the handle is not the application that opened the channel
	case ERR_SYS_RESOURCE_PB : iDebug=0; break; // resource problem: the peripheral could not open the channel because it could not be allocated, run tasks, find its configuration.
	}
}

void tctd0005(void)
{
	int i;
	int ret;

	ret = hmiOpen("DEFAULT",&hHmi);
	CHECK(ret==RET_OK,lblKO);

	for( i=1; i<=12; i++ )
	{
		BeepTest( i );
		tmrPause( 4 );
	}

	ret = hmiClose(hHmi);
	hHmi=0;

	CHECK(ret==RET_OK,lblKO);
	return;

lblKO:
	return;
}

#endif
