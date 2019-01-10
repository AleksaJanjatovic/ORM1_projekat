#include <stdio.h>
#include "include/initRoutingP.h"
#include <string.h>
#include "include/routerHost.h"
#include <pthread.h>

int main(int argc, char * argv[]) {
    routerModel rm;
    initRM(atoi(argv[1]), argv[2], argv[3], &rm, 0);
    //printRouterModel(&rm);
    pthread_t tp_parsing, map_routing, map_recv;
    //printf("Pravljenje threada\n");
//    printf("Greksa: %d\n", parseTP(&rm));
    pthread_create(&tp_parsing, NULL, (void*)parseTP, &rm);
    pthread_create(&map_recv, NULL, (void*)receiveRouterTable, &rm);
//    pthread_join(tp_parsing, NULL);
    while(1) {
        routerTableTimeControl(&rm);
        sendRouterTable(&rm);
        printRouterTable(&rm);
        usleep(1000000);
    }
//    while(1) {
//        routerTableTimeControl(&rm);
//        sendRouterTable(&rm);
//        printRouterTable(&rm);
//        usleep(1000000);
//    }
//    for(int i = 0; i < 10; i++) {
//        initRM(9000 + i*3, "127.0.0.1", &rm[i]);
//    }
//    unsigned char matrix[256][256];
//    for(int i = 0; i < 256; i++) {
//        path[i] = 0;
//        for(int j = 0; j < 256; j++) {
//            rm.routerTable[i][j] = 0;
//        }
//    }

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
//        printf("Path member %d is router %d\n", i,path[i]);
//    }
    closeRM(&rm);
    return 0;
}
