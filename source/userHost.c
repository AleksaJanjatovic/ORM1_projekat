#include "../include/userHost.h"
#include "../include/initRoutingP.h"
#include <unistd.h>
#include <stdio.h>
#include "../include/utilityFun.h"

int sendTPtoRouter(transferPackage * tp, userModel * um, char attempts) {
    int errorCounter = 0;
    convertTPackageToArray(tp, um->sendTPBuffer);
    while(sendto(um->socket, um->sendTPBuffer, CONVBUFFSIZETP, 0, (struct sockaddr *)&(um->homeHost), sizeof(struct sockaddr_in)) == -1){
        ////printf("Upomoc\n");
        usleep(100); // ovo se desava jedino ako dodje do greske, tj ako se paket ne posalje(to moze i zato sto je isteklo vreme od 100us)
        if(++errorCounter >= attempts) {
            return -4;
        }
    }
    return 0;
}

int receiveTPfromRouter(transferPackage* tp, userModel * um) {
    size_t len = sizeof(struct sockaddr_in);
    int i;
    if(recvfrom(um->socket, um->recieveTPBuffer, CONVBUFFSIZETP, 0, (struct sockaddr*)&(um->homeHost), (socklen_t*)&len) == -1) {
        return -4;
    }
    convertArrayToTPackage(um->recieveTPBuffer, tp);
    ////printTPPackage(&tp);
    ////printf("Primljen paket\n");
    return 0;
}
