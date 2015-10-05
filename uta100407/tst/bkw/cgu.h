#ifndef _CGU_H
#define _CGU_H

#include "fom.h"

int displayForm(Form_Handle_t * form);
int ProcessSwipeEvent();
void StartCGUI();
void EndCGUI();
int HandlePinEntryPage(WGUI_CANVAS_HANDLE canvas, WGUI_BROWSER_HANDLE browser, Form_Handle_t * form);
int HandleAjaxPage(WGUI_CANVAS_HANDLE canvas, WGUI_BROWSER_HANDLE browser, Form_Handle_t * form);
void SaveFormReturnValues(WGUI_HTML_EVENT_HANDLE evt);
#endif

