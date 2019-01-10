#include <netinet/in.h>
#include "../include/utilityFun.h"

void sockaddr_inToArray(struct sockaddr_in * sadr, unsigned char buffer[8]) {
    buffer[0] = (unsigned char)(sadr->sin_family);
    buffer[1] = (unsigned char)((sadr->sin_family)>>8);
    buffer[2] = (unsigned char)(sadr->sin_port);
    buffer[3] = (unsigned char)((sadr->sin_port)>>8);
    buffer[4] = (unsigned char)(sadr->sin_addr.s_addr);
    buffer[5] = (unsigned char)((sadr->sin_addr.s_addr)>>8);
    buffer[6] = (unsigned char)((sadr->sin_addr.s_addr)>>16);
    buffer[7] = (unsigned char)((sadr->sin_addr.s_addr)>>24);
}

void arrayTosockaddr_in(unsigned char * buffer, struct sockaddr_in * sadr) {
    sadr->sin_family = buffer[0] + (((unsigned short)buffer[1])<<8);
    sadr->sin_port = buffer[2] + (((unsigned short)buffer[3])<<8);
    sadr->sin_addr.s_addr = buffer[4] + (((unsigned long)buffer[5])<<8) + (((unsigned long)buffer[6])<<16) + (((unsigned long)buffer[7])<<24);
    int i;
    for(i = 0; i < 8; i++) {
        sadr->sin_zero[i] = 0;
    }
}

void convertTPackageToArray(transferPackage * tp, unsigned char buffer[CONVBUFFSIZETP]) {
    int pom = sizeof(RPAddress)*2;
    int i;
    for(i = 0; i < sizeof(RPAddress); i++) {
        buffer[i]  = tp->sourceAddress[i];
        buffer[sizeof(RPAddress) + i] = tp->destinationAddress[i];
    }
    for(i = 0; i < tp->dataSent; i++) {
        buffer[i+pom] = tp->data[i];
    }
    buffer[CONVBUFFSIZETP - 3] = (unsigned char)((tp->dataSent)>>8);
    buffer[CONVBUFFSIZETP - 2] = (unsigned char)(tp->dataSent);
    buffer[CONVBUFFSIZETP - 1] = tp->packageType;
}

void convertArrayToTPackage(unsigned char buffer[CONVBUFFSIZETP], transferPackage * tp) {
    int pom = sizeof(RPAddress);
    for(int i = 0; i < sizeof(RPAddress); i++) {
        tp->sourceAddress[i] = buffer[i];
        tp->destinationAddress[i] = buffer[i + pom];
    }
    pom *= 2;
    tp->dataSent = (((unsigned short)buffer[CONVBUFFSIZETP - 3]) << 8) + buffer[CONVBUFFSIZETP - 2];
    for(int i = 0; i < tp->dataSent; ++i) {
        tp->data[i] = buffer[i + pom];
    }
    tp->packageType = buffer[CONVBUFFSIZETP - 1];
}
