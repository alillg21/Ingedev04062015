
// Functional test case tctd0001.
//
// author:      Tamas Dani
// date:        10.05.2005
// description: working with non-volatile memory

#include <string.h>
#include "log.h"

#define CHK CHECK( ret >= 0, lblKO )

static code const char *srcFile = __FILE__;

void tctd0001(void)
{
	int    i;
	int    ret;
	tTable tab;
	char   tmp[dspW+1];

	nvmStart(); //initialising non-volatile memory
	ret= dspStart(); CHK;

	ret = tabInit( &tab, 0, 0, dspW, 4 );
	VERIFY( ret == tabSize( &tab ));

	for( i = 0; i < tabDim( &tab ); i++ )
	{
		ret = tabGetStr( &tab, i, tmp ); //retrieve record

		if( strlen( tmp ) == 0 )
		{
			// initialising in case the memmory was never filled yet
			switch( i )
			{
			case  0: ret = tabPutStr( &tab, i, "Sanyi" ); CHK; break;
			case  1: ret = tabPutStr( &tab, i, "Robi"  ); CHK; break;
			case  2: ret = tabPutStr( &tab, i, "Pali"  ); CHK; break;
			case  3: ret = tabPutStr( &tab, i, "Tomi"  ); CHK; break;
			default: ret = -1;                            CHK; break;
			}
		}
		else
		{
			ret= dspLS( i, tmp ); CHK;
			tmrPause(1);
		}
	}

	goto lblEnd;

lblKO:
	trcErr(ret);
lblEnd:
	dspStop();
}
