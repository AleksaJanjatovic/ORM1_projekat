#include <stdio.h>
#include "include/initRoutingP.h"
#include "include/routerHost.h"
#include <string.h>
#include <pthread.h>

void routerInit(routerModel * baseRouterModel) {
    char ip_address[100];
    unsigned short port;
    RPAddress routerRPAddress;
    while(1) {
        printf("Enter the router IP address:\n");
        scanf("%s", ip_address);
        printf("Enter the router PORT number:\n");
        scanf("%hu", &port);
        getchar();
        printf("Enter the router RPAddress:\n");
        scanf("%s", routerRPAddress);
        getchar();
        if(initRM(port, ip_address, routerRPAddress, baseRouterModel) == -1) {
            printf("ERROR! Invalid router IP address passed!\n");
            continue;
        }
        break;
    }
    printRouterModel(baseRouterModel);
}

//void menu(routerModel * baseRouterModel, pthread_t * rTTcontrol, pthread_t * tpParsing) {
//    int c = 0;
//    char ip_address[100];
//    unsigned short port = 0;
//    routerModel destRouter;
//    while(1) {
//        //system("clear");
//        switch (c) {
//        case 0:
//            printf("Main menu:\n");
//            printf("1. Reinitialise base router\n");
//            printf("2. Connect router to another router from network\n");
//            printf("3. Print router model\n");
//            printf("4. Print router table\n");
//            scanf("%d", &c);
//            continue;
//            break;
//        case 1:
//            pthread_destroy(*rTTcontrol, 0);
//            pthread_destroy(*tpParsing, 0);
//            closeRM(baseRouterModel);
//            routerInit(baseRouterModel, rTTcontrol, tpParsing);
//            break;
//        case 2:
//            printf("Enter the destination router IP address:\n");
//            scanf("%s", ip_address);
//            printf("Enter the destination router PORT number:\n");
//            scanf("%hu", &port);
//            if(initRM(port, ip_address, NULL, &destRouter) == -1) {
//                printf("ERROR! Invalid router IP address passed!\n");
//                continue;
//            }
//            printf("Zivot\n");
//            if(connectRMToNetwork(baseRouterModel, &destRouter) == -8) {
//                printf("ERROR! Router with this RPAddress: %s is already connected to the router\n", baseRouterModel->routerAddress);
//                c = 0;
//            }
//            break;
//        case 3:
//            printRouterModel(baseRouterModel);
//            break;
//        case 4:
//            printRouterTable(baseRouterModel);
//            break;
//        default:
//            printf("Wrong menu option!\n");
//            break;
//        }
//        c = 0;
//        usleep(5000000);
//        printf("%d\n",c);
//    }
//}

int main(int argc, char * argv[]) {
    routerModel thisRouter;
    routerModel destRouter;

    pthread_t rTTControl, tpParsing;
    int c = 0;
    char ip_address[100];
    unsigned short port = 0;

    routerInit(&thisRouter);

    pthread_create(&rTTControl, 0, (void*)routerTableTimeControl,  &thisRouter);
    pthread_create(&tpParsing, 0, (void*)parseReceivedData, &thisRouter);

    while(1) {
        //system("clear");
        switch (c) {
        case 0:
            printf("Main menu:\n");
            printf("1. Reinitialise base router\n");
            printf("2. Connect router to another router from network\n");
            printf("3. Print router model\n");
            printf("4. Print router table\n");
            scanf("%d", &c);
            continue;
            break;
        case 1:
            pthread_kill(rTTControl, 0);
            pthread_kill(tpParsing, 0);
            closeRM(&thisRouter);
            routerInit(&thisRouter);
//            pthread_create(&rTTControl, 0, (void*)routerTableTimeControl,  &thisRouter);
//            pthread_create(&tpParsing, 0, (void*)parseReceivedData, &thisRouter);
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
            //printf("Zivot\n");
            if(connectRMToNetwork(&thisRouter, &destRouter) == -8) {
                printf("ERROR! Router with this RPAddress: %s is already connected to the router\n", thisRouter.routerAddress);
            }
            break;
        case 3:
            printRouterModel(&thisRouter);
            break;
        case 4:
            printRouterTable(&thisRouter);
            break;
        default:
            printf("Wrong menu option!\n");
            break;
        }
        c = 0;
        //printf("%d\n",c);
    }


    return 0;
}
