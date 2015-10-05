#ifndef _FOM_H
#define _FOM_H

#define MAX_FILE_LENGTH 150
#define MAX_NUM_FORM    128


#define FORM_TYPE_PLAIN			0
#define FORM_TYPE_SIGNCAPT	    1
#define FORM_TYPE_PINENTRY      2
#define FORM_TYPE_AJAX          3

typedef struct Form_Handle
{
	char   				 name[MAX_FILE_LENGTH];
	int 			     type;
	bool 				 enable_msr;
	bool 			     enable_rfid;
	char * 				 html_code;
} Form_Handle_t;

Form_Handle_t *     loadForm(char * filename, int type, bool enableMSR, bool enableRFID);
void 				preloadForm();
Form_Handle_t *  	getForm(char * filename);

#endif
