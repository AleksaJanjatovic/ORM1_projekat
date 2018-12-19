#ifndef UDP_LAYER
#define UDP_LAYER
#include "sys/socket.h"
#include "sys/types.h"
#include "protocolStructure.h"

//UDPinitTP initializes the UDP members of a transferPackage structure (more particularly, the sourceHost and destinationHost sockaddr_in members)
//Also, this function creates a socket, so that the user of the Router Protocol does not have to manage them
//The first argument is Port Number, the second argument is the IP, the third argument is the actual transferPackage
int UDPinitTP(unsigned int portNumber, char ipAddress[16], transferPackage* tp);
//UDPinitRP initializes the UDP members of a routerPackage structure
//Also, this function creates a socket, so that the user of the Router Protocol does not have to manage them
//The first argument is Port Number, the second argument is the IP, the third argument is the actual routerPackage (more particularly, the destinationHost sockaddr_in)
int UDPinitRP(unsigned int portNumber, char ipAddress[16], routerPackage* tp);
int UDPcloseTP(transferPackage* tp);
int UDPcloseRP(routerPackage* rp);

#endif