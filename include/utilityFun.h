#ifndef UTILITY_FUN
#define UTILITY_FUN
#include "protocolStructure.h"

void sockaddr_inToArray(struct sockaddr_in * sadr, unsigned char buffer[8]);
void arrayTosockaddr_in(unsigned char * buffer, struct sockaddr_in * sadr);
void convertTPackageToArray(transferPackage * tp, unsigned char buffer[CONVBUFFSIZETP]);
void convertArrayToTPackage(unsigned char buffer[CONVBUFFSIZETP], transferPackage * tp);

#endif
