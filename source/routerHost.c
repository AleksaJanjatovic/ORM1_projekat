#include "../include/routerHost.h"
#include "../include/initRoutingP.h"
#include "../include/utilityFun.h"
#include "../include/debugging.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>

void convertRTableToArray(routerModel * rm, unsigned char array[ROUTERBUFFER]) {
    int a = 0, i, j;
    for(i = 0; i < MAXROUTERS; i++) { //preskacemo prvi red
        for(j = 0; j < MAXROUTERS; j++) {
            array[a++] = rm->routerTable[i][j];
        }
    }
}

int sendRouterTable(routerModel * rm) {

    unsigned char thisRouter = getRouterNumber(rm->routerAddress);
    unsigned char i;

    convertRTableToArray(rm, rm->sendTableBuffer);
    for(i = 1; i <= rm->routerTable[thisRouter][0]; i++){ // u sledecim redovima je negde segfault
        if(sendto(rm->socket, rm->sendTableBuffer, ROUTERBUFFER, 0,
    (struct sockaddr *)&(rm->routerHosts[rm->routerTable[thisRouter][i]]), (socklen_t)sizeof(struct sockaddr_in)) == -1) { //salje se na svaki ruter
            perror("ERROR! RouterTable sending error");
        }
    }

    return 0;
}

void convertArrayToRTable(unsigned char array[ROUTERBUFFER], routerModel * rm) {
    int i, j, a = MAXROUTERS; //zato sto pocinjemo od prvog reda
    unsigned char thisRouter = getRouterNumber(rm->routerAddress);
    for(i = 0; i < MAXROUTERS; i++) {
        if(array[i] > rm->routerTable[0][i]) {
            rm->routerTable[0][i] = array[i];
        }
    }
    for(i = 1; i < MAXROUTERS; i++) {
        if(array[a] > rm->routerTable[i][0]){ //Asking if received table has more neighbour routers
            rm->routerTable[i][0] = array[a++];
            for(j = 1; j < MAXROUTERS; j++) {
                rm->routerTable[i][j] = array[a++];
            }
        } else {
            a += MAXROUTERS; //Skipping an entire row of a router table if the upper is not the case
        }
    }
}

void parseRouterTable(routerModel * rm) {
    pthread_mutex_lock(&rm->routerTableMutex);

    convertArrayToRTable(rm->receiveBuffer, rm); //Updating our router table

    pthread_mutex_unlock(&rm->routerTableMutex);
}

void shiftNeighbours(unsigned char array[MAXROUTERS]) {
    int zero_indx = 1;
    int i, j;
    for(i = 1; i < array[0]; i++) {
        if(array[i] == 0) {
            zero_indx = i;
            for(j = zero_indx; j < array[0]; j++) {
                array[j] = array[j + 1];
            }
            break;
        }
    }
}

void routerTableTimeControl(routerModel * rm) {
    while(1) {
        int i,j,k;
        pthread_mutex_lock(&rm->routerTableMutex);
        rm->routerTable[0][getRouterNumber(rm->routerAddress)] = REFRESHVALUE; //stavljanje refresh counter-a na 20
        for(i = 1; i < MAXROUTERS; i++) {
            if(rm->routerTable[0][i]) { //Asking if table counters are not zero
                rm->routerTable[0][i]--;
            } else { //If upper is not the case, then we remove the outdated router
                if(rm->routerTable[i][0]) { //Asking if router i has any neighbours
                    for(j = 1; j <= rm->routerTable[i][0]; j++) {
                       rm->routerTable[i][j] = 0; //Deleting an entire row of the removing router
                    }
                    rm->routerTable[i][0] = 0;

                    for(j = 1; j < MAXROUTERS; j++) { //Checking if other routers have the removed router as a neighbour
                        if(rm->routerTable[j][0] && i != j) { //Skipping routers that are not in the network (that have 0 neighbours)
                            for(k = 1; k <= rm->routerTable[j][0]; k++) {
                                if(rm->routerTable[j][k] == i) {
                                        rm->routerTable[j][k] = 0;
                                        shiftNeighbours(rm->routerTable[j]); //Shifting the other neighbour routers to the left
                                        rm->routerTable[j][(rm->routerTable[j][0])--] = 0;
                                        break;
                                }
                            }

                        }
                    }
                }
            }
        }
        sendRouterTable(rm); //Broadcasting the updated router table
        pthread_mutex_unlock(&rm->routerTableMutex);
        usleep(RTABLEREFRESHRATE);
    }
}

int findPath(unsigned char currentRouter, unsigned char destRouter, routerModel * rm, unsigned char path[MAXROUTERS], unsigned char * nodeNum, char * finished) {
    path[(*nodeNum)++] = currentRouter;
    unsigned char i,j;
    char routerAlreadyInPath;

    for(i = 1; i <= rm->routerTable[currentRouter][0]; i++) { //Pass through all neighbouring routers
        if(!(*finished)) {

            routerAlreadyInPath = 0;
            for(j = 0; j < *nodeNum; j++) {
                if(path[j] == rm->routerTable[currentRouter][i]) { //Checking if the current router is already in path
                    routerAlreadyInPath = 1;
                    break;
                }
            }

            if(!routerAlreadyInPath) { //If the current router is not in the path
                if(rm->routerTable[currentRouter][i] == destRouter) { //Check if we found the destination router
                    *finished = 1; //Set the end flag
                    path[*nodeNum] = destRouter;
                    return 0;
                } else {
                    findPath(rm->routerTable[currentRouter][i], destRouter, rm, path, nodeNum, finished); //Otherwise search deeper into the network
                }
            }

        } else {
            return 0;
        }
    }
    if(!*finished) {
        path[--(*nodeNum)] = 0;
    }
    return -1;
}

int sendTPToNextRouter(routerModel * rm, transferPackage * tp) {
    unsigned char i;
    unsigned char thisRouter = getRouterNumber(rm->routerAddress);
    char nextRouterPresent = 0, finished = 0;

    pthread_mutex_lock(&rm->routerTableMutex);

    for(i = 1; i <= rm->routerTable[thisRouter][0]; i++) { //Checking if the router is present among neighbouring routers
        if(rm->routerTable[thisRouter][i] == tp->path[tp->currentNode + 1] && tp->nodeNum) {
            nextRouterPresent = 1;
            break;
        }
    }

    if(!nextRouterPresent) { //If not, then create a new path
        memset(tp->path, 0, MAXROUTERS);
        tp->currentNode = 0;
        tp->nodeNum = 0;
        if(findPath(thisRouter, getRouterNumber(tp->destinationAddress), rm, tp->path, &tp->nodeNum, &finished)) {
            printf("ERROR! No valid path found from this router!\n");
        }
    }

    tp->currentNode++;

    convertTPackageToArray(tp, rm->sendTPBuffer);

    //Sending the package to the next router in the path
    if(sendto(rm->socket, rm->sendTPBuffer, CONVBUFFSIZETP, 0,
              (struct sockaddr*)&(rm->routerHosts[tp->path[tp->currentNode]]), (socklen_t)sizeof(struct sockaddr_in)) == -1) {
        perror("ERROR! Send to next router failed");
        return -5;
    }

    pthread_mutex_unlock(&rm->routerTableMutex);
}

int parseTP(routerModel * rm, struct sockaddr_in * recv_address) {
    transferPackage tp;
    convertArrayToTPackage(rm->receiveBuffer, &tp);

    unsigned char path[MAXROUTERS];
    unsigned char thisRouter = getRouterNumber(rm->routerAddress);
    unsigned char destinationRouter = getRouterNumber(tp.destinationAddress);
    unsigned char receivingUser = getUserNumber(tp.destinationAddress);
    unsigned char otherRouter, i, pathFinished = 0;
    size_t len = sizeof(struct sockaddr_in);

    switch(tp.packageType) {
        case 0: //Accepting and sending a network package
            printTPackage(&tp);

            if(thisRouter == destinationRouter) {
                convertTPackageToArray(&tp, rm->sendTPBuffer);
                if(sendto(rm->socket, rm->sendTPBuffer, CONVBUFFSIZETP, 0, (struct sockaddr*)&rm->userHosts[receivingUser], (socklen_t)sizeof(struct sockaddr_in)) == -1) {
                    perror("ERROR! Message not sent to user");
                    return -1;
                }
            } else {
                sendTPToNextRouter(rm, &tp);
            }
            printTPackage(&tp);
        break;
        case 1: //Accepting a user connection package
            for(i = 1; i < MAXUSERS; i++) {
                if(!(rm->users[i])) {
                    rm->users[i] = 1;
                    break;
                }
            }
            rm->userHosts[i].sin_addr.s_addr = recv_address->sin_addr.s_addr;
            rm->userHosts[i].sin_family = AF_INET;
            rm->userHosts[i].sin_port = recv_address->sin_port;
            setRouterNumber(thisRouter, tp.sourceAddress);
            setUserNumber(i, tp.sourceAddress);

            printf("User %d connected to this router\n", i);

            convertTPackageToArray(&tp, rm->sendTPBuffer); //Sending the user number back to the user
            if(sendto(rm->socket, rm->sendTPBuffer, CONVBUFFSIZETP, 0, (struct sockaddr *)&(rm->userHosts[i]), sizeof(struct sockaddr_in)) == -1) {
                return -1;
            }

        break;
        case 2: //Accepting a router connection package
            otherRouter = getRouterNumber(tp.sourceAddress);

            pthread_mutex_lock(&rm->routerTableMutex);

            for(i = 1; i <= rm->routerTable[thisRouter][0]; i++) { //Checking if received router address is already taken
                if(rm->routerTable[thisRouter][i] == otherRouter)  {
                    i = -1;
                    break;
                }
            }
            if(i != -1) {
                rm->routerTable[thisRouter][++(rm->routerTable[thisRouter][0])] = otherRouter;
                rm->routerTable[0][thisRouter] = REFRESHVALUE;
                rm->routerTable[0][otherRouter] = REFRESHVALUE;
                rm->routerHosts[otherRouter].sin_addr.s_addr = recv_address->sin_addr.s_addr;
                rm->routerHosts[otherRouter].sin_port = recv_address->sin_port;
                rm->routerHosts[otherRouter].sin_family = AF_INET;
                setRouterNumber(thisRouter, tp.sourceAddress);
                printf("Router %d connected to this router\n", otherRouter);
            } else {
                setRouterNumber(0, tp.sourceAddress); //Setting the returning router address to 0 if the address is already taken
            }

            pthread_mutex_unlock(&rm->routerTableMutex);

            setUserNumber(0, tp.sourceAddress); //The user number of a router RPAddress has to be 0

            tp.packageType = 3; //Router connection response package
            convertTPackageToArray(&tp, rm->sendTPBuffer);
            if(sendto(rm->socket, rm->sendTPBuffer, CONVBUFFSIZETP, 0, // Sending the response package back to router
                      (struct sockaddr *)&rm->routerHosts[otherRouter], (socklen_t)sizeof(struct sockaddr_in)) == -1) {
                return -2;
            }

        break;
        case 3: //Accepting a router connection RESPONSE package
            convertArrayToTPackage(rm->receiveBuffer, &tp);

            if(!strcmp(tp.sourceAddress,  "000.000")) {
                return -8;
            }

            thisRouter = getRouterNumber(rm->routerAddress);
            otherRouter = getRouterNumber(tp.sourceAddress); //The returned RPAddress of the router this router connected to

            pthread_mutex_lock(&rm->routerTableMutex);

            rm->routerTable[0][thisRouter] = REFRESHVALUE; //Refreshing the table counter values of our routers
            rm->routerTable[0][otherRouter] = REFRESHVALUE;
            rm->routerTable[thisRouter][++rm->routerTable[thisRouter][0]] = otherRouter;
            rm->routerHosts[otherRouter].sin_addr.s_addr = recv_address->sin_addr.s_addr;
            rm->routerHosts[otherRouter].sin_port = recv_address->sin_port;
            rm->routerHosts[otherRouter].sin_family = AF_INET;

            pthread_mutex_unlock(&rm->routerTableMutex);

            printf("Router %d succesfully connected to router %d\n", thisRouter, otherRouter);
        break;
        default:
            printf("ERROR! Wrong package type!\n");
        break;
    }
}

int parseReceivedData(routerModel * rm) {
    struct sockaddr_in recv_address;
    unsigned short byteNum = 0;
    size_t len = sizeof(struct sockaddr_in);

    while(1) {
        if(memset(&recv_address, 0, sizeof(struct sockaddr_in)) == 0) { //
            printf("ERROR! recv_address memory fault in parseReceivedData!");
            return -1;
        }

        if((byteNum = recvfrom(rm->socket, rm->receiveBuffer, ROUTERBUFFER, 0, (struct sockaddr *)&recv_address, (socklen_t*)&len)) == -1) {
            perror("Message not recieved: ");
            return -2;
        } else if(byteNum == ROUTERBUFFER) {
            parseRouterTable(rm);
        } else if(byteNum == CONVBUFFSIZETP) {
            parseTP(rm, &recv_address);
        }
        byteNum = 0;
    }
}
