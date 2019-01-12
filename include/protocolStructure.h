#ifndef PROTOCOL_STRUCTURE
#define PROTOCOL_STRUCTURE
#include <pthread.h>
#include <netinet/ip.h>
#define PACKAGEMAX 800
#define MAXUSERS 255
#define MAXROUTERS 100//maksimalni indeks rutera
#define CONVBUFFSIZETP PACKAGEMAX+35
#define REFRESHVALUE 5
#define ROUTERBUFFER MAXROUTERS*MAXROUTERS
//RPAddr represents the Routing Protocol address in the following form
//"rrr.hhh"
//	rrr is the router number uniquelly defined for each router
//	hhh is the host number uniquelly defined for each host in router's subnet
typedef unsigned char RPAddress[8];

//Transfer package used for file transfer

typedef struct userModel_t {
    RPAddress userAddress;
    struct sockaddr_in userHost;
    struct sockaddr_in homeHost;
    unsigned char sendTPBuffer[CONVBUFFSIZETP];
    unsigned char recieveTPBuffer[CONVBUFFSIZETP];
    int socket;
}userModel;

typedef struct transferPackage_t {
    RPAddress destinationAddress;
    RPAddress sourceAddress;
    char data[PACKAGEMAX];
    unsigned short dataSent;
    char packageType; //packageType = 0 za podatke, packageTpye = 1 za konekciju korisnika, packageType = 2 za konekciju rutera
}transferPackage;
//router

typedef struct routerModel_t{
    RPAddress routerAddress;
    unsigned char routerTable[MAXROUTERS][MAXROUTERS]; // 0 pozicija prve kolone predstavlja broj komsija svakog rutera
    pthread_mutex_t routerTableMutex;
    unsigned char receiveBuffer[ROUTERBUFFER];
    unsigned char sendTableBuffer[ROUTERBUFFER];
    unsigned char sendTPBuffer[CONVBUFFSIZETP];

    unsigned char users[MAXUSERS]; // svaki ruter moze da poseduje maskimalno 255 hostova // hostovi su na pocetku inicijalizovani na 0, na nultoj poziciji je broj user hostova
    struct sockaddr_in userHosts[MAXUSERS];

    int socket; //Broadcast soket za razmenu tabela

    struct sockaddr_in routerHosts[MAXROUTERS];
    struct sockaddr_in homeHost;
}routerModel;

//setRouterNumber() sets the router's part of a routerProtocol address
int setRouterNumber(unsigned char routerNumber, char * address);
//getRouterNumber() returns the number that represents the router's part of a Routing protocol address
unsigned char getRouterNumber(char * address);
//setHostNumber sets the host's part of a TransferPackage address
int setUserNumber(unsigned char userNumber, char * address);
//getHostNumber() return the number that represents the host's part of a Router Protocol address
unsigned char getUserNumber(char * address);

#endif
