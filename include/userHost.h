#ifndef USER_HOST
#define USER_HOST
#include <stdlib.h>
#include "protocolStructure.h"


int sendTPtoRouter(transferPackage* tp, userModel * um);
int receiveTPfromRouter(transferPackage * tp, userModel * um);
int connectToRouter(userModel * um, RPAddress routerAddress);
int disconnectFromRouter(userModel * um);
#endif
