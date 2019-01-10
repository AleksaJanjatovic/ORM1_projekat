#ifndef INIT_ROUTINGP
#define INIT_ROUTINGP
#include "sys/socket.h"
#include "sys/types.h"
#include "../include/protocolStructure.h"

//UDPinitTP initializes the UDP members of a transferPackage structure (more particularly, the sourceHost and destinationHost sockaddr_in members)
//Also, this function creates a socket, so that the user of the Router Protocol does not have to manage them
//The first argument is Port Number, the second argument is the IP, the third argument is the actual transferPackage
int initUM(unsigned int portNumber, char ipAddress[16], userModel* um);
int connectUMToNetwork(userModel * user, routerModel * destRouter);
//UDPinitRP initializes the UDP members of a routerPackage structure
//Also, this function creates a socket, so that the user of the Router Protocol does not have to manage them
//The first argument is Port Number, the second argument is the IP, the third argument is the actual routerPackage (more particularly, the destinationHost sockaddr_in)
int initRM(unsigned int portNumber, char ipAddress[16], RPAddress rpAddress, routerModel* rm, char userInitialisation);
int connectRMToNetwork(routerModel * sourceRouter, routerModel * destRouter);
int closeUM(userModel* um);
int closeRM(routerModel* rm);

#endif
