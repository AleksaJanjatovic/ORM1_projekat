#include "../include/debugging.h"

void printRouterModel(routerModel * rm) {
    printf("Router model specifications: \n");
    printf("Router socket: %d\n", rm->socket );
    printf("Router IP: %d\n", rm->homeHost.sin_addr.s_addr);
    printf("Router port: %d\n", rm->homeHost.sin_port);
    printf("Router RPAddress: %s\n", rm->routerAddress);
    int i;
    for(i = 1; i < MAXUSERS; i++) {
        if(rm->users[i]) {
            printf("Router user %d present: \n", i);
        }
    }
}

void printUserModel(userModel * um) {
    printf("User model specifications: \n");
    printf("User socket: %d\n", um->socket);
    printf("User IP: %d\n", um->userHost.sin_addr.s_addr);
    printf("User port: %d\n", um->userHost.sin_port);
    printf("User RPAdress: %s\n", um->userAddress);
}

void printRouterTable(routerModel * rm) {
    unsigned char i,j;
    printf("Router table elements to %d :\n", MAXROUTERS/5);
    for(i = 0; i < MAXROUTERS/5; i++) {
        printf("%d: ", i);
        for(j = 0; j < MAXROUTERS/5; j++) {
            printf("%d ", rm->routerTable[i][j]);
        }
        printf("\n");
    }
}

void printTPackage(transferPackage * tp) {
    printf("Transfer package specifications: \n");
    printf("Source address %s\n", tp->sourceAddress);
    printf("Destination address %s\n", tp->destinationAddress);
    printf("Data sent: %d\n", tp->dataSent);
    printf("Data: %s\n", tp->data);
    printf("Pacakge type: %d\n", tp->packageType);
    printf("Current node number: %d\n", tp->currentNode);
    printf("Number of nodes: %d\n", tp->nodeNum);
    printf("Path: ");
    int i;
    for(i = 0; i < MAXROUTERS; i++) {
        printf("%d ", tp->path[i]);
    }
    printf("\n");
}
