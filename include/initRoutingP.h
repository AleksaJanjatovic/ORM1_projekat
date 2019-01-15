#ifndef INIT_ROUTINGP
#define INIT_ROUTINGP
#include "sys/socket.h"
#include "sys/types.h"
#include "../include/protocolStructure.h"


/**
 * @brief initUM: Function initializes the userModel
 * @param portNumber: target port number
 * @param ipAddress: target IP address
 * @param um: userModel passed for initialization
 * @return negative numbers in case of an error, otherwise 0
 */
int initUM(unsigned int portNumber, char ipAddress[16], userModel* um);


/**
 * @brief connectUMToNetwork: Function connects userModel to target router
 * @param user: userModel passed for connection
 * @param destRouter: routerModel that user connects to
 * @return negative numbers in case of an error, otherwise 0
 */
int connectUMToNetwork(userModel * user, routerModel * destRouter);


/**
 * @brief initRM: Function initializes the routerModel
 * @param portNumber: target port number
 * @param ipAddress: target ip address
 * @param rpAddress: target RPAddres (note that this address must not be used by other routers in the network)
 * @param rm: routerModel passed for initialization
 * @return negative numbers in case of error, otherwise 0
 */
int initRM(unsigned int portNumber, char ipAddress[16], RPAddress rpAddress, routerModel* rm);


/**
 * @brief connectRMToNetwork: Funtion connects routerModel to anotherRouter (this can be called multiple times, each time with a different destRouter)
 * @param sourceRouter: routerModel passed for connection
 * @param destRouter: routerModel passed as a connection destination
 * @return negative numbers in case of error, otherwise 0
 */
int connectRMToNetwork(routerModel * sourceRouter, routerModel * destRouter);


/**
 * @brief closeUM: Function that closes the userModel socket
 * @param um: target userModel
 * @return negative numbers in case of an error, otherwise 0
 */
int closeUM(userModel* um);


/**
 * @brief closeRM: Function that closes the routerModel socket and destroys routerTableMutex
 * @param rm: target routerModel
 * @return negative numbers in case of an error, otherwise 0
 */
int closeRM(routerModel* rm);

#endif
