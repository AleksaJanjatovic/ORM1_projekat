#ifndef PROTOCOL_STRUCTURE
#define PROTOCOL_STRUCTURE

#include <netinet/ip.h>
#define PACKAGEMAX 500

//RPAddr represents the Router Protocol address in the following form
//"rrr.hhh"
//	rrr is the router number uniquelly defined for each router
//	hhh is the host number uniquelly defined for each host in router's subnet	
typedef unsigned char RPAddress[8];

//Transfer package used for file transfer
typedef struct transferPackage_t { 
    RPAddress destinationAddress;
    RPAddress sourceAddress;
    char data[PACKAGEMAX];
    unsigned short dataSent;
    unsigned short socket;
    struct sockaddr_in sourceHost;
    struct sockaddr_in destinationHost;
}transferPackage;

//routerPackage exchanged periodically between routers 
typedef struct routerPackage_t {
    RPAddress destinationAddress;
    unsigned short routerTable[256][256];
    unsigned short socket;
	struct sockaddr_in sourceHost;
	struct sockaddr_in destinationHost;	
}routerPackage;

//setRouterNumber() sets the router's part of a routerProtocol address 
int setRouterNumber(unsigned int, char*);
//getRouterNumber() returns the number that represents the router's part of a Router Protocol address
int getRouterNumber(char*);
//setHostNumber sets the host's part of a TransferPackage address
int setHostNumber(unsigned int, char*);
//getHostNumber() return the number that represents the host's part of a Router Protocol address
int getHostNumber(char*);

#endif
