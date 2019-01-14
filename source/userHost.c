#include "../include/userHost.h"
#include "../include/initRoutingP.h"
#include <unistd.h>
#include <stdio.h>
#include "../include/utilityFun.h"

int sendTPtoRouter(transferPackage * tp, userModel * um) {
    int errorCounter = 0;
    tp->packageType = 0;
    convertTPackageToArray(tp, um->sendTPBuffer);
    if(sendto(um->socket, um->sendTPBuffer, CONVBUFFSIZETP, 0, (struct sockaddr *)&(um->homeHost), sizeof(struct sockaddr_in)) == -1){
        perror("User-To-Router TP sending error:");
        return - 1;
    }
    return 0;
}

int receiveTPfromRouter(transferPackage* tp, userModel * um) {
    size_t len = sizeof(struct sockaddr_in);
    printf("%d\n", um->userHost.sin_addr.s_addr);
    printf("%d\n", um->userHost.sin_port);
    //printf("Poziv receiveTPFROMROUTER\n");
    if(recvfrom(um->socket, um->recieveTPBuffer, CONVBUFFSIZETP, 0, (struct sockaddr*)&(um->homeHost), (socklen_t*)&len) == -1) {
        perror("Router-To-User TP receiving error:");
        return -1;
    }
    convertArrayToTPackage(um->recieveTPBuffer, tp);
    printf("Package received:\n");
    printTPPackage(&tp);
    if(!tp->packageType) {
        printf("%s\n", tp->data);
    }

    //printTPPackage(&tp);
    //printTPPackage(&tp);
    //printf("Primljen paket\n");
    return 0;
}
