#include "resources.h"
//#include "tst.h"

void tcmlMnu(int itm){
    switch (itm){
      case ID_FONT: tcml0005(); break;
      case ID_BCR_SYS: tcik0105(); break;
      //case ID_BCR_PRI: tcik0106(); break;
      default: break;
    }
}
