#include <netinet/in.h>
#include "../include/utilityFun.h"
#include <stdio.h>

void convertTPackageToArray(transferPackage * tp, unsigned char buffer[CONVBUFFSIZETP]) {
    int i, pom = sizeof(RPAddress)*2;
    for(i = 0; i < sizeof(RPAddress); i++) {
        buffer[i]  = tp->sourceAddress[i];
        buffer[sizeof(RPAddress) + i] = tp->destinationAddress[i];
    }
    for(i = 0; i < MAXROUTERS; i++) {
        buffer[i + pom] = tp->path[i];
    }
    buffer[pom + MAXROUTERS] = tp->nodeNum;
    for(i = 0; i < tp->dataSent; i++) {
        buffer[i + MAXROUTERS + pom + 1] = tp->data[i];
    }
    buffer[CONVBUFFSIZETP - 4] = (unsigned char)((tp->dataSent)>>8);
    buffer[CONVBUFFSIZETP - 3] = (unsigned char)(tp->dataSent);
    buffer[CONVBUFFSIZETP - 2] = tp->packageType;
    buffer[CONVBUFFSIZETP - 1] = tp->currentNode;
}

void convertArrayToTPackage(unsigned char buffer[CONVBUFFSIZETP], transferPackage * tp) {
    int i, pom = sizeof(RPAddress);
    for(i= 0; i < sizeof(RPAddress); i++) {
        tp->sourceAddress[i] = buffer[i];
        tp->destinationAddress[i] = buffer[i + pom];
    }
    pom *= 2;
    for(i = 0; i < MAXROUTERS; i++) {
        tp->path[i] = buffer[pom + i];
    }
    tp->nodeNum = buffer[pom + MAXROUTERS];
    tp->dataSent = (((unsigned short)buffer[CONVBUFFSIZETP - 4]) << 8) + buffer[CONVBUFFSIZETP - 3];
    for(i = 0; i < tp->dataSent; ++i) {
        tp->data[i] = buffer[i + pom + MAXROUTERS + 1];
    }
    tp->packageType = buffer[CONVBUFFSIZETP - 2];
    tp->currentNode = buffer[CONVBUFFSIZETP - 1];
}
