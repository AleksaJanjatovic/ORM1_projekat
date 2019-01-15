#ifndef ROUTER_HOST
#define ROUTER_HOST
#include "../include/protocolStructure.h"


/**
 * @brief routerTableTimeControl: Function designed for periodic refreshing of a router table
 * @param rm: targer router
 */
void routerTableTimeControl(routerModel * rm);


/**
 * @brief parseReceivedData: FUnction handles the reception of data of a router
 * @param rm: targe router
 * @return error flags (negative number), or 0 if no errors occurred
 */
int parseReceivedData(routerModel * rm);
#endif
