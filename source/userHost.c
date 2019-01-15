#include "../include/userHost.h"
#include "../include/initRoutingP.h"
#include <unistd.h>
#include <stdio.h>
#include "../include/utilityFun.h"
#include "../include/debugging.h"

int sendTPtoRouter(transferPackage * tp, userModel * um) {
    tp->packageType = 0;
    tp->nodeNum = 0;
    tp->currentNode = 0;

    convertTPackageToArray(tp, um->sendTPBuffer);
    if(sendto(um->socket, um->sendTPBuffer, CONVBUFFSIZETP, 0, (struct sockaddr *)&(um->homeHost), sizeof(struct sockaddr_in)) == -1){
        perror("User-To-Router TP sending error:");
        return - 1;
    }

    return 0;
}

int receiveTPfromRouter(transferPackage* tp, userModel * um) {
    size_t len = sizeof(struct sockaddr_in);
    if(recvfrom(um->socket, um->receiveTPBuffer, CONVBUFFSIZETP, 0, (struct sockaddr*)&(um->homeHost), (socklen_t*)&len) == -1) {
        perror("Router-To-User TP receiving error:");
        return -1;
    }
    convertArrayToTPackage(um->receiveTPBuffer, tp);
    printf("Package received: \n");
    printTPackage(tp);
    return 0;
}
