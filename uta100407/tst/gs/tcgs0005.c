// tcgs0005
//
// author:      Gábor Sárospataki
// date:        03.01.2006
// description: test case to printing styles

#include <string.h>
#include "sys.h"

#define CHK CHECK(ret>=0,lblKO)

void tcgs0005(void) {
    int ret;

    ret = prtStart();           //open the printer
    CHECK(ret >= 0, lblKO);

    // print string in differnt sizes
    ret = prtS("\x1b\x1b" "String in diff. sizes:");
    CHK;
    ret = prtS("\x1b\x20" "Here is the string!");
    CHK;
    ret = prtS("\x1b\x21" "Here is the string!");
    CHK;
    ret = prtS("\x1b\x1b" "Here is the string!");
    CHK;
    ret = prtS("\x1b\x1e" "Here is the string!");
    CHK;
    ret = prtS("\x1b\x1f" "Here is the string!");
    CHK;
    ret = prtS("\x1b\x1b" "In delault size:");
    CHK;
    ret = prtS("Here is the string!");
    CHK;

    ret = prtS("\x1b\x1b" "Locations:");
    CHK;
    //it doesn't work in simulator, but works on terminal (bug in simulator)
    ret = prtS("\x1b\x21Left" "\x1b\x17" "Center" "\x1b\x18" "Right");
    CHK;

    ret = prtS("\x1b\x20Left" "\x1b\x17" "Center" "\x1b\x18" "Right");
    CHK;

    ret = prtS("\x1b\x1b" "Types:");
    CHK;
    ret =
        prtS("\x1b\x1bNormal" "\x1b\x1a\x1b\x17" "Bold" "\x1b\x19\x1b\x18"
             "Normal");
    CHK;

    goto lblEnd;

  lblKO:
    trcErr(ret);
  lblEnd:;
    prtStop();                  //close resource
}
