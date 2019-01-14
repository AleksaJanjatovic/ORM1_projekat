#include <stdio.h>
#include "include/initRoutingP.h"
#include "include/userHost.h"
#include <string.h>
#include <pthread.h>

typedef struct receiveStruct_t {
    transferPackage * tp;
    userModel * um;
}receiveStruct;

void threadReceiveTPFromRouter(receiveStruct * rs) {
    receiveTPfromRouter(rs->tp, rs->um);
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
    transferPackage tp;
    unsigned short port;
    char ip_address[100];
    memset(&tp, 0, sizeof(transferPackage));

    receiveStruct rs;
    rs.tp = &tp;
    rs.um = &thisUser;

    pthread_t receiveDataThread;

    tp.dataSent = 3;
    tp.data[0] = 'A';
    tp.data[1] = 'B';
    tp.data[2] = 0;
    strcpy(tp.destinationAddress, "234.002");
    strcpy(tp.sourceAddress, "241.222");
    tp.path[0] = 123;
    tp.path[1] = 125;
    tp.path[2] = 220;
    tp.nodeNum = 23;
    convertTPackageToArray(&tp, destRouter.sendTPBuffer);
    convertArrayToTPackage(destRouter.sendTPBuffer, &tp);
    printTPPackage(&tp);

    initUser(&thisUser);

    while(1) {
        //system("clear");
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
            //pthread_create
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
            printf("Zivot\n");
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
            //fgets
            //printf("Destination address is %s\n", tp.destinationAddress);
            strcpy(tp.sourceAddress, thisUser.userAddress);
            tp.dataSent = strlen(tp.data) + 1;
            sendTPtoRouter(&tp, &thisUser);
            break;
        default:
            printf("Wrong menu option!\n");
            break;
        }
        c = 0;
        //printf("%d\n",c);
    }
//    routerModel rm;
//    if(argc < 6) {
//        //printf("Not enough arguments passed");
//        return -1;
//    }
//    userModel um;
//    transferPackage tp;

//    initUM(atoi(argv[1]), argv[2], &um);
//    initRM(atoi(argv[3]), argv[4], NULL, &rm);
////    //printRouterModel(&rm1);
//    connectUMToNetwork(&um, &rm);
//    printUserModel(&um);
//    strcpy(tp.data, "ZABA");
//    tp.packageType = 0;
//    strcpy(tp.sourceAddress, um.userAddress);
//    strcpy(tp.destinationAddress, argv[5]);


//    //printf("sending tp:\n");
//    tp.dataSent = 5;
//    sendTPtoRouter(&tp, &um);
//    //printTPPackage(&tp);
//    while(1) {
//        receiveTPfromRouter(&tp, &um);
//        printTPPackage(&tp);
//        usleep(1000000);
//    }
//    //printTPPackage(&tp);
//    closeRM(&rm);
//    closeUM(&um);
    return 0;
}
