#ifndef PROTOCOL_STRUCTURE
#define PROTOCOL_STRUCTURE

#include <netinet/ip.h>
#define PACKAGE_MAX_SIZE 1000

//RPAddr represents the Router Protocol address in the following form
//"rrr.hhh"
//	rrr is the router number uniquelly defined for each router
//	hhh is the host number uniquelly defined for each host in router's subnet	
typedef char RPAddr[8];

//Transfer package generated when user sends data via the routerProtocol command
typedef struct transferPackage_t { 
    RPAddr destAddress[8];
	RPAddr sourceAddress[8];
    char data[PACKAGE_MAX_SIZE];
    struct sockaddr_in sourceHost;
    struct sockaddr_in destinationHost;
}transferPackage;

//routerPackage exchanged periodically between routers 
typedef struct routerPackage_t {
	char destAddress[8];
	int routerTable[256][256];
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
