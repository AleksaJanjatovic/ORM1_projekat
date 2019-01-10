#include <stdio.h>
#include "protocolStructure.h"

void printRouterModel(routerModel * rm) {
    printf("Router model specifications: \n");
    printf("Router RPAddress: %s\n", rm->routerAddress);
    int i;
    for(i = 1; i < MAXUSERS; i++) {
        if(rm->users[i]) {
            printf("Router user %d present\n", i);
        }
    }
//            typedef struct routerModel_t{
//                RPAddress routerAddress;
//                unsigned char routerTable[MAXROUTERS][MAXROUTERS]; // 0 pozicija prve kolone predstavlja broj komsija svakog rutera
//                pthread_mutex_t rT_mutex;
//                unsigned char transferBuffer[CONVBUFFSIZETP];
//                unsigned char receiveTableBuffer[ROUTERBUFFER];
//                unsigned char sendTableBuffer[ROUTERBUFFER];

//                unsigned char users[MAXUSERS]; // svaki ruter moze da poseduje maskimalno 255 hostova // hostovi su na pocetku inicijalizovani na 0, na nultoj poziciji je broj user hostova
//                struct sockaddr_in userHosts[MAXUSERS];

//                int socketTable; //Broadcast soket za razmenu tabela
//                int socketTransferPackage; //soket za razmenu paketa

//                struct sockaddr_in routerHosts[MAXROUTERS];
//                struct sockaddr_in homeHost;
//            }routerModel;
}

void printUserModel(userModel * um) {
    printf("User model specifications: \n");
    printf("User RPAdress: %s\n", um->userAddress);
}

void printRouterTable(routerModel * rm) {
    int i,j;
    for(i = 0; i < MAXROUTERS; i++) {
        printf("%d: ", i);
        for(j = 0; j < MAXROUTERS/5; j++) {
            printf("%d ", rm->routerTable[i][j]);
        }
        printf("\n");
    }
}

void printTPPackage(transferPackage * tp) {
    printf("Transfer package specifications: \n");
    printf("Source address %s\n", tp->sourceAddress);
    printf("Destination address %s\n", tp->destinationAddress);
    printf("Data sent: %d\n", tp->dataSent);
    printf("Pacakge type: %d\n", tp->packageType);
}
