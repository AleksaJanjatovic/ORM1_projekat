#include <stdio.h>
#include "include/initRoutingP.h"
#include <string.h>
#include <pthread.h>

int main(int argc, char * argv[]) {
    routerModel rm1;
    userModel um;
    transferPackage tp;
//    setRouterNumber(200, tp.destinationAddress);
//    setUserNumber(123, tp.destinationAddress);
//    setUserNumber(202, tp.destinationAddress);

//    setRouterNumber(12, tp.sourceAddress);
//    setUserNumber(125, tp.sourceAddress);
    tp.packageType = 1;
//    tp.dataSent = 10;
//    printTPPackage(&tp);
//    convertTPackageToArray(&tp, um.transferBuffer);
//    convertArrayToTPackage(um.transferBuffer, &tp);
//    printTPPackage(&tp);

    initUM(atoi(argv[1]), argv[2], &um);
    RPAddress dummy;
    initRM(atoi(argv[3]), argv[4], dummy, &rm1, 1);
//    printRouterModel(&rm1);
//    printUserModel(&um);
    connectUMToNetwork(&um, &rm1);

    closeRM(&rm1);
    closeUM(&um);
    return 0;
}
