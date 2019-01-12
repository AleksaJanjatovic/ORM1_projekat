#include <stdio.h>
#include "include/initRoutingP.h"
#include "include/routerHost.h"
#include <string.h>
#include <pthread.h>

int main(int argc, char * argv[]) {
    routerModel rm;
    if(argc < 6) {
        //printf("Not enough arguments passed");
        return -1;
    }
    routerModel destRouter;
    RPAddress dummy;

    initRM(atoi(argv[1]), argv[2], argv[3], &rm, 0);
    initRM(atoi(argv[4]), argv[5], dummy, &destRouter, 1);
    connectRMToNetwork(&rm, &destRouter);
    pthread_t dataParsing, routerTableControl;
    ////printf("Pravljenje threada\n");
//    //printf("Greksa: %d\n", parseTP(&rm));
    pthread_create(&dataParsing, NULL, (void*)parseReceivedData, &rm);
    //pthread_join(dataParsing, NULL);
   // //printf("Pravljenje thread-a je probelm\n");
   while(1) {
        routerTableTimeControl(&rm);
//        routerTableTimeControl(&rm);
//        //printRouterTable(&rm);
//        //printRouterModel(&rm);
//        printRouterTable(&rm);
         usleep(20000);
   }

    ////printRouterModel(&rm);

//    char finished = 0;
//    rm.routerTable[1][0] = 2;
//    rm.routerTable[1][1] = 2;
//    rm.routerTable[1][2] = 3;
//    rm.routerTable[2][0] = 3;
//    rm.routerTable[2][1] = 1;
//    rm.routerTable[2][2] = 4;
//    rm.routerTable[2][3] = 7;
//    rm.routerTable[3][0] = 1;
//    rm.routerTable[3][1] = 1;
//    rm.routerTable[4][0] = 3;
//    rm.routerTable[4][1] = 2;
//    rm.routerTable[4][2] = 5;
//    rm.routerTable[4][3] = 8;
//    rm.routerTable[5][0] = 2;
//    rm.routerTable[5][1] = 9;
//    rm.routerTable[5][2] = 4;
//    rm.routerTable[8][0] = 2;ls
//    rm.routerTable[8][1] = 4;
//    rm.routerTable[8][2] = 10;
//    rm.routerTable[9][0] = 2;
//    rm.routerTable[9][1] = 8;
//    rm.routerTable[9][2] = 5;
//    rm.routerTable[10][0] = 2;
//    rm.routerTable[10][1] = 8;
//    rm.routerTable[10][2] = 11;
//    rm.routerTable[11][0] = 1;
//    rm.routerTable[11][1] = 10;
//    path[0] = 9;
//    findPath(9, 7, &rm, path, &nodeNum, &finished);
//    for(int i = 0; i <= nodeNum; i++) {
//        //printf("Path member %d is router %d\n", i,path[i]);
//    }

    return 0;
}
