#ifndef TRANSFER_PROTOCOL
#define TRANSFER_PROTOCOL
#include "../include/protocolStructure.h"

#define CONVBUFFSIZETP 544 // ovoliko je dovoljno za duzinu buffer-a konverzije za TransferPackage
//sendTransferPackage is used for sending data over to the destination host defined in the trasnferPackage,
//First argument is an output buffer that we read data from,
int sendTransferPackage(char* outputBuffer, size_t bufferLength, transferPackage* tp, char attempts);
int receiveTransferPackage(char * inputBuffer, size_t  bufferLength, transferPackage* tp);
int sendRouterPackage(routerPackage* rp, char attempts); //Ovo nemoj jos koristiti, nisam je zavrsio, a i svejedno ti ovo treba za ruter samo
int receiveRouterPackage(routerPackage* rp); //Ovo nemoj jos koristiti, nisam je zavrsio, a i svejedno ti ovo treba za ruter samo


#endif
