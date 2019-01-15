#ifndef PROTOCOL_STRUCTURE
#define PROTOCOL_STRUCTURE
#include <pthread.h>
#include <netinet/ip.h>
#define PACKAGEMAX 800
#define MAXUSERS 255
#define MAXROUTERS 100
#define CONVBUFFSIZETP PACKAGEMAX+MAXROUTERS+36
#define REFRESHVALUE 100
#define RTABLEREFRESHRATE 200
#define ROUTERBUFFER MAXROUTERS*MAXROUTERS


/**
 * RPAddress presents the address used by Routing Protocol(TM)
 */
typedef unsigned char RPAddress[8];


/** @struct userModel
 * @brief This structure represents a user
 * @var userModel::userAddress: RPAddress of the user
 * @var userModel::userHost: User sockaddr_in structure
 * @var userModel::homeHost: A sockaddr_in structure of a router user is connected to
 * @var userModel::sendTPBuffer: Buffer used for sending data
 * @var userModel::receiveTPBuffer: Buffer used for receiving data
 * @var userModel::socket: User's socket
 */
typedef struct userModel_t {
    RPAddress userAddress;
    struct sockaddr_in userHost;
    struct sockaddr_in homeHost;
    unsigned char sendTPBuffer[CONVBUFFSIZETP];
    unsigned char receiveTPBuffer[CONVBUFFSIZETP];
    int socket;
}userModel;


/** @struct transferPackage
 * @brief This structure represents a network package
 * @var transferPackage::destinationAddress: Wanted destination RPAddress
 * @var transferPackage::sourceAddress: Source RPAddress that transferPackage was sent from
 * @var transferPackage::data[PACKAGEMAX]: Data buffer
 * @var transferPackage::path[MAXROUTERS]: transferPackage path used by routers for passing a package through a network
 * @var transferPackage::nodeNum: Number of routers a transferPackage has to pass to reach the destination
 * @var transferPackage::currentNum: Number of routers passed
 * @var transferPackage::dataSent: Number of bytes in Data buffer
 * @var transferPackage::packageType: Determines what the transferPackage is used for:
 *          packageType = 0 //Data package
 *          packageType = 1 //User connection package
 *          packageType = 2 //Router connection package
 *          packageType = 3 //Router connection RESPONSE package
 */
typedef struct transferPackage_t {
    RPAddress destinationAddress;
    RPAddress sourceAddress;
    char data[PACKAGEMAX];
    unsigned char path[MAXROUTERS];
    unsigned char nodeNum;
    unsigned char currentNode;
    unsigned short dataSent;
    char packageType; //packageType = 0 za podatke, packageTpye = 1 za konekciju korisnika, packageType = 2 za konekciju rutera,
    //packageType = 3 za response rutera
}transferPackage;


/** @struct routerModel
 * @brief This structure represents a router
 * @var routerModel::routerAddress: RPAddress of a router (user part of RPAddress is always 0)
 * @var routerModel::routerTable: Router's routing table used for network mapping :
 *      routerTable[0][i] for i > 1 holds counter value of the router "i" thatis periodically
 *          refreshed if router is present in the network
 *      routerTable[i][0] for i > 1 holds the number of neighbouring routers of the router "i"
 *      routerTable[i][j] for i > 1 and j > 1 hols neighbouring routers "j" of the router "i"
 * @var routerModel::routerTableMutex: A mutex that prevents routerTable's data race
 * @var routerModel::receiveBuffer: Buffer used for receiving both transferPackages and routerTables
 * @var routerModel::sendTableBuffer: Buffer used for routerTable sending
 * @var routerModel::sendTPBuffer: Buffer used for transferPackage sending
 * @var routerModel::users: Buffer that contains available users connected to the router
 * @var routerModel::userHosts: Array of sockaddr_in structures that corespond to available users
 * @var routerModel::routerHosts: Array of sockaddr_in structures that corespond to neighbouring routers
 * @var routerModel::homeHost: Router's sockaddr_in structure
 * @var routerModel::socket: Router's socket
 */
typedef struct routerModel_t{
    RPAddress routerAddress;
    unsigned char routerTable[MAXROUTERS][MAXROUTERS];
    pthread_mutex_t routerTableMutex;
    unsigned char receiveBuffer[ROUTERBUFFER];
    unsigned char sendTableBuffer[ROUTERBUFFER];
    unsigned char sendTPBuffer[CONVBUFFSIZETP];

    unsigned char users[MAXUSERS];
    struct sockaddr_in userHosts[MAXUSERS];

    int socket;

    struct sockaddr_in routerHosts[MAXROUTERS];
    struct sockaddr_in homeHost;
}routerModel;


/**
 * @brief setRouterNumber: Function sets the router number of passed RPAddress
 * @param routerNumber: target number
 * @param address: target RPAddress
 * @return negative values if passed routerNumber is below 1 or above MAXROUTERS, otherwise 0
 */
int setRouterNumber(unsigned char routerNumber, char * address);


/**
 * @brief getRouterNumber: Function returns the router number of passed RPAddress
 * @param address: target RPAddress
 * @return router number
 */
unsigned char getRouterNumber(char * address);


/**
 * @brief setUserNumber: Function sets the user number of passed RPAddress
 * @param userNumber: target userNumber
 * @param address: target RPAddress
 * @return negative values if passed userNumber is below 1 or above MAXUSERS, otherwise 0
 */
int setUserNumber(unsigned char userNumber, char * address);


/**
 * @brief getUserNumber: Function returns the user number of passed RPAddress
 * @param address: target RPAddress
 * @return user number
 */
unsigned char getUserNumber(char * address);
#endif
