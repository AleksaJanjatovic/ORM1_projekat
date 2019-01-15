#include <stdio.h>
#include "include/initRoutingP.h"
#include "include/userHost.h"
#include <string.h>
#include <pthread.h>
#include "include/debugging.h"

typedef struct receiveStruct_t {
    transferPackage * tp;
    userModel * um;
}receiveStruct;

void threadReceiveTPFromRouter(receiveStruct * rs) {
    while(1) {
        receiveTPfromRouter(rs->tp, rs->um);
    }
}

void initUser(userModel * baseUserModel) {
    char ip_address[100];
    unsigned short port;
    while(1) {
        printf("Enter the user IP address:\n");
        scanf("%s", ip_address);
        printf("Enter the user PORT number:\n");
        scanf("%hu", &port);
        getchar();
        if(initUM(port, ip_address, baseUserModel) == -7) {
            printf("ERROR! Invalid user IP address passed!\n");
            continue;
        }
        break;
    }
    printUserModel(baseUserModel);
}

int main(int argc, char * argv[]) {
    int c;
    userModel thisUser;
    routerModel destRouter;
    transferPackage tp_recv;
    transferPackage tp;
    unsigned short port;
    char ip_address[100];
    memset(&tp, 0, sizeof(transferPackage));
    memset(&tp_recv, 0, sizeof(transferPackage));

    receiveStruct rs;
    rs.tp = &tp_recv;
    rs.um = &thisUser;

    pthread_t receiveDataThread;

    initUser(&thisUser);
    system("clear");
    while(1) {
        switch (c) {
        case 0:
            printf("Main menu:\n");
            printf("1. Reinitialise user\n");
            printf("2. Connect user to a router from the network\n");
            printf("3. Print user model\n");
            printf("4. Send data\n");
            scanf("%d", &c);
            continue;
            break;
        case 1:
            pthread_kill(&receiveDataThread, 0);
            closeUM(&thisUser);
            initUser(&thisUser);
            break;
        case 2:
            printf("Enter the destination router IP address:\n");
            scanf("%s", ip_address);
            printf("Enter the destination router PORT number:\n");
            scanf("%hu", &port);
            if(initRM(port, ip_address, NULL, &destRouter) == -1) {
                printf("ERROR! Invalid router IP address passed!\n");
                continue;
            }

            connectUMToNetwork(&thisUser, &destRouter);

            pthread_create(&receiveDataThread, 0, (void*)threadReceiveTPFromRouter, &rs);

            break;
        case 3:

            printUserModel(&thisUser);

            break;
        case 4:
            printf("Enter your data ( NO MORE THEN %d ):\n", CONVBUFFSIZETP);
            scanf("%s", tp.data);
            getchar();
            printf("Enter the destination address\n");
            scanf("%s", tp.destinationAddress);
            getchar();

            strcpy(tp.sourceAddress, thisUser.userAddress);
            tp.dataSent = strlen(tp.data) + 1;
            sendTPtoRouter(&tp, &thisUser);

            break;
        default:
            printf("Wrong menu option!\n");
            break;
        }
        c = 0;
    }
    return 0;
}
