#ifndef USER_HOST
#define USER_HOST
#include <stdlib.h>
#include "protocolStructure.h"


int sendTPtoRouter(transferPackage* tp, userModel * rm, char attempts);
int receiveTPfromRouter(transferPackage * tp, userModel * rm);
int connectToRouter(userModel * um, RPAddress routerAddress);
int disconnectFromRouter(userModel * um);
#endif
