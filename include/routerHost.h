#ifndef ROUTER_HOST
#define ROUTER_HOST
#include "../include/protocolStructure.h"

int sendRouterTable(routerModel * rm);
int receiveRouterTable(routerModel * rm);
void findPath(unsigned char nextRouter, unsigned char destRouter, routerModel * rt, unsigned char path[MAXROUTERS], unsigned char * nodeNum, char * finished);
void routerTableTimeControl(routerModel * rm);
//void removeRouter(routerModel * rm);
//void parseTPRouter(routerModel * rm, transferPackage * tp);
int parseTP(routerModel * rm);
#endif
